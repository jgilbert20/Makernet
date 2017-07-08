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

uint16_t getHardwareID()
{
	return( 0xBEE2 );

}

#endif