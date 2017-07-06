#include <Interval.h>
#include <Types.h>

#include <ArduinoAPI.h>

Interval::Interval(  uint32_t period ) : _lastFired(0)
{
	_period = period;
}

boolean Interval::hasPassed()
{
	if ( millis() > _lastFired + _period )
	{
		_lastFired = millis();
		return 1;
	}
	return 0;
}
