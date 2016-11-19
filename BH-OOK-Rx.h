

/*
*
*    BHRecieverA.h
*
*
*
*
*/



#ifndef _BH-Rx_h_
#define _BH-Rx_h_
#include "Arduino.h"

void RecieverSetup(unsigned long lastPacketTime );	//Setup reciever timing for last packet lockout
void RecieverBegin(int RxPin, int RxInterupt);		//Starts reciever by attaching Irq
void RecieveDetach(int RxPin, int RxInterupt);		//Stops reciever by detaching Irq

byte packetLength();				// returns # of bits in packet
unsigned long RecieveAvailable();	// returns 24 or 18 bit address packet




//unsigned long RecieveAvailable16();  // returns 16 bit address packet


// unsigned long _lastPacketTimeQ = 2900000;     // time since last packet in microseconds

//unsigned long RecieveAvailableL();   // returns 24 or 18 bit unique address packet	

//nsigned long RecieveAvailableL16(); // returns 16 bit unique address packet






void recieve();


#endif //



    
    