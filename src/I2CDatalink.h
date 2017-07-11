/********************************************************
 **
 **  I2CDatalink.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#ifndef I2CDATALINK_H
#define I2CDATALINK_H

#ifdef ARDUINO

#include <Debug.h>
#include <Network.h>
#include <Datalink.h>
#include <Makernet.h>
#include <Types.h>

// Implements a Datalink layer using Arduino I2C. This code is
// ONLY intended to run on MCUs.

class I2CDatalink : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
	virtual void loop();

	// Flag that tracks if we have a packet to respond with (slave mode only)
	int returnFrameSize;
private:


};




#endif
#endif
