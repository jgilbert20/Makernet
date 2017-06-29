// Makernet revised core library rebuilt with improved hygene and ability to
// be tested and debugged on OSX

//  c++ -DMASTER makernet.cpp -o master && ./master
//  c++ -DSLAVE makernet.cpp -o slave && ./slave


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// globals.h

#define ADDR_UNASSIGNED 0x00
#define ADDR_BROADCAST  0xFF

// util.h

#define UI8(x) static_cast<uint8_t>(x)
#define DPR( X... )			   printDebug( X )
#define DLN( X... )			   printDebugln( X )
#define DPF( X... )			   { char debugBuffer[255]; snprintf( debugBuffer, 255, X ); printDebug( debugBuffer ); }
#define MAX(x,y) x > y ? x : y;
#define MIN(x,y) x > y ? y : x;

// Ardiono proxy stuff

#ifndef ARDUINO
#define HEX 0x10

void printDebug( const char *s );
void printDebug( char *s );
void printDebug( int i);
void printDebug( uint8_t i, int format);
void printDebugln( char *s );

#endif

// Stuff must go away if not in debug mode...





// util.cpp

void hexPrint( uint8_t *buffer, int size );
uint8_t crc8_ccitt_update (uint8_t inCrc, uint8_t inData);


#ifndef ARDUINO

void printDebug( const char *s )
{
	printf( "%s", s );
}


void printDebug( char *s )
{
	printf( "%s", s );
}


void printDebug( int i )
{
	printf( "%d", i );
}

void printDebug( uint8_t i, int format)
{
	if ( format == HEX ) {
		printf( "%x", i );
	}
}

void printDebugln( char *s )
{
	printf( "%s\n", s );
}

void printDebugln( )
{
	printf( "\n" );
}

#endif

void hexPrint( uint8_t *buffer, int size )
{
	for ( int i = 0 ; i < size ; i++ ) {
		uint8_t value = buffer[i];
		if (value < 0x10)
			DPR("0");
		DPR(value, HEX);
		DPR( " " );
	}
}

// CCITT CRC, originally from Atmel

uint8_t crc8_ccitt_update (uint8_t inCrc, uint8_t inData)
{
	uint8_t i;
	uint8_t data;
	data = inCrc ^ inData;
	for ( i = 0; i < 8; i++ ) {
		if (( data & 0x80 ) != 0 ) {
			data <<= 1;
			data ^= 0x07;
		} else {
			data <<= 1;
		}
	}
	return data;
}


uint8_t calculateCRC( uint8_t in, uint8_t *b, int size )
{
	uint8_t crc = 0;
	for ( int i = 0 ; i < size ; i++ )
		crc = crc8_ccitt_update( crc, *b++ );
	return crc;
}


// The makernet packet header is a common object used to "address" a packet.
// For maximimum effciency and economy of processor time and memory, this
// header is the direct byte order and contents of the first N bytes of all
// makernet messages. Note that the entire makernet message is actually
// sizeof( Packet ) + payload + 1 bytes for CRC

typedef struct {
	uint8_t dest; // The destination address (0=unassigned, 1=controller, FF=bcast)
	uint8_t src;  // The source address
	uint8_t port; // The destination port
	uint8_t size; // The size of the payload in bytes
	uint8_t payload[]; // The actual payload
} Packet;

// A "service" is a generic endpoint that can be a source, sink or both for
// packets. The makernet framework will pass handlePacket() to its registered
// services. Inside a handlePacket control flow, the handler is free to
// populate a response packet which will be immediately transmitted when the
// call is over.  It may also wait for the next polling event.
//
// Services are provided periodic opportunities to generate new packets. If
// datalink requires a master/slave architecture and the device is the slave,
// than the services are polled in order of port number to generate packets
// via pollPacket(). This function offers a pre-allocated buffer and pointer
// to the address block. In cases of being a master, or in cases of a peer-to-
// peer datalink (such as RS485 or RFM69W radio), the polling occurs during
// system loop(). This architecture allows a Service to be largely ignorant if
// it is being used in a master/slave or peer-to-peer network.
//
// Of course, it is always possible simply to get a handle to the Network
// object and spontaneously generate a new packet at any time but the
// mechanisms above will offer a slight efficiency gain by re-using the low-
// level buffers, thus avoiding copying.
//
// Note that in the makernet low-level architecture, packets are never queued.
// Therefore, as soon as a packet is formed, it is either passed to a caller
// and read, or it is passed to the network. This allows reuse of the ~255
// bytes of buffer space.

class ServiceBase {

public:
	virtual void handlePacket( Packet *p ) = 0;
	virtual void pollPacket( Packet *p ) = 0;

};


namespace Service {

class DeviceControl : public ServiceBase {
	virtual void handlePacket( Packet *p );
	virtual void pollPacket( Packet *p );
};

};





// The Network object implements a layer of the "network" - contiguous set of
// nodes over one or more data links. Handles forming packets, managing
// addresses, etc. All Makernet networks must have one "controller".
//
// Some specific definitions:
// "addresss" - a one-byte assignable locator. 00 is not assigned. FF is broadcast.
// "deviceID" - a permanent 16 bit
//
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

class Datalink;

class Network {

public:
	// Called by the datalink layer on receipt of a new frame
	void handleFrame( uint8_t *buffer, uint8_t len );
	// A universal way to send a packet
	int sendPacket( uint8_t address, uint8_t port, uint8_t size, uint8_t *payload);
	Datalink *datalink;
	void initialize();

	int address;

private:
	int sendRawPacket( uint8_t destination, uint8_t src, uint8_t port, uint8_t size, uint8_t *payload);
	int routeFrame( uint8_t *buffer, uint8_t len  );
};


// The datalink layer handles putting bytes on wires with no knowledge of what
// those bytes mean. Each collection of bytes is called a "frame". The
// datalink interface is made interchangable so that multiple datalinks such
// as I2C, RFM, and even ethernet could all be makernet enabled.

#define MAX_MAKERNET_FRAME_LENGTH 255

typedef void (*frameReceiveCallback_t)( uint8_t *buffer, uint8_t readSize );

class Datalink {

public:
	// Start the datalink including any external peripherals
	virtual void initialize() = 0;
	// Send a single frame
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len ) = 0;

	uint8_t frameBuffer[MAX_MAKERNET_FRAME_LENGTH];


	// Register a callback when new frames arrive
// 	void onReceive( frameReceiveCallback_t t );

// private:
// 	frameReceiveCallback_t frameReceiveCallback;

	Network *network;
	uint8_t address;

	// Service *services[16];

};


// This is a core singleton for the entire application. Right now, as an
// optimization, the singleton is used inside the Makernet framework itself
// so that each object is not forced to carry around pointers to the
// framework objects like Network and Datalink.

class _Makernet {
	Network network;
	void initialize();
};



void _Makernet::initialize()
{
	network.initialize();
}

// Singleton object
_Makernet Makernet;


// Called when the framework is initialized

void Network::initialize()
{

	return; 
}



// Called when we have a valid packet that is meant for us

int Network::routeFrame( uint8_t *buffer, uint8_t len  )
{


}


// Called when a network interface identifies an inbound frame

void Network::handleFrame(uint8_t *buffer, uint8_t len )
{
	if ( len <= 0 or buffer == NULL ) return;
	Packet *mp = (Packet *)buffer;

	if ( mp->dest != ADDR_BROADCAST or (address != ADDR_UNASSIGNED and address != mp->dest )) {
		DPR( "Dropping packet not for us");
		return;
	}

	DPF( "%%%%%%%% Inbound packet dest=[%i] src=[%i] port=[%i] size=[%i]\n",
	     mp->dest, mp->src, mp->port, mp->size );

	DPR( sizeof(Packet) );
	DLN();

	// Verify checksum
	uint8_t calculatedCRC = calculateCRC(0, buffer, len - 1 );
	uint8_t presentedCRC = buffer[len - 1];

	if ( calculatedCRC != presentedCRC ) {
		DPF( "%%%%%%%% CRC check failed: (%x) vs (%x), frame dropped\n", calculatedCRC, presentedCRC );
		return;
	}

	if ( routeFrame( buffer, len ) < 0 ) {
		DPR( "Frame failed to route" );
	}
}



// sendRawPacket(..) is the core function that assembles a packet into a
// frame. The resulting frame looks like this:
//
// (0)  [dst address]
// (1)  [src adddress]
// (2)  [port]
// (2)  [size 0-255 of payload]
// (3+) [payload 0...size]
// (n)  [CRC]

int Network::sendRawPacket( uint8_t destination, uint8_t src, uint8_t port, uint8_t size, uint8_t *payload)
{
	if ( size >= MAX_MAKERNET_FRAME_LENGTH - 1 )
		return -100;

	uint8_t *buffer = datalink->frameBuffer;

	((Packet *)buffer)->dest = destination;
	((Packet *)buffer)->src = src;
	((Packet *)buffer)->port = port;
	((Packet *)buffer)->size = size;

	int crc = calculateCRC( 0, buffer, sizeof( Packet ) );

	uint8_t *payloadPtr = ((Packet *)buffer)->payload;

	for ( int i = 0 ; i < size ; i++ ) {
		payloadPtr[i] = payload[i];
		crc = crc8_ccitt_update( crc, payload[i] );
	}

	payloadPtr[size] = crc;

	datalink->sendFrame( buffer, sizeof( Packet ) + size + 1 );
}

int Network::sendPacket( uint8_t destination, uint8_t port, uint8_t size, uint8_t *payload)
{
	if ( address == ADDR_UNASSIGNED ) {
		DPR( "ASSERT: Address not configured");
		return -1;
	}

	return sendRawPacket( destination, address, port, size, payload );

}


// Our callers can use this to set a callback when a new frame is received

// void Datalink::onReceive( frameReceiveCallback_t t )
// {
// 	frameReceiveCallback = t;
// }

// An implementation of UNIX domain socket master

class UnixMaster : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
private:
	struct sockaddr_un remote;
	int sock;

};


#define SOCK_PATH "echo_socket"


void UnixMaster::initialize()
{

	int t, len;

	char str[100];

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	printf("Trying to connect...\n");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(sock, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}

	printf("Connected.\n");

}

int UnixMaster::sendFrame( uint8_t *inBuffer, uint8_t len )
{
	if (send(sock, inBuffer, len, 0) == -1) {
		perror("send");
		exit(1);
	}

	DPR( "<<<< (" );
	DPR( len );
	DPR( ") ");
	hexPrint( inBuffer, len );
	DLN();


	int t;

	if ((t = recv(sock, frameBuffer, MAX_MAKERNET_FRAME_LENGTH, 0)) > 0) {
		frameBuffer[t] = '\0';

		DPR( ">>>> (" );
		DPR( t );
		DPR( ") ");
		hexPrint( frameBuffer, t );
		DLN();
		// printf("echo> %s", receiveBuffer);
	} else {
		if (t < 0) perror("recv");
		else printf("Server closed connection\n");
		exit(1);
	}

}


class UnixSlave : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
	void loop();
private:
	struct sockaddr_un local, remote;
	int sock;
	uint8_t receiveBuffer[MAX_MAKERNET_FRAME_LENGTH];

};


void UnixSlave::initialize()
{

	int len;

	char str[100];

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	local.sun_family = AF_UNIX;
	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(sock, (struct sockaddr *)&local, len) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sock, 5) == -1) {
		perror("listen");
		exit(1);
	}
}

int UnixSlave::sendFrame( uint8_t *inBuffer, uint8_t len )
{

}

void UnixSlave::loop()
{
	int t, s2;
	int done, n;
	printf("Waiting for a connection...\n");
	t = sizeof(remote);
	if ((s2 = accept(sock, (struct sockaddr *)&remote, (socklen_t *)&t)) == -1) {
		perror("accept");
		exit(1);
	}

	printf("<<<< Inbound transaction\n");

	done = 0;
	do {
		n = recv(s2, frameBuffer, MAX_MAKERNET_FRAME_LENGTH, 0);
		if (n <= 0) {
			if (n < 0) perror("recv");
			done = 1;
		}

		DPR( ">>>> (" );
		DPR( n );
		DPR( ") ");
		hexPrint( frameBuffer, n );
		DLN();

		network->handleFrame( frameBuffer, n );

		if (!done)
			if (send(s2, frameBuffer, n, 0) < 0) {
				perror("send");
				done = 1;
			}

		DPR( "<<<< (" );
		DPR( n );
		DPR( ") ");
		hexPrint( frameBuffer, n );
		DLN();

	} while (!done);

	close(s2);
}













#ifdef MASTER

int main(void)
{
	Network net;

	uint8_t x[] = { 1, 2, 3, 4, 5 };

	UnixMaster um;
	um.initialize();
	net.datalink = &um;
	um.network = &net;

	// um.sendFrame( x , 5 );

	net.address = 0x55;
	net.sendPacket( 0x22, 0x15, 5, x );


}

#else

int main(void)
{
	Network net;



	UnixSlave us;
	us.initialize();
	net.datalink = &us;
	us.network = &net;

	while (1)
		us.loop();


}

#endif
