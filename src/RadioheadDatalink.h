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

#ifndef RADIOHEAD_DATALINK_H
#define RADIOHEAD_DATALINK_H

#if( defined(ARDUINO) && defined(RADIO_SUPPORT) )

#include <Debug.h>
#include <Network.h>
#include <Datalink.h>
#include <MakernetSingleton.h>
#include <Types.h>

#include <RH_RF69.h>

// Implements a Datalink layer using Arduino I2C. This code is
// ONLY intended to run on MCUs.

class RadioheadDatalink : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
	virtual void loop();
	const bool datalinkDrivesPolling = false;
	
private:
	RH_RF69 rf69 = RH_RF69(8, 3);

};




#endif
#endif
