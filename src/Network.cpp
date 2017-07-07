

#include <Network.h>
#include <Debug.h>
#include <Makernet.h>
#include <Util.h>
#include <Datalink.h>

Network::Network() {
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
		// TODO: What does this do? an init??
	}
}



// Called when the framework is initialized

void Network::initialize()
{
	datalink->initialize();

	return;
}

void Network::useDatalink( Datalink *dl)
{
	datalink = dl;
}

// Internal handler that polls all services for their next packet and
// generates exactly one packet onto the wire. This route is only used in
// peer-to-peer, not for I2C-like transactional communciation, and it is called
// by the master loop.

int Network::sendNextPacket()
{
	Packet *p = (Packet *)datalink->frameBuffer;
	p->clear();
	p->src = address;

	// Find the next service that has a packet to send
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
		if ( s != NULL ) {
			if ( s->pollPacket(p) > 0 ) {
				int r = sendPacket( p );
				if ( r < 0 ) {
					DPR( dNETWORK | dERROR, "sendPacket had error:");
					DLN( r );


					// DLN( p->size );

				}
				return 1;
			}
		}
	}
	return 0;
}





void Network::loop()
{
// 	DLN( "--- Network Loop ---");
	DPF( dNETWORK, "--- Network Loop :: Generation [%d], hardwareID [%d], deviceType [%d]\n", Makernet.generation, Makernet.hardwareID, Makernet.deviceType );

	// Give all services a loop() opportunity
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
		if ( s != NULL )
			s->loop();
	}

	if ( Makernet.network.role != slave )
		sendNextPacket();
}


// routePacket() is alled when we have a valid packet that is meant for us.
// From this point on upwards into the stack, we can assume everything about
// the packet checks out (checksum, address, etc).
//
// Negative return values mean an error. Zero return values mean everything is
// fine. Positive return values have meaning TBD (could mean a reply is queued
// in the future.)


int Network::routePacket( Packet *p  )
{
	if ( p == NULL )
		return -200;

	if ( p->destPort < 0 or p->destPort >= NUM_PORTS )
		return -204;

	Service *service = services[p->destPort];
	if ( service == NULL )
		return -201;

	int retVal =  service->handlePacket( p );

	if ( retVal > 0 )
	{
		DPR( dNETWORK, "Route: Immediate packet sendback!" );
		if ( Makernet.network.role == Network::slave )
			DPR( dALL, "WARNING: Untested code path!");
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
	if ( port >= NUM_PORTS or port < 0 )
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

	if (!((mp->dest == ADDR_BROADCAST) or (address == ADDR_UNASSIGNED) or (address == mp->dest ))) {
		DLN( dNETWORK | dWARNING, "Dropping packet not for us");
		return;
	}

	if ( mp->destPort < 0 or mp->destPort >= NUM_PORTS ) {
		DLN( dNETWORK | dERROR, "Dropping invalid packet port.");
		return;
	}

	if ( mp->size < 0 or mp->size >= MAX_MAKERNET_FRAME_LENGTH - 1 - sizeof(Packet) ) {
		DPR( dNETWORK | dERROR, "Dropping invalid sized packet.");
		return;
	}

	DPF( dNETWORK, "%%%%%%%% Inbound packet dest=[%i] src=[%i] dPort=[%i] size=[%i]",
	     mp->dest, mp->src, mp->destPort, mp->size );
	DLN( dNETWORK );

	// Verify checksum
	uint8_t calculatedCRC = calculateCRC(0, buffer, len - 1 );
	uint8_t presentedCRC = buffer[len - 1];

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

// pollFrame is invoked by the datalink layer when a network condition allows
// a frame to be transmitted (e.g. link is pending), typically in cases where
// the datalink is acting as a slave and the master has given it permission to
// send. Must return the number of bytes to send. Note that in transaction-
// free peer-to-peer network architectures, pollFrame may NEVER be called so
// it should not be relied on.

int Network::pollFrame( uint8_t *buffer, uint8_t len )
{
	Packet *p = (Packet *)buffer;

	// Find the next service that has a packet to send
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
		if ( s != NULL ) {
			int pollRetValue = s->pollPacket(p);
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
		}
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
	if ( Makernet.network.role != master ) {
		DLN( dOBJFRAMEWORK | dERROR, "Internal consistency issue! Non-master tried to send a unrequested packet");
		return -1000;
	}

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

