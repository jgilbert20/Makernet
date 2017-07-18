/********************************************************
 ** 
 **  ArduinoAPI.cpp
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

// This file contains helper functions implemented using the Arduino API. 
// It can only be compiled on the Arduino toolchains

#ifdef ARDUINO

#include <ArduinoAPI.h>

uint16_t getRandomNumber16()
{
	return 1; // random( 16000 );

}


#ifdef __SAMD11D14AM__

// Returns a 16-bit unique ID based on hardwired processor information
// See SAMD11 datasheet - you need all four for guarneteed uniqueness
uint16_t getHardwareID()
{
  // Extract the serial number from the specific addresses according to the data sheet
  uint32_t serialNum[] = {
    *(uint32_t*)(0x0080A00C),
    *(uint32_t*)(0x0080A040),
    *(uint32_t*)(0x0080A044),
    *(uint32_t*)(0x0080A048)
  };

  // Hash the 128 bit unique ID into a 16 bit unique-ish ID
  uint16_t deviceID = 0;
  for (int i = 0; i < 4; i++) {
    deviceID ^= (serialNum[i] & 0xFFFF);
    deviceID ^= (serialNum[i] >> 16);
  }

  // Ensure that the generated device ID is never invalid
  if (deviceID == 0 or deviceID == 0xFFFF) {
    deviceID = 1;
  }

  return deviceID;
}

#else
uint16_t getHardwareID()
{
	return( 0xBEE2);
}


#endif


#endif