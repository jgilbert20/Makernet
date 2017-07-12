/********************************************************
 ** 
 **  Debug.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#ifndef DEBUG_H
#define DEBUG_H

#include <ArduinoAPI.h>
#include <Types.h>

#define dDATALINK     1 << 1
#define dNETWORK      1 << 2
#define dSERVICE      1 << 3
#define dDCS 	      1 << 4
#define dERROR        1 << 5
#define dOBJFRAMEWORK 1 << 6
#define dWARNING      1 << 7
#define dSTATUSMSG    1 << 8
#define dMAILBOX      1 << 9
#define dROUTE        1 << 10
#define dPOLL         1 << 11
#define dRESET        1 << 12
#define dALL          0xFFFFFFFF
#define dANY          0xFFFFFFFF
#define dNONE         0x00000000

#define DEBUGLEVEL    	dMAILBOX|dWARNING|dERROR
//|dSTATUSMSG|dDCS|dERROR|dWARNING|dDATALINK

/// dDCS|dERROR|dWARNING|dSTATUSMSG|dMAILBOX|dRESET

// dMAILBOX|dWARNING|dERROR|dSTATUSMSG
//   (dSTATUSMSG|dOBJFRAMEWORK)

// The following three macros are found throughout the code and implement an
// exceptionally lightweight conditional debugging framwork. When the
// DEBUGLEVEL mask is set to 0, the C++ compiler will strip these statements
// out entirely from the generated code thus creating ZERO overhead.
//
// DPR = print a single value
// DLN = print a single value with a newline
// DPF = printf

#define DPR( mask, X... )	    if( ((mask) & (DEBUGLEVEL)) > 0 ) { printDebug( X ); }
#define DFL( mask )	    		if( ((mask) & (DEBUGLEVEL)) > 0 ) { DEBUGSERIAL.flush(); }
#define DLN( mask, X... )	    if( ((mask) & (DEBUGLEVEL)) > 0 ) { printDebugln( X ); }
#define DPF( mask, X... )	    if( ((mask) & (DEBUGLEVEL)) > 0 ) { char debugBuffer[255]; snprintf( debugBuffer, 255, X ); printDebug( debugBuffer ); }
#define HPR( mask, ptr, size )  if( ((mask) & (DEBUGLEVEL)) > 0 ) { hexPrint( ptr, size ) }


#ifndef ARDUINO

#include <stdio.h>
#include <stdlib.h>

inline void printDebug( const char *s )
{
	printf( "%s", s );
}

inline void printDebug( char *s )
{
	printf( "%s", s );
}

inline void printDebug( int i )
{
	printf( "%d", i );
}

inline void printDebug( uint32_t i )
{
	printf( "%u", i );
}

inline void printDebug( uint8_t i, int format)
{
	if ( format == HEX ) {
		printf( "%x", i );
	}
}

inline void printDebugln( const char *s )
{
	printf( "%s\n", s );
}


inline void printDebugln( char *s )
{
	printf( "%s\n", s );
}

inline void printDebugln( )
{
	printf( "\n" );
}

inline void printDebugln( int i )
{
	printf( "%d\n", i );
}

inline void hexPrint( int mask, uint8_t *buffer, int size )
{
	for ( int i = 0 ; i < size ; i++ ) {
		uint8_t value = buffer[i];
		if (value < 0x10)
			DPR(mask, "0");
		DPR(mask, value, HEX);
		DPR(mask, " " );
	}
}


#else

#if defined(__SAMD11D14AM__)
#define DEBUGSERIAL Serial1
#else
#define DEBUGSERIAL Serial
#endif


#define printDebug( X... )    DEBUGSERIAL.print( X )
#define printDebugln( X... )    DEBUGSERIAL.println( X )


inline void hexPrint( int mask, uint8_t *buffer, int size )
{
	for ( int i = 0 ; i < size ; i++ ) {
		uint8_t value = buffer[i];
		if (value < 0x10)
			DPR(mask, "0");
		DPR(mask, value, HEX);
		DPR(mask, " " );
	}
}



#endif


#endif // DEBUG_H