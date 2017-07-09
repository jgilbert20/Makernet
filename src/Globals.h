/********************************************************
 ** 
 **  Globals.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

#define HWID_UNASSIGNED 0x0000

#define ADDR_UNASSIGNED 0x00
#define ADDR_BROADCAST  0xFF


// If true, support for controllers will be included in the runtime. There is
// no harm in leaving this on all the time, but it contributes to code size.
// As of July 8, disabling on a testbed is 6004 bytes, and enabling it
// adds about 800 bytes.
#define CONTROLLER_SUPPORT 1

// This is used throughout the code for calculations of maximum payload size.
#define MAX_MAKERNET_FRAME_LENGTH 25

// If enabled, certain boundary and configuration checks are made at runtime
// Slight increase to code size but makes errors much more sensible
#define API_CHECK 0

#endif
