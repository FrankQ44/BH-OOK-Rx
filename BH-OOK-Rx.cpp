
/*
BH-OOK-Rx.ccp
This is a mod to get up to 32 bits of OOK, (on off keying) 

*/ 

/*
*Bunker Hill Reciever library
*
* BH-Rx
* BHReciever
*
*/

/* Bunker Hill driveway alert system reciever for Arduino
*
*
*
*/

///////////////////////////////////////////////////////////
/*
New 24 + 0 bit timing
packet space   = 15000us
short space/bit  = 460us
long  space/bit = 1400us

Old 18 bit
packet space = 5000us
short space/bit = 320us
long  space/bit = 1000us

last packet time ~ 63,000us for 24 bits  ~61,500

50 packets per incident 

*/
//const byte packets = 50 ;       // Number of packets to transmit each time (50)
//------------------------------------------------------------------------------------------------------
#include "Arduino.h"
#include "BH-OOK-Rx.h"
//-----------------------------------
volatile unsigned long   recvBuffer00 = 0;
volatile byte            rbFull00     = 0;
volatile unsigned long   outBuffer    = 0;
volatile byte            outBufFull   = 0;
//-----------------------
volatile int rPin = 2;                   // Recieve pin
//-----------------------
static volatile  unsigned long _lastPacketTime = 600000;     // time since last packet in microseconds
//-----------------------
static volatile  byte _bitCount = 0;
static volatile  byte _bitsMin  = 12;  
static volatile  byte _bitsMax  = 32;  
static volatile  byte _bitCountOut = 0; 

//------set lockout period--------------------------------------------------------------------------------
void RecieverSetup( unsigned long lastPacketTime ){//Setup reciever timing for last packet lockout
    _lastPacketTime = (lastPacketTime);          // set the lockout time between identical packets   
    return;
  }
//------start reciever--attach irq -----------------------------------------------------------------------
void RecieverBegin(int RxPin, int RxInterupt){   // Uno (Pin 2, interupt 0) or (Pin 3, interupt 1) Irq
    rPin = RxPin;
	pinMode(RxPin, INPUT); 
    attachInterrupt(RxInterupt, recieve, CHANGE);// Interrupt on pin 2
    return;
  }
//-----stop reviever--detach irq -------------------------------------------------------------------------
void RecieveDetach(int RxInterupt){
    detachInterrupt(RxInterupt);
    return;
  }
//-----get bit count -------------------------------------------------------------------------------------
byte packetLength() { return(_bitCountOut); }	 // returns # of bits in packet
//-----poll for packets-----------------------------------------------------------------------------------
unsigned long RecieveAvailable(){                // Poll for packets
   if (rbFull00 == 1){
      outBuffer = recvBuffer00;
      rbFull00 = 0;
      outBufFull = 1;
    }
   else outBufFull = 0;
   if (outBufFull == 1){    // pkt Recieved
      outBufFull  = 0;
      return(outBuffer);    // ship it
    }
   else return(0);
  }
//----ISR Interupt service routine for reciever pin -------------------------------------------
void recieve() {
  const int _bitShort_18    =   320;      // space/bit short duration in microseconds
  const int _bitLong_18     =  1000;      // 18 bit older
  const int _packetSpace_18 =  5000;      // packet space duration 

  const int _bitShort_24     =   460;     // space/bit Long duration in microseconds
  const int _bitLong_24      =  1400;     // 24 + zero bit newer
  const int _packetSpace_24  = 15000;     // packet space duration 

  const int _bitShort_Min =  250;         // space/bit short duration minimum in microseconds
  const int _bitShort     =  320;         // packet space duration
  const int _bitShort_Max =  600;         // space/bit short duration maximum in microseconds

  const int _bitLong_Min  =  800;         // packet space duration
  const int _bitLong      = 1000;         // packet space duration
  const int _bitLong_Max  = 1600;         // packet space duration

  const int _packetSpace_Min =  4000;     // packet space duration
  const int _packetSpace =     15000;     // packet space duration
  const int _packetSpace_Max = 17000;     // packet space duration
  ///-----------------------
  const byte pacBits = 32;          	  // Number of bits in a packet (32)
  ///-----------------------
  static volatile  unsigned long recvBuffer = 0;             // Recieve packet buffer 32 bit
  static volatile  byte rbFull = 0;                          // Recieve packet buffer ready
  static volatile  byte rbIndex = (pacBits - 1);            // Recieve packet buffer index
  static volatile  byte rbIndexFlag = 0;
  
  static volatile unsigned long lowTimer = 0;        //Timer low time
  static volatile unsigned long lowMics  = 0;        //Low microseconds
  static volatile unsigned long lowWidth = 0;        //Low microseconds

  static volatile unsigned long highTimer = 0;        //Timer high time
  static volatile unsigned long highMics  = 0;        //High microseconds
  static volatile unsigned long highWidth = 0;        //High microseconds

  static volatile unsigned long _lastPacketTimer = 0;        //Timer since last packet
  static volatile unsigned long _lastPacketMics  = 0;        //temp microseconds
  static volatile unsigned long _lastPacketWidth = 0;        //Time since last packet
  
  static volatile unsigned long _PreviousPkt     = 0;        //Previous packet
  static volatile unsigned long _timeStampPrev   = 0;
  static volatile unsigned long _timeStamp       = 0;
  //---------------------------
  static volatile byte rPinLevelC = 0;                       //current reciever state
  static volatile byte rPinLevelP = 0;                       //Previous recieve state
  static volatile byte recvState  = 1;                       //Recieve state
  //---------------------------
  rPinLevelP = rPinLevelC;                   //Save previous revieved pin level
  rPinLevelC = digitalRead(rPin);            //Save recieve pin level
  lowMics = micros(); 
  highMics = micros(); 
  if (rPinLevelC == 1) highTimer = micros();       
  else lowTimer = micros();
  // Reciever states
  switch (recvState) 
   { ////////////////////////////////////////////////////////////////////////////////////
     // recvIdle_1
    case 1:  // Reciever Idle Waiting for statr pulse or packet space
       rbFull = 0;
       rbIndex = (pacBits - 1);             // Set to msb
       recvBuffer = 0;                      // Clear buffer
       if (rPinLevelC == 1) recvState = 2; // Check for start pulse on next Int
       else recvState = 3;                 // Check for packet space
       return;
       break;
    ////////////////////////////////////////////////////////////////////////////////////  
    // startPulseMin_2 
    case 2:  // Check for start pulse high 
      highWidth = highMics-highTimer; 
      if ((highWidth >= _bitShort_Min)&&(highWidth <= _bitShort_Max)) recvState = 3; // Good
      else recvState = 1; // No good
      return;  
      break;
    ////////////////////////////////////////////////////////////////////////////////////
    // packetStartSp_3 
    case 3:  //Check for packet start space
       lowWidth = lowMics-lowTimer; 
       if ((lowWidth >= _packetSpace_Min)&&(lowWidth <= _packetSpace_Max)){
	     recvState = 4;     // Check for first bit
         return;
        }
       else recvState = 1;  // No good
       return;
       break;
    ////////////////////////////////////////////////////////////////////////////////////
    // firstBit_4 
	// intializes buffers, indexes, flags and bit count
    case 4:   // check for First  bit
       rbFull = 0;
       rbIndex = (pacBits - 1);             // Set to msb
       recvBuffer = 0;                      // Clear buffer
	   _bitCount  = 0;						// clear bit count
       highWidth = highMics-highTimer;      // Get pulse width 
       if ((highWidth >= _bitShort_Min)&&(highWidth <= _bitShort_Max)){  //Check for zero bit
          // It's a zero bit
          bitClear(recvBuffer, rbIndex);   // Put bit in receive buffer
          recvState = 5;
          _bitCount++ ;         				// inc bit cout
          return;
        }  
       if ((highWidth <= _bitLong_Max)&&(highWidth >= _bitLong_Min)){    //Check for one bit
          // It's a one bit
          bitSet(recvBuffer, rbIndex);    // Put bit in receive buffer
          recvState = 5;                  // Check for next bit
		  _bitCount++ ;        				// inc bit cout
          return;
        }
       else { 
          recvState = 1;                  // No good     
          return;
        }
       return;          
       break;
     //////////////////////////////////////////////////////////////////////////////////////
     // nextBit_5
     case 5:  // Check for next bit (Low pulse)
      lowWidth = lowMics-lowTimer;
      if ((lowWidth >= _bitShort_Min) && (lowWidth <= _bitLong_Max)){ //Check for bit space
          recvState = 6; // Check for BitHighT, get next bit
          return;
        }
      if ((lowWidth >= _packetSpace_Min)&&(lowWidth <= _packetSpace_Max)){ //Check for start pulse space (low) 
   		 if (_bitCount >= _bitsMin){    // it's _bitsMin bit
              recvState = 8;
              return;
			}
         else if ((rbIndex <= 0) ||(_bitCount >= _bitsMax)){    // it's 32 bit, filled up buffer or 25
              recvState = 8;
              return;
            }
         else {
              recvState = 4; // Check for first bit              
              return;
            }
        }
      else recvState = 1; // No good
      return;
      break;
    //////////////////////////////////////////////////////////////////////////////////////  
    // BitHigh_6 
    case 6:  // Check for BitHighT
       highWidth = highMics-highTimer;               // Get pulse width 
       if ((highWidth >= _bitShort_Min)&&(highWidth <= _bitShort_Max)){ 		// It's a zreo
          rbIndex-- ;                               // dec index
		  _bitCount++;							    // inc bitCount
          bitClear(recvBuffer, rbIndex);            // Put bit in receive buffer
		  if ((rbIndex <= 0) || (_bitCount >= _bitsMax)){
             recvState = 8;
             return;
            }
          else {
             recvState = 5;                         // Check for next bit
             return;
            }
        }                   
       if ((highWidth >= _bitLong_Min)&&(highWidth <= _bitLong_Max)){  // It's a one 
         rbIndex-- ;              	               // dec index
		 _bitCount++;							   // inc bitCount
         bitSet(recvBuffer, rbIndex);        	   // Put bit in receive buffer
         if ((rbIndex <= 0) || (_bitCount >= _bitsMax)){
             recvState = 8;
             return;
            }
         else {
             recvState = 5;                  // Check for next bit
             return;
            }
        }                   
       recvState = 1;                     // Return to idle     
       return;          
       break;
   ////////////////////////////////////////////////////////////////////////////////////////  
   // packetStart_8
   case 8:  // prep recieve buffer, Check for packet start
   
	if (_bitCount >= _bitsMin) { //|| (_bitCount >= _bitsMax)){    // it's 18 bits or 25 bits, good
	  if (_bitCount == 18) recvBuffer = recvBuffer >> 2;
	  if (_bitCount == 19) recvBuffer = recvBuffer >> 2;
	  rbFull = 1;                  // Recieve buffer is full
	  _timeStampPrev =  _timeStamp;
	  _timeStamp = micros();
	  if ( _PreviousPkt == recvBuffer){		// If it's the same check the time
	     if (( _timeStamp - _timeStampPrev )>= _lastPacketTime){ // If it's been long enough upadate buffers, else ignor
             if (rbFull00 == 0){
                 recvBuffer00 = recvBuffer;
                 rbFull = 0;                  
                 rbFull00 = 1;
			     _bitCountOut = _bitCount;
                }
		    }
          else{
		     rbFull = 0;
		    }
	    }
	   else{  // rbFull = 1;                  // Recieve buffer is full
           if (rbFull00 == 0){
			  recvBuffer00 = recvBuffer;
              rbFull = 0;                  
              rbFull00 = 1;
			  _bitCountOut = _bitCount;
            }
	       else{
              rbFull = 0;
		    }
	    }
	  // save packet and time stamp  ---------------------
	  _PreviousPkt = recvBuffer;         // current packet is now previous packet
      _lastPacketTimer = micros();       // reset last packet timer
      lowWidth = lowMics-lowTimer;       // Check for packet start ---------
      if ((lowWidth >= _packetSpace_Min)&&(lowWidth <= _packetSpace_Max)) { //Check for packet space duration
         recvState = 4; // Check for first bit
         return;
        }
      else { 
         recvState = 1; // No good
         return;
        } 
	  //-------------------------------------------------
    } 
   else { 
     recvState = 1; // No good
     return;
    }
   return;   
   break;
   /////////////////////////////////////////////////////////////////////////////////////
  default: // if nothing else matches, do the default
  recvState = 1;    // No good               
  return;
 } // end switch case  
}  // recieve()---ISR------------
 


