/********************************************************
 **
 **  Interval.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/


#include <Interval.h>
#include <Types.h>

#include <ArduinoAPI.h>

Interval::Interval( uint32_t period ) : _lastFired(0)
{
	_period = period;
}

boolean Interval::hasPassed()
{
	if ( _lastFired == 0 or (millis() > _lastFired + _period )) {
		_lastFired = millis();
		return 1;
	}
	return 0;
}

boolean Interval::ready()
{
	if ( millis() > _lastFired + _period ) {
		return 1;
	}
	return 0;
}

void Interval::trigger()
{
	_lastFired = 0;
}


void Interval::reset()
{
	_lastFired = millis();
}
