/********************************************************
 **
 **  Network.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/


#include <Network.h>
#include <Debug.h>
#include <MakernetSingleton.h>
#include <Util.h>
#include <Datalink.h>
#include <BasePeripheral.h>

Network::Network() {
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		services[i] = NULL;
	}
}



// Called when the framework is initialized

void Network::initialize()
{
	// Register the DCS (this also inits it!)
	Makernet.network.registerService(PORT_DCS, &deviceControlSvc);
	// Init our datalink
	datalink->initialize();
	return;
}

void Network::useDatalink( Datalink *dl)
{
	datalink = dl;
}


// Internal handler function that is called at times when the framework is
// ready to send a  packet. This can occur in two different cases. One of them
// is during the ::loop() operation. The second is in master/slave
// communication when the slave is explicity polled for anything it wants to
// send back. Returns positive values in the case of a packet found, otherwise
// zero.

int Network::pollPacket(Packet *p)
{
	// DLN( dPOLL, "Network::polling all services and peripherals for packets");

	if(pendingPacket)
		DPR( dERROR, "Internal assert, should not be polling for a new packet while one is pending!\n");

	// Find the next service that has a packet to send
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
		if ( s != NULL ) {
			int retValue = s->pollPacket(p);
			if ( retValue > 0 )
				return retValue;
		}
	}

//   DLN( dPOLL, "Network::polling all peripherals");


#if CONTROLLER_SUPPORT
	// If we are the master, do the same for every BasePeripheral object


	if ( role == Network::master ) {
		int r = BasePeripheral::pollPacket(p);
		if ( r > 0 )
			return r;
	}
#endif

	return 0;
}

// Internal handler that polls all services for their next packet and
// generates exactly one packet onto the wire. This route is only used in
// peer-to-peer, not for I2C-like transactional communciation, and it is
// called by the master loop. Returns 1 if a packet was snet, otherwise 0.

int Network::sendNextPacket()
{
	if(pendingPacket)
		DPR( dERROR, "Internal assert, should not be sending a new packet while one is pending!\n");

	Packet *p = (Packet *)datalink->frameBuffer;
	p->clear();
	p->src = address;

	int rval = pollPacket( p );
	if ( rval > 0 )
	{
		int r = sendPacket( p );
		if ( r < 0 ) {
			DPR( dNETWORK | dERROR, "sendPacket had error:");
			DLN( r );
			return 0;
		}
		return 1;
	}
	return 0;
}



Interval topOfLoop = Interval(1000);

void Network::loop()
{
	// DLN( dNETWORK, "++ Top of net loop");

	if ( topOfLoop.hasPassed() )
		DPF( dNETWORK, "--- Network Loop :: Generation [%d], hardwareID [%d], deviceType [%d], addr [%d]\n",
		     Makernet.generation, Makernet.hardwareID, Makernet.deviceType, address );

	// If on the last loop call we generated a packet, send it back now
	if (pendingPacket)
	{
		DLN( dROUTE, "Route: Deferred packet sendback!" );
		// if ( Makernet.network.role == Network::slave )
		// 	DLN( dALL, "WARNING: not fully tested code path!");
		int retValSend = sendPacket( (Packet *)datalink->frameBuffer );
		if ( retValSend < 0 )
		{
			DPR( dNETWORK | dERROR, "Route: Immediate packet sendback failed, err=" );
			DPR( dNETWORK | dERROR, retValSend );
			DLN( dNETWORK | dERROR );
		}
		pendingPacket = 0;


		// CONTROVERSIAL - could cause starvation, but may be important for
		// time critical applications
		return;
	}

	// Pass along the loop to lower level driver. This may actually be what
	// triggers the receipt of new packets depending on the driver's
	// architecture.
	datalink->loop();


	// Check if a packet has been queued for next loop(). If so, take no further action
	if( pendingPacket )
		return;

	// Send up to three packets per loop
    //	if ( Makernet.network.role != slave or !datalink->datalinkDrivesPolling )
	if ( 1 )
		for ( int i = 0 ; i < 1 ; i++ )
		{
			//	DLN( dNETWORK, "******");
			//		DPF( dNETWORK, "Network: Requesting sendNextPacket time# %d\n", i );
			if ( sendNextPacket() == 0 )
				return;
		}
}

// Notification of a reset. This is called exactly once at the very end of
// initialization (so its guarnenteed to be called at the start of the
// program), and then again when requested by the controller over the network.

void Network::busReset()
{
	DLN( dNETWORK | dRESET, "Network: Resetting all services");
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
		if ( s != NULL )
			s->busReset();
	}
}


void Network::issueBusReset()
{
	deviceControlSvc.issueBusReset();;

}

// routePacket() is alled when we have a valid packet that is meant for us.
// From this point on upwards into the stack, we can assume everything about
// the packet checks out (checksum, address, etc).
//
// Negative return values mean an error. Zero return values mean everything is
// fine. Positive return values have meaning TBD (could mean a reply is queued
// in the future.)

#define DOIMMEDIATE 0

int Network::routePacket( Packet *p )
{
	if ( API_CHECK and p == NULL )
		return -5200;

	if ( p->destPort < 0 or p->destPort >= NUM_PORTS )
		return -5201;

	Service *service = NULL;

	// Routing path if we are a slave or the message is a DCS packet is to
	// route into our Network registered Service...

	if ( p->destPort == 0 or Makernet.network.role == slave ) {
		service = services[p->destPort];
		if ( service == NULL )
			return -5202;
	}

	// Routing path if we are a controller is to first attempt routing into a
	// BasePeripheral object

	if ( CONTROLLER_SUPPORT and Makernet.network.role == master and
	        p->src != ADDR_UNASSIGNED and p->dest != ADDR_BROADCAST ) {
		BasePeripheral *bp = BasePeripheral::findByAddress( p->src );

		if ( bp != NULL )
			service = bp->services[p->destPort];
		else
			DLN( dROUTE, "Found a peripheral but no matching service");
	}

	if ( service == NULL )
		return ( -5203 );


	long m = micros();
	int retVal =  service->handlePacket( p );
	DST( dTIMING );
	DPR( dTIMING,  "Service dispatch took us=");
	DPR( dTIMING,  micros() - m );
	DLN( dTIMING );

	if ( retVal > 0 )
	{
		// This is the case where delivery of the packet has generated
		// something we need to return. We either return it right away,
		// or wait for the next ::loop() call. Deferring this has the benefit
		// of reducing latency on the loop call.

		if ( DOIMMEDIATE ) {
			DLN( dROUTE, "Route: Immediate packet sendback!" );
			// if ( Makernet.network.role == Network::slave )
			// 	DLN( dALL, "WARNING: not fully tested code path!");
			int retValSend = sendPacket( p );
			if ( retValSend < 0 )
			{
				DPR( dNETWORK | dERROR, "Route: Immediate packet sendback failed, err=" );
				DPR( dNETWORK | dERROR, retValSend );
				DLN( dNETWORK | dERROR );
				return retValSend;
			}
			return 0;
		}
		else {
			pendingPacket = true;
			DST( dROUTE);
			DPR( dROUTE, "Route: My reply packet is being deferred.." );
			DLN( dROUTE);
		}
	}

	if ( retVal < 0 )
	{
		DPR( dNETWORK | dERROR, "Route: Exception on handlePacket: ");
		DPR( dNETWORK | dERROR, retVal );
		DLN( dNETWORK | dERROR, );
	}

	return retVal;
}

// Registers and initializes service

int Network::registerService( int port, Service* s )
{
	if ( API_CHECK && (port >= NUM_PORTS or port < 0 ))
		return -1;
	s->port = port;
	services[port] = s;
	s->initialize();
	return 1;
}

// Called when a network interface (the Datalink object) identifies an inbound
// frame. Purpose is to boundary check everything and ensure it is a valid
// packet so that no further data validation is needed upstream and then route
// the packet up into the network stack.

void Network::handleFrame(uint8_t *buffer, uint8_t len )
{
	if ( len <= 0 or buffer == NULL ) return;
	Packet *mp = (Packet *)buffer;

	DPF( dNETWORK, "Inbound frame, my address = %i\n", address );


	// drop unless broadcast packet or i have an assigned address and this packet is for me
	// drop if destination is not broadcast or if i am unassined or if the packet isn't for me
	if (mp->dest != ADDR_BROADCAST)
		if ( (address == ADDR_UNASSIGNED) or (address != mp->dest )) {
			DLN( dNETWORK | dWARNING, "handleFrame: Dropping packet not for us");
			return;
		}

	if ( mp->destPort < 0 or mp->destPort >= NUM_PORTS ) {
		DLN( dNETWORK | dERROR, "handleFrame: Dropping invalid packet port.");
		return;
	}

	if ( mp->size < 0 or mp->size >= MAX_MAKERNET_FRAME_LENGTH - 1 - sizeof(Packet) ) {
		DPR( dNETWORK | dERROR, "Dropping invalid sized packet.");
		return;
	}

	uint8_t packetSize = sizeof( Packet ) + mp->size;
	uint8_t packetSizeWithCRC = packetSize + 1;

	DPF( dNETWORK, "%%%%%%%% Inbound packet dest=[%i] src=[%i] dPort=[%i] frameSize=[%i] packetSize=[%i]",
	     mp->dest, mp->src, mp->destPort, mp->size, packetSize );
	DLN( dNETWORK );

	// Surpress this warning - its extremely common because I2C code in Arduino
	// routinely doesn't detect when the client is "done" and reads the full length
	// of the buffer
#if 0
	if ( packetSizeWithCRC != len)
		DPF( dNETWORK | dWARNING, "%%%%%%%% WARNING: Inbound frame size [%d] != frameSize [%d]; ignoring extras!\n", packetSizeWithCRC, len  )
#endif
		// Verify checksum
		uint8_t calculatedCRC = calculateCRC(0, buffer, packetSize );
	uint8_t presentedCRC = buffer[packetSize];

	if ( calculatedCRC != presentedCRC ) {
		DPF( dNETWORK, "%%%%%%%% CRC check failed: (%x) vs (%x), frame dropped\n", calculatedCRC, presentedCRC );
		return;
	}

	int s = routePacket( mp );
	if ( s < 0 ) {
		DPR( dNETWORK | dERROR, "Frame failed to route, err=" );
		DPR( dNETWORK | dERROR, s );
		DLN( dNETWORK | dERROR );
	}
}

// Packet *preparePacketShortcut( uint8_t addr, uint8_t )

// pollFrame is invoked by the datalink layer when a network condition allows
// a frame to be transmitted (e.g. link is pending), typically in cases where
// the datalink is acting as a slave and the master has given it permission to
// send. Must return the number of bytes to send. Note that in transaction-
// free peer-to-peer network architectures, pollFrame may NEVER be called so
// it should not be relied on.

int Network::pollFrame( uint8_t *buffer, uint8_t len )
{
	Packet *p = (Packet *)buffer;


	int pollRetValue = pollPacket(p);
	if ( pollRetValue > 0 ) {
		int finalRetValue = finalizePacketToFrame( p );
		if ( finalRetValue > 0 )
			return finalRetValue;
		else {
			DPR( dNETWORK | dERROR, "Return packet failed to finalize: err");
			DPR( dNETWORK | dERROR, finalRetValue );
			DLN( dNETWORK | dERROR, );
		}
	}
	else if ( pollRetValue < 0) {
		DPR( dNETWORK | dERROR, "pollPacket returned negative?? Something wrong");
		DLN( dNETWORK | dERROR );
	}
	return 0;
}

// sendPacket(..) is the core function that assembles a packet into a
// frame. The resulting frame looks like this:
//
// (0)  [dst address]
// (1)  [src adddress]
// (2)  [port]
// (2)  [size 0-255 of payload]
// (3+) [payload 0...size]
// (n)  [CRC]

int Network::sendPacket( uint8_t destination, uint8_t src, uint8_t destPort, uint8_t size, uint8_t *payload )
{
	if ( size >= MAX_MAKERNET_FRAME_LENGTH - 1 )
		return -100;

	uint8_t *buffer = datalink->frameBuffer;

	((Packet *)buffer)->dest = destination;
	((Packet *)buffer)->src = src;
	((Packet *)buffer)->destPort = destPort;
	((Packet *)buffer)->size = size;

	uint8_t *payloadPtr = ((Packet *)buffer)->payload;

	for ( int i = 0 ; i < size ; i++ ) {
		payloadPtr[i] = payload[i];
	}

	return sendPacket( (Packet *)buffer );
}


// Handles all of the mojo needed to make a packet able to sit on the wire
// such as adding a checksum and range checking the packet. Returns full size
// of the frame if the packet is OK, otherwise negative.

int Network::finalizePacketToFrame( Packet *p )
{
	if ( p == NULL )
		return -3000;

	DST( dNETWORK );
	DPR( dNETWORK, "Finalize: ");
	hexPrint( dNETWORK, (uint8_t *)p, 25 );
	DLN( dNETWORK );


	int size = p->size;

	if ( size >= MAX_MAKERNET_FRAME_LENGTH - 1 )
		return -3001;

	if ( p->dest == ADDR_UNASSIGNED )
		return -3002;

	// Crc the header
	int crc = calculateCRC( 0, (uint8_t *)p, sizeof( Packet ) );

	// Crc the packet contents
	uint8_t *payloadPtr = p->payload;
	for ( int i = 0 ; i < size ; i++ ) {
		crc = crc8_ccitt_update( crc, payloadPtr[i] );
	}

	payloadPtr[size] = crc;

	return sizeof( Packet ) + p->size + 1;
}

// returns 1 on successful send, otherwise negative

int Network::sendPacket( Packet *p )
{
	// if ( Makernet.network.role != master ) {
	// 	DLN( dOBJFRAMEWORK | dERROR, "Internal consistency issue! Non-master tried to send a unrequested packet");
	// 	return -1000;
	// }

	int r = finalizePacketToFrame( p );
	if ( r <= 0 )
		return r;

	return datalink->sendFrame( (uint8_t *)p, r );
}


int Network::sendPacket( uint8_t destination, uint8_t destPort, uint8_t size, uint8_t *payload)
{
	if ( address == ADDR_UNASSIGNED ) {
		DPR( dOBJFRAMEWORK | dERROR, "ASSERT: Address not configured");
		return -1;
	}

	return sendPacket( destination, address, destPort, size, payload );

}


