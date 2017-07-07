// This defines a common way to pull in the Arduino API (usually a #include
// <Arduino.h>) in a way that is multiple-platform compatable. It also defines
// a few helper functions that are highly hardware dependant and are not
// properly implemented in the ArduinoCore.
//
// In the case that we are on the actual Arduino platform this file pulls in
// the main header file. Otherwise, it defines constants and functions that
// allow a non-Arduino platform (such as UNIX) to hobble along.

#ifndef ARDUINO_API_H
#define ARDUINO_API_H

#ifdef ARDUINO

#include <Types.h>
#include <Arduino.h>

#else

#define HEX 0x10
uint32_t millis();

#endif

// These are common functions that are NOT in the arduino core but are highly
// hardware dependant. They are either implemented in ArduinoUtil.cpp or in
// UnixArduino.cpp

uint16_t getHardwareID();
uint16_t getRandomNumber16();



#endif