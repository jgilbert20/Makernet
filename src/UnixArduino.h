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
