/********************************************************
 **
 **  MakernetSingleton.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#ifndef MAKERNETSINGLETON_H
#define MAKERNETSINGLETON_H


// This is a core singleton for the entire application. Right now, as an
// optimization, the singleton is used inside the Makernet framework itself
// so that each object is not forced to carry around pointers to the
// framework objects like Network and Datalink.
//

#include <I2CDatalink.h>
#include <Interval.h>
#include <Packet.h>
#include <Network.h>
#include <MailboxService.h>

class _Makernet {
public:
	Interval reportingInterval = Interval(1000);
	Network network;
	DeviceType deviceType;
	uint16_t hardwareID;
	uint16_t generation;

	void initialize( DeviceType d );
	void initialize( DeviceType d, MailboxService& ms );
	void loop();
	void busReset();
	void issueBusReset();

	// Only used if i'm a peripheral
	DeviceProfile controller;

// #ifdef ARDUINO
// 	I2CDatalink i2cDatalink;
// #endif



};

extern _Makernet Makernet;



#endif
