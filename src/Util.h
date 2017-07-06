#ifndef UTIL_H
#define UTIL_H

#include <Types.h>

#define UI8(x) static_cast<uint8_t>(x)
#define MAX(x,y) x > y ? x : y;
#define MIN(x,y) x > y ? y : x;

uint8_t calculateCRC( uint8_t in, uint8_t *b, int size );
uint8_t crc8_ccitt_update (uint8_t inCrc, uint8_t inData);

#endif