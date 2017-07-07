#ifdef ARDUINO

#include <Types.h>
#include <Arduino.h>

#else
#ifndef ARDUINO_API_H
#define ARDUINO_API_H


#define HEX 0x10
uint32_t millis();

#endif
#endif

uint16_t getHardwareID();