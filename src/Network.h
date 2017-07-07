/********************************************************
 ** 
 **  Network.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/


#ifndef NETWORK_H
#define NETWORK_H

#include <Packet.h>
#include <Service.h>



// The Network object implements a layer of the "network" - contiguous set of
// nodes over one or more data links. Handles forming packets, managing
// addresses, etc. 
//
// Some specific definitions:
// "addresss" - a one-byte assignable locator. 00 is not assigned. FF is broadcast.
// "deviceID" - a permanent 16 bit
// All Makernet networks must have one "controller".
// All makernet packets have the following format:
//
// (0)  [address]
// (1)  [port]
// (2)  [size 0-255 of payload]
// (3+) [payload 1...size]
// (n)  [CRC]
//
// All network traffic will follow this format
//
// The network class maintains a registry of packet Endpoints each identified by
// a port number 0..15.
//
// 0 is reserved as an endpoint for network control functions.

#define NUM_PORTS 16

class Datalink;

class Network {

public:
	Network();
	// Called by the datalink layer on receipt of a new frame
	void handleFrame( uint8_t *buffer, uint8_t len );
	// Called when the datalink layer has been cleared to send a frame. Return
	// positive values if a frame has been populated, zero if not
	int pollFrame( uint8_t *buffer, uint8_t len );
	// A universal way to send a packet
	int sendPacket( uint8_t destination, uint8_t src, uint8_t port, uint8_t size, uint8_t *payload);
	int sendPacket( uint8_t address, uint8_t port, uint8_t size, uint8_t *payload);
	int	sendPacket( Packet *p );
	// Takes a packet populated by upper layers and finalizes the entire
	// buffer to be a frame
	int finalizePacketToFrame( Packet *p );

	int sendNextPacket();

	Datalink *datalink;
	void initialize();
	void useDatalink( Datalink *dl);
	int registerService( int port, Service *s );
	void loop();
	void busReset();

	enum { master, slave, peer } role;
	int address;
	Service *services[NUM_PORTS];

private:

	int routePacket( Packet *p );
};



#endif
