/********************************************************
 ** 
 **  UnixArduino.h
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
//
// Note: the actual function signatures for the ArduinoAPI (like millis()
// should not be defined here), only the supporting library


#if !defined(UNIXARDUINO_H) && !defined(ARDUINO)
#define UNIXARDUINO_H

#include <Types.h>

extern long long MICROSECOND_START;
extern long long MICROSECOND_COUNTER;

void startMicrosecondCounter();
void updateMicrosecondCounter();

void printDebug( const char *s );
void printDebug( char *s );
void printDebug( int i);
void printDebug( uint8_t i, int format);
void printDebugln( char *s );

extern uint16_t FAKEHARDWAREID;


#endif
