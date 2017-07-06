#ifndef INTERVAL_H
#define INTERVAL_H

#include <Types.h>

class Interval {
public:
	Interval( uint32_t period );
	boolean hasPassed();
private:
	uint32_t _lastFired;
	uint32_t _period;
};

#endif
