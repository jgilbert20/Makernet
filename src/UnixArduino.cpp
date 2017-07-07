/********************************************************
 ** 
 **  UnixArduino.cpp
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/



// The UnixArduino.* fileset implement functions and environment aspects that
// are expected in the Arduino environment but not available in UNIX. These
// adapters allow the Makernet framework to compile unit tests within UNIX.

#ifndef ARDUINO

// Faking it libraries

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>

#include <Globals.h>
#include <Types.h>
#include <ArduinoAPI.h>
#include <UnixArduino.h>

// Everything here is in the faking it category

uint16_t FAKEHARDWAREID = 0;


long long MICROSECOND_START = 0;
long long MICROSECOND_COUNTER = 0;


uint32_t millis()
{
	updateMicrosecondCounter();
	return MICROSECOND_COUNTER / 1000;
}

// Returns the 16 bit hardware ID. THis can be any value except
// HWID_UNASSIGNED.

uint16_t getHardwareID()
{
	if ( FAKEHARDWAREID != HWID_UNASSIGNED )
		return FAKEHARDWAREID;
	else
		return FAKEHARDWAREID + 1;
}

uint16_t getRandomNumber16()
{
	return random();
}

struct timeval stTimeVal;

inline long long getMicrosecondTime()
{
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * 1000000ll + stTimeVal.tv_usec;
}

void startMicrosecondCounter()
{
	MICROSECOND_START = getMicrosecondTime();
	MICROSECOND_COUNTER = 0;
}

void updateMicrosecondCounter()
{
	MICROSECOND_COUNTER = getMicrosecondTime() - MICROSECOND_START;
}





#endif
