      



//**************************************************************************************

#include <BH-OOK-Rx.h>
#include "NonBlockFlasher.h"


//#include <RTClib.h> // for 1307 real time clock calander I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int hits = 0;
byte bitCount = 0;
unsigned long recievedPkt = 0;
unsigned long lastPacketTime = 800000; // lockout time between the same packets in microseconds //milliseconds


LiquidCrystal_I2C lcd(0x20,20,4);  // set the LCD address to 0x20 for a 20 chars and 4 line display
//RTC_DS1307 rtc;

NonBlockFlasher slowFlasher(13, 800, 1);
NonBlockFlasher c10Flasher(12, 100, 1);




void setup()
{
 //BH ---------------------------------------------- 
 RecieverSetup(lastPacketTime);//Setup reciever timing for last packet lockout
 RecieverBegin(3, 1); //Uno (Pin 2, interupt 0) or (Pin 3, interupt 1)
 
 //none blocking flashers -------------------------- 
 slowFlasher.flash(20);
 c10Flasher.flash(15);

 // Serial -----------------------------------------
 Serial.begin(115200);
 
 // I2C ----------------------------------------- 
 Wire.begin();
 
 // LCD -----------------------------------------
 lcd.init();                      // initialize the lcd 
 lcd.backlight();
 lcd.clear();
 
 lcd.setCursor(0,0);
 lcd.print("Arduino_Bunker_Hill");
 lcd.setCursor(0,1);
 lcd.print(__TIME__);
 lcd.print(" ");  
 lcd.print(__DATE__);
 lcd.setCursor(0,2);
//lcd.print(__FILE__);  
  

   
  //Serial. 
  
    Serial.print(__TIME__);
    Serial.print("   ");
    Serial.println(__DATE__); 
    Serial.println(__FILE__); 
  
  
  
   delay(3000);
   lcd.clear();
  
 
  
///////////////////////////////////////////////////////////////////////////////////////////////////
}// End setup
////////////////////////////////////////////////////

void loop () 
  {
    
    /////////////////////////////////////////////////////////////////////////
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input >= 48 && input <= 57) //[0,9]
    {
     /*
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(TRANSMITPERIOD);
      Serial.println("ms\n");
     */ 
    }

    if (input == 'r') //d=dump register values
      dummyTask();
      
    if (input == 'r') //d=dump register values
      dummyTask();
      
    if (input == 'r') //d=dump register values
      dummyTask();
      
    if (input == 'r') //d=dump register values
      dummyTask();
      
    if (input == 'r') //d=dump register values
      dummyTask();
      
    if (input == 'r') //d=dump register values
      dummyTask();
      
   } // end serial available  
  //-----------------------------------------------
 
   recievedPkt = 0;
   bitCount = packetLength();
   recievedPkt = RecieveAvailable();        // full packet 18-25-32 bits
   		    // returns # of bits in packet
   if (recievedPkt != 0)
     {
       gotUnqPacket();   //pktRecieved
     }
    // service flashers 
    slowFlasher.flash(60);
    slowFlasher.flashRun();
    c10Flasher.flashRun();
      
  }//End loop() 


void dummyTask()
  {
    return;
  }

  
void gotUnqPacket()
  {
   // hits counter -----------
   if ( hits >= 999 ) hits = 0;
   hits++ ;
   unsigned long add = 0;
   displayLcd(recievedPkt);
   c10Flasher.flash(15);
   
   Serial.print("Bunker_Hill ");
   Serial.print("0x"); 
   add = recievedPkt;
      if (add <= 0x0fffffff ) Serial.print("0");
        if (add <= 0x00ffffff ) Serial.print("0");
          if (add <= 0x000fffff ) Serial.print("0");
            if (add <= 0x0000ffff ) Serial.print("0");
              if (add <= 0x00000fff ) Serial.print("0");
                if (add <= 0x000000ff ) Serial.print("0");
                  if (add <= 0x0000000f ) Serial.print("0");
   Serial.print(recievedPkt, HEX);
   Serial.print("__bit count=[");
   Serial.print(bitCount);
   Serial.print("] ");
   Serial.print("hits=[");
   Serial.print(hits);
   Serial.println("]");
   return;
  }
///////////////////////////////////////////////   
void  displayLcd(unsigned long pkt)
  {
   static byte row = 1;
   static byte col = 0;
   
   row = 0;
   col = 0;
   lcdPrintHex(col,row, pkt);
   lcdPrintBitCount(bitCount);
   lcdPrintBinary(pkt);
     return;
  }// end displayLcd() 
 
// lcd print bit count
void lcdPrintBitCount(byte _b)
  {
    lcd.setCursor(0,1);
    lcd.print(_b);
    return; 
  }
// LCD Print Hex 
void lcdPrintHex(byte col, byte row, unsigned long pkt) 
   {
     unsigned long _outBuffer = 0;
     _outBuffer = pkt; 
     
     lcd.setCursor(col,row);
     //lcd.print("0x");
     if (_outBuffer <= 0x0FFFFFFF)
       {
     if (_outBuffer <= 0x00FFFFFF)
       {
     
     if (_outBuffer <= 0x000FFFFF)
       {
        if (_outBuffer <= 65535)
          {
           if (_outBuffer <= 4095)
             {
               if (_outBuffer <= 255)
                 {
                  if (_outBuffer <= 15)
                    {
                      lcd.print("0"); 
                    }
                  lcd.print("0"); 
                 }
               lcd.print("0");
              }
            lcd.print("0"); 
           }
         lcd.print("0"); 
        }
         lcd.print("0"); 
        }
         lcd.print("0"); 
        }

      lcd.print(_outBuffer,HEX);
     return;
   }

// print binary on 3 lines
void lcdPrintBinary(unsigned long pkt)
    {
      unsigned long _outBuffer = 0;
     _outBuffer = pkt; 
      lcd.setCursor(10,0);
      for (int i = 31; i >= 24; i--)
        {
          int bitB = bitRead(_outBuffer, i);
          if (bitB == 0)
            {
               lcd.print("0");   
            }
          else
            {
              lcd.print("1");  
            }
        }
        
     
      lcd.setCursor(10,1);
      for (int i = 23; i >= 16; i--)
        {
          int bitB = bitRead(_outBuffer, i);
          if (bitB == 0)
            {
               lcd.print("0");   
            }
          else
            {
              lcd.print("1");  
            }
        }
        
      lcd.setCursor(10,2);
      for (int i = 15; i >= 8; i--)
        {
          int bitB = bitRead(_outBuffer, i);
          if (bitB == 0)
            {
               lcd.print("0");   
            }
          else
            {
              lcd.print("1");  
            }
        }
        
      lcd.setCursor(10,3);
      for (int i = 7; i >= 0; i--)
        {
          int bitB = bitRead(_outBuffer, i);
          if (bitB == 0)
            {
               lcd.print("0");   
            }
          else
            {
              lcd.print("1");  
            }
        }
    return;
  }
  
  
  











