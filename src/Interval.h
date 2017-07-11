/********************************************************
 ** 
 **  Interval.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#ifndef INTERVAL_H
#define INTERVAL_H

#include <Types.h>

class Interval {
public:
	Interval( uint32_t period );
	boolean hasPassed();
	void reset();
	void trigger();
	boolean ready();
private:
	uint32_t _lastFired;
	uint32_t _period;
};

#endif
