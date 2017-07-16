/********************************************************
 ** 
 **  ArduinoAPI.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

// This defines a common way to pull in the Arduino API (usually a #include
// <Arduino.h>) in a way that is multiple-platform compatable. It also defines
// a few helper functions that are highly hardware dependant and are not
// properly implemented in the ArduinoCore.
//
// In the case that we are on the actual Arduino platform this file pulls in
// the main header file.

// In the case we are not on an arduino platform (such as UNIX), this defines
// constants and functions that allow a non-Arduino platform (such as UNIX) to
// hobble along and 'fake' it. 

#ifndef ARDUINO_API_H
#define ARDUINO_API_H

#include <Types.h>

#ifdef ARDUINO

#include <Arduino.h>

#else

#define HEX 0x10
uint32_t millis();

// from wiring_constants.h

#define LOW             (0x0)
#define HIGH            (0x1)

#define INPUT           (0x0)
#define OUTPUT          (0x1)
#define INPUT_PULLUP    (0x2)
#define INPUT_PULLDOWN  (0x3)

#endif

// These are common functions that are NOT in the arduino core but are highly
// hardware dependant. They are either implemented in ArduinoUtil.cpp or in
// UnixArduino.cpp

uint16_t getHardwareID();
uint16_t getRandomNumber16();



#endif