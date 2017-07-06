// Makernet revised core library rebuilt with improved hygene and ability to
// be tested and debugged on OSX

//  c++ -DMASTER makernet.cpp -o master && ./master
//  c++ -DSLAVE makernet.cpp -o slave && ./slave

// This option makes a ton of warnings go away...
// -std=c++11


/********************************************************
 *
 * <FILE>
 *
 * Part of the Makernet framework by Jeremy Gilbert
 *
 * License: GPL 3
 * See footer for copyright and license details.
 *
 ******/


/********************************************************
 *
 * Copyright (C) 2017 Jeremy Gilbert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * If GPL 3 does not suit your needs, feel free to contact the author for
 * alternative licensing arrangements.
 *
 ********************************************************/


#define dDATALINK     1 << 1
#define dNETWORK      1 << 2
#define dSERVICE      1 << 3
#define dDCS 	      1 << 4
#define dERROR        1 << 5
#define dOBJFRAMEWORK 1 << 6
#define dWARNING      1 << 7
#define dSTATUSMSG      1 << 8
#define dMAILBOX      1 << 9
#define dALL          0xFFFFFFFF



#define DEBUGLEVEL dSTATUSMSG


// The following three macros are found throughout the code and implement an
// exceptionally lightweight conditional debugging framwork. When the
// DEBUGLEVEL mask is set to 0, the C++ compiler will strip these statements
// out entirely from the generated code thus creating ZERO overhead.
//
// DPR = print a single value
// DLN = print a single value with a newline
// DPF = printf

#define DPR( mask, X... )	    if( (mask & DEBUGLEVEL) > 0 ) { printDebug( X ); }
#define DLN( mask, X... )	    if( (mask & DEBUGLEVEL) > 0 ) { printDebugln( X ); }
#define DPF( mask, X... )	    if( (mask & DEBUGLEVEL) > 0 ) { char debugBuffer[255]; snprintf( debugBuffer, 255, X ); printDebug( debugBuffer ); }
#define HPR( mask, ptr, size )  if( (mask & DEBUGLEVEL) > 0 ) { hexPrint( ptr, size ) }



// globals.h

#define HWID_UNASSIGNED 0x0000

#define ADDR_UNASSIGNED 0x00
#define ADDR_BROADCAST  0xFF


// Faking it libraries

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


// Everything here is in the faking it category

uint16_t FAKEHARDWAREID = 0;

// Returns the 16 bit hardware ID. THis can be any value except
// HWID_UNASSIGNED.

uint16_t getHardwareID()
{
	if ( FAKEHARDWAREID != HWID_UNASSIGNED )
		return FAKEHARDWAREID;
	else
		return FAKEHARDWAREID + 1;
}



long long MICROSECOND_START = 0;
long long MICROSECOND_COUNTER = 0;

inline uint32_t millis()
{
	return MICROSECOND_COUNTER / 1000;
}

#include <sys/time.h>
#include <poll.h>

struct timeval stTimeVal;

inline long long getMicrosecondTime()
{
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * 1000000ll + stTimeVal.tv_usec;
}

void startMicrosecondCounter()
{
	MICROSECOND_START = getMicrosecondTime();
	MICROSECOND_COUNTER = 0;
}

void updateMicrosecondCounter()
{
	MICROSECOND_COUNTER = getMicrosecondTime() - MICROSECOND_START;
}


typedef bool boolean;

#ifndef INTERVAL_H
#define INTERVAL_H

class Interval {
public:
	Interval( uint32_t period );
	boolean hasPassed();
private:
	uint32_t _lastFired;
	uint32_t _period;
};

#endif


// Interval.cpp

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









// util.h

#define UI8(x) static_cast<uint8_t>(x)
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

void printDebugln( const char *s )
{
	printf( "%s\n", s );
}


void printDebugln( char *s )
{
	printf( "%s\n", s );
}

void printDebugln( )
{
	printf( "\n" );
}

void printDebugln( int i )
{
	printf( "%d\n", i );
}

#endif

inline void hexPrint( int mask, uint8_t *buffer, int size )
{
	for ( int i = 0 ; i < size ; i++ ) {
		uint8_t value = buffer[i];
		if (value < 0x10)
			DPR(mask, "0");
		DPR(mask, value, HEX);
		DPR(mask, " " );
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


// This is the parent of all packet types. The first bytes of all packet types
// are represented here. Different protocols can downcast to their own
// specific types of packets. This doesn't cost anything at runtime and means
// that the various sub-protocols can be more self-documenting.

struct PacketHeader {
	void clear() { dest = ADDR_UNASSIGNED; src = 0; destPort = 0; size = 0; };
	uint8_t dest; // The destination address (0=unassigned, 1=controller, FF=bcast)
	uint8_t src;  // The source address
	uint8_t destPort; // The destination port
	uint8_t size; // The size of the payload in bytes
};


// The makernet packet header is a common object used to "address" a packet.
// For maximimum effciency and economy of processor time and memory, this
// header is the direct byte order and contents of the first N bytes of all
// makernet messages. Note that the entire makernet message is actually
// sizeof( Packet ) + payload + 1 bytes for CRC

struct Packet : public PacketHeader {
	uint8_t payload[]; // The actual payload
};






#define MAILBOX_OP_FOURBYTE_DEFINITIVE 0x10

// A mailbox represents an extremely flexible place where values can be set
// and get. Any time a mailbox is set, it triggers a push to a remote mailbox.
// The mailbox object represents a single one of these item places, and
// Mailbox is the abstract interface thus lacks any implementation.
//
// The mailbox framework is a general purpose tool that is completely agnostic
// to packet or frame formats. It could easily exist outside of Makernet. The
// core interface to this "network agnostic" is that its caller will poll it
// for changes, and provide it a place to create a new message of its own
// format. The caller transports that message, treating the contents like a
// black box, to another Mailbox object which then applies the changes and
// brings the items in sync.
//


class Mailbox {
public:
	uint8_t *contents;
	const char *description;
	uint8_t size;
	uint8_t synchronized;

	uint8_t flags;

	virtual void reset();
	virtual void trigger() = 0;
	virtual int prepareUpdatePacket( uint8_t *buffer, int size ) = 0;
	virtual int handleUpdatePacket( uint8_t *buffer, int size ) = 0;
	virtual int handleAckPacket( uint8_t *buffer, int size ) = 0;
	virtual int hasPendingChanges() = 0;
};



// Small mailbox is an implementation of a mailbox of a small number of bytes
// like an RGB value or an integer.  It always uses opcode 0 to update
// (transmission of whole value) and it doesn't worry about version numbers.

#define MAILBOX_SMALLFORMAT_SIZE 4

class SmallMailbox : public Mailbox {

public:
	SmallMailbox(uint8_t configFlags, const char *d);
	uint8_t __contents[4];

	virtual void reset();
	virtual void trigger();
	virtual int prepareUpdatePacket( uint8_t *buffer, int size );
	virtual int handleUpdatePacket( uint8_t *buffer, int size );
	virtual int handleAckPacket( uint8_t *buffer, int size );
	virtual int hasPendingChanges();
	void setLong( uint32_t v );
	uint32_t getLong();
};

// MailboxDictionary implements an array of Mailbox objects and acts as a
// supervisor to each "entry".  The mailbox dictionary uses the same "network
// agnostic" calling interface as a mailbox object. The caller is expected to
// periodically call it for updates, then transmit the packet messages
// unchanged to the remote server where those changes will be applied.

#define MAX_DICT_ENTRIES 16

class MailboxDictionary {
public:
	MailboxDictionary();
	virtual void configure();
	Mailbox *mailboxes[MAX_DICT_ENTRIES];
	void set( int index, Mailbox& m );
	Mailbox *get( int index );
	int hasPendingChanges();
	Mailbox* nextMailboxWithChanges();
	int nextPendingMailboxIndex();
	int prepareUpdatePacket( uint8_t *buffer, uint8_t size );
	int handleUpdatePacket( uint8_t *buffer, uint8_t size );
	int handleAckPacket( uint8_t *buffer, uint8_t size );

};






void Mailbox::reset()
{
	// firstInvalidByte = 0;
	// lastInvalidByte = size;
	// version++;
	synchronized = 1;
	if ( contents != NULL and size > 0 )
		memset( contents, 0, size ) ;
}

SmallMailbox::SmallMailbox(uint8_t configFlags, const char *d)
{
	contents = __contents;
	flags = configFlags;
	size = 4;
	description = d;
	reset();
}

void SmallMailbox::reset()
{
	Mailbox::reset();
}

void SmallMailbox::trigger()
{
	synchronized = 0;
}

int SmallMailbox::hasPendingChanges()
{
	return !synchronized;
}


int SmallMailbox::prepareUpdatePacket( uint8_t *buffer, int size )
{
	if ( size < MAILBOX_SMALLFORMAT_SIZE + 2 )
		return -1;
	if ( buffer == NULL )
		return -2;

	buffer[0] = MAILBOX_OP_FOURBYTE_DEFINITIVE;
	buffer[1] = contents[0];
	buffer[2] = contents[1];
	buffer[3] = contents[2];
	buffer[4] = contents[3];

	return 5;
}

int SmallMailbox::handleAckPacket( uint8_t *buffer, int size )
{
	if ( size < 1)
		return -1;
	if ( buffer == NULL )
		return -2;

	if ( buffer[0] == MAILBOX_OP_FOURBYTE_DEFINITIVE ) {
		synchronized = 1;
	}


	DLN( dMAILBOX, "Ack received for mailbox!");

	return 0;
}

int SmallMailbox::handleUpdatePacket( uint8_t *buffer, int size )
{
	if ( size < 5)
		return -1;
	if ( buffer == NULL )
		return -2;

	if ( buffer[0] != MAILBOX_OP_FOURBYTE_DEFINITIVE )
		return -3;

	contents[0] = (uint8_t)(buffer[1]);
	contents[1] = (uint8_t)(buffer[2]);
	contents[2] = (uint8_t)(buffer[3]);
	contents[3] = (uint8_t)(buffer[4]);
	synchronized = 1;

	// Populate response

	buffer[0] = MAILBOX_OP_FOURBYTE_DEFINITIVE;

	DLN( dMAILBOX, "New value received for mailbox!");

	DPR( dMAILBOX, "&&&& MailboxChange: [");
	DPR( dMAILBOX, description );
	DPR( dMAILBOX, "] updated over network to: [");
	hexPrint( dMAILBOX, contents, 4 );
	DLN( dMAILBOX, "]");

	return 1;
}

void SmallMailbox::setLong( uint32_t v )
{
	contents[0] = (uint8_t)(v >> 24);
	contents[1] = (uint8_t)(v >> 16);
	contents[2] = (uint8_t)(v >> 8);
	contents[3] = (uint8_t)(v);
	synchronized = 0;


		DPR( dMAILBOX, "&&&& MailboxChange: [");
		DPR( dMAILBOX, description );
		DPR( dMAILBOX, "] set to: [");
		hexPrint( dMAILBOX, contents, 4 );
		DLN( dMAILBOX, "]");
	
}

uint32_t SmallMailbox::getLong()
{
	return ( (((uint32_t)contents[0]) << 24 ) |
	         (((uint32_t)contents[1]) << 16 ) |
	         (((uint32_t)contents[2]) << 8  ) |
	         (((uint32_t)contents[3]) ) );
}

MailboxDictionary::MailboxDictionary()
{
	for (int i = 0 ; i < MAX_DICT_ENTRIES ; i++ )
		mailboxes[i] = NULL;
}


void MailboxDictionary::set( int index, Mailbox& m )
{
	if ( index < MAX_DICT_ENTRIES )
		mailboxes[index] = &m;
}

Mailbox *MailboxDictionary::get( int index )
{
	if ( index < MAX_DICT_ENTRIES )
		return mailboxes[index];
	return NULL;
}

Mailbox* MailboxDictionary::nextMailboxWithChanges()
{
	for ( int i = 0 ; i < MAX_DICT_ENTRIES ; i++ )
		if ( mailboxes[i] != NULL )
			if ( mailboxes[i]->hasPendingChanges() )
				return mailboxes[i];
	return NULL;
}

int MailboxDictionary::hasPendingChanges()
{
	return ( nextMailboxWithChanges() != NULL );
}

int MailboxDictionary::nextPendingMailboxIndex()
{
	for ( int i = 0 ; i < MAX_DICT_ENTRIES ; i++ )
		if ( mailboxes[i] != NULL )
			if ( mailboxes[i]->hasPendingChanges() )
				return i;
	return -11;
}

int MailboxDictionary::prepareUpdatePacket( uint8_t *buffer, uint8_t size )
{
	int index = nextPendingMailboxIndex();
	if ( index < 0 )
		return 0;

	if ( size < 11 )
		return -11;
	if ( buffer == NULL )
		return -12;

	Mailbox *m = mailboxes[index];

	buffer[0] = index;
	return 1 + m->prepareUpdatePacket( buffer + 1, size - 1 );

}

int MailboxDictionary::handleUpdatePacket( uint8_t *buffer, uint8_t size )
{
	if ( size < 1 )
		return -8;
	if ( buffer == NULL )
		return -9;

	int index = buffer[0];

	if ( index >= MAX_DICT_ENTRIES )
		return -27;

	Mailbox *m = mailboxes[index];
	if ( m == NULL )
		return -10;

	return 1 + m->handleUpdatePacket( buffer + 1, size - 1 );
}

int MailboxDictionary::handleAckPacket( uint8_t *buffer, uint8_t size )
{
	if ( size < 1 )
		return -11;
	if ( buffer == NULL )
		return -12;

	int index = buffer[0];

	Mailbox *m = mailboxes[index];
	if ( m == NULL )
		return -13;

	return 1 + m->handleAckPacket( buffer + 1, size - 1 );
}

void MailboxDictionary::configure()
{
	
		DLN( dMAILBOX|dWARNING, "*** WARNING - MAILBOX BASE CONFIGURE CALLED, NO MAILBOXES SET UP");
	return;
}














// A "service" is a generic endpoint that can be a source, sink or both for
// packets. The makernet framework will pass handlePacket() to its registered
// services. Inside a handlePacket control flow, the handler is free to
// populate a response packet which will be immediately transmitted when the
// call is over.  It may also wait for the next polling event. If
// handlePacket() returns > 0, a new packet is replied immediately. Otherwise,
// no response is assumed unless the handler calls sendPacket.
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

class Service {

public:
	// Called when we are registered, do run-time setup here
	virtual void initialize() = 0;
	// Called when a packet is routed to us, return > 0 means a outgoing packet was generated
	virtual int handlePacket( Packet *p ) = 0;
	// Called with a scatch packet template, return 1 if a packet populated, 0 otherwise
	virtual int pollPacket( Packet *p ) = 0;
	virtual void loop() = 0;
	int port;

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

	enum { master, slave, peer } role;
	int address;
	Service *services[NUM_PORTS];

private:

	int routePacket( Packet *p );
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
	uint8_t address;

	// Service *services[16];

};


// This is a core singleton for the entire application. Right now, as an
// optimization, the singleton is used inside the Makernet framework itself
// so that each object is not forced to carry around pointers to the
// framework objects like Network and Datalink.
//
// Note that DeviceType will be embedded into structures that define network
// messages so its type needs be explicitly defined so it can be consistent
// across platforms. (C++11 actually does allow subclassing from a integer
// type but its unclear if this is supported across all makernet desired
// platforms

enum class DeviceType {
	Unassigned = 0,
	Controller = 1,
	Encoder = 2
} ;


class _Makernet {
public:
	Interval reportingInterval = Interval(5000);
	Network network;
	DeviceType deviceType;
	uint16_t hardwareID;
	uint16_t generation;

	void initialize();
	void loop();
};

Network::Network() {
	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		Service *s = services[i];
	}
}

void _Makernet::initialize()
{
	DLN( dOBJFRAMEWORK, "**** Makernet framwork init");
	generation = random();
	hardwareID = getHardwareID();
	network.initialize();
}

void _Makernet::loop()
{
	if ( reportingInterval.hasPassed() )
	{
		DPF( dSTATUSMSG, "+++ hwID[%d] type[%d] gen[%d] millis=[%u]\n", hardwareID, deviceType, generation, millis() );
	}
	network.loop();
}



// Singleton object
_Makernet Makernet;


// This structure defines the essential parameters of a device identity.

struct DeviceProfile
{
	bool connected = false;
	uint16_t hardwareID = HWID_UNASSIGNED;
	DeviceType deviceType = DeviceType::Unassigned;
	uint8_t address = ADDR_UNASSIGNED;
};


// Baseperipheral.h


// The base peripheral class defines a group of proxy objects that provide a
// clean programmer interface to makernet peripherals. The constructor and
// destructor have been designed to automatically maintain a global linked
// list of all object instances so that no additional action from the end-user
// is needed to handle dispatch and discovery. (I learned this pattern from EKT
// who uses it very effectively in her Modulo framework.)
//
// Remember the BasePeripherals are inherently proxy objects. They are OO
// stand-ins for real network devices.

class BasePeripheral {
public:
	// destructor, enables universal linked list tracking
	virtual ~BasePeripheral();
	// Constructor
	BasePeripheral(DeviceType deviceType);
	// Configure is called 1x at system configure time by the superclass and
	// is intended to be overridden
	virtual void configure();

	// Given a device network description, returns a proxy object if one exists
	static BasePeripheral *findPeripheralObjectForDevice( DeviceProfile *dp );

	// Look up a peripheral by a device ID
	// static BasePeripheral *findByDeviceID(uint16_t query);


	// Returns the device ID
	// uint16_t getDeviceID();


	// Internal tracking UUID
	long _uuid;

	// Contains the connection address details
	DeviceProfile connectedDevice;

private:

	// Internal init handler function
	void _init();

	// uint8_t _deviceType;
	// uint16_t _deviceID;
	// uint8_t _address;
	// bool _disconnected;

	DeviceType _deviceType;



	// Linked list of peripherals
	static BasePeripheral *_firstPeripheral;
	BasePeripheral *_nextPeripheral;
};


long uuid_gen = 0x10000;
BasePeripheral* BasePeripheral::_firstPeripheral = NULL;

BasePeripheral::BasePeripheral(DeviceType deviceType) :
	_deviceType(deviceType)
{

	// Add to our our linked list
	BasePeripheral **endPtr = &_firstPeripheral;
	while (*endPtr) {
		endPtr = &((*endPtr)->_nextPeripheral);
	}

	*endPtr = this;
	_uuid = uuid_gen++;
}


void BasePeripheral::_init() {
	DPR( dOBJFRAMEWORK, "Init called, object uuid=");
	DPR( dOBJFRAMEWORK, _uuid, HEX );
	DPR( dOBJFRAMEWORK, "  Type:");
	DLN( dOBJFRAMEWORK, );

	configure();
}

BasePeripheral::~BasePeripheral() {
	// Remove this peripheral from the global linked list
	BasePeripheral *prev = NULL;
	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral) {
		if (p == this) {
			if (prev) {
				prev->_nextPeripheral = _nextPeripheral;
			} else {
				_firstPeripheral = _nextPeripheral;
			}
		}
	}
}

void BasePeripheral::configure()
{
	DLN( dOBJFRAMEWORK, "Default BasePeripheral::configure() called");
}


// This function is called by the framework when a connected device requests
// address assignement. The key variables for the device are loaded into a
// deviceprofile object and passed to this function which then selects a base
// peripherals that should handle the connection.
//
// In current implementation, preference is given first to objects that
// previously mapped to this hardware ID, then to device type in general.

BasePeripheral *BasePeripheral::findPeripheralObjectForDevice( DeviceProfile *dp )
{
	// First look for hardware matches

	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral)
		if ( p->connectedDevice.hardwareID != HWID_UNASSIGNED and
		        dp->hardwareID == p->connectedDevice.hardwareID )
			return p;

	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral)
		if ( p->connectedDevice.connected == 0 and
		        p->_deviceType == dp->deviceType )
			return p;



	return NULL;

}





class EncoderPeripheral : public BasePeripheral {
public:
	EncoderPeripheral();


};

EncoderPeripheral::EncoderPeripheral() :
	BasePeripheral(DeviceType::Encoder)
{
}


EncoderPeripheral encoder;


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






class DeviceControlService : public Service {

public:
	virtual void initialize();
	virtual int handlePacket( Packet *p );
	virtual int pollPacket( Packet *p );
	virtual void loop();



private:

	Interval pollingTimer = Interval(1000);

};


void DeviceControlService::initialize()
{

}

#define DCS_REQUEST_ADDRESS 0x50
#define DCS_ASSIGN_ADDRESS 0x51
#define DCS_GENERAL_POLL 0x55

#define DCS_DEFAULT_PORT 0x00

// Payload of all generic device control messages

struct DeviceControlMessage {
	uint8_t command;
	uint8_t payload[];
};

// Payload of messages requesting an address

struct DCSAddressRequestMessage {
	uint8_t command;
	uint8_t deviceType;
	uint8_t hardwareID_H;
	uint8_t hardwareID_L;
} __attribute__((packed)); // magic to prevent compiler from aligning contents of struct

// Payload of messages setting an address

struct DCSAddressAssignMessage {
	uint8_t command;
	uint8_t address;
	uint8_t hardwareID_H;
	uint8_t hardwareID_L;
	int generation;
} __attribute__((packed));


// A structure used to describe the network properties of a device. Contains
// fields that are managed by the Device Control Service and can  be stored on
// peripherals or other addressable objects.

// struct DeviceDescriptor {
// 	uint16_t hardwareID; // immutable hardware ID
// 	uint8_t deviceType;
// 	uint8_t address;
// 	bool connected;
// 	int generation;
// };



// Address vending logic design
//
// In the current Makernet architecutre, devices only receive IDs when they
// are linked to a controll object on the controller. This is basically the
// entire connection architecture, and devices that cannot be linked will not
// get addresses.
//
// Periodically devices needing addresses emit a REQUEST_ADDRESS packet with
// some meta-data including their unique device ID. The controller on the
// network will assign them an available address by calling into the
// Peripheral framework with the metadata. The peripheral wanting a link will
// return its pointer and the address is assigned. A packet is emitted to make
// the assignment. If the packet is received, the remote device will define
// its network location to the new variable and all further peripheral data
// will now flow to the address issued completing the link. If the packet is
// dropped, subsequent REQUEST_ADDRESS requests will re-establish th



// Holds the next available address if there isn't one already Staring
// assignment at 0xA0 for ease of identification in packet dumps.

uint8_t nextAddressToVend = 0xA0;

// General incoming packet handler.
//
// Negative return values mean an error. Zero return values mean everything is
// fine. Positive return values have meaning TBD (could mean a reply is queued
// in the future.)



int DeviceControlService::handlePacket(Packet *p)
{

	if ( p->size < 1) {
		DLN( dDCS | dERROR, "Runt packet rejected");
		return -300;
	}


	DeviceControlMessage *dm = (DeviceControlMessage *)p->payload;

	DPR( dDCS, "DCS: handle packet, cmd=");
	DLN( dDCS, dm->command );

	if ( dm->command == DCS_REQUEST_ADDRESS && Makernet.network.role == Network::master ) {
		DLN( dDCS, "DCS: Req addr");
		if ( p->size > 1 ) {
			DCSAddressRequestMessage *msg = (DCSAddressRequestMessage *)p->payload;
			DeviceType type = (DeviceType)msg->deviceType;
			DPF( dDCS, "Assign address for type [%d]", type );
			DLN( dDCS );
			if ( Makernet.network.role == Network::master ) {
				DeviceProfile dd;
				dd.hardwareID = (uint16_t)msg->hardwareID_H << 8 | (uint16_t)msg->hardwareID_L;
				dd.deviceType = (DeviceType)msg->deviceType;
				BasePeripheral *proxy = BasePeripheral::findPeripheralObjectForDevice( &dd );
				if ( proxy == NULL ) {
					DPR( dDCS | dWARNING, "No proxy BasePeripheral found, dropping packet" );
					return 0;
				}
				int newAddress = proxy->connectedDevice.address;
				if ( newAddress == ADDR_UNASSIGNED ) {
					DPF( dDCS, "Assigning new address [%d] to uuid [%ld]", nextAddressToVend, proxy->_uuid );
					DLN( dDCS );
					newAddress = nextAddressToVend++;
				}
				proxy->connectedDevice.address = newAddress;
				proxy->connectedDevice.hardwareID = dd.hardwareID;
				proxy->connectedDevice.connected = 1;

				DPF( dDCS, "Link established uuid=[%ld] addr=[%d] hardwareid=[%x]\n",
				     proxy->_uuid,
				     proxy->connectedDevice.address,
				     proxy->connectedDevice.hardwareID
				   );

				// Generate address assignment message

				p->clear(); // clear the packet header

				p->dest = ADDR_BROADCAST;
				p->src = Makernet.network.address;
				p->destPort = DCS_DEFAULT_PORT;
				p->size = sizeof(DCSAddressAssignMessage);

				DCSAddressAssignMessage *msg = (DCSAddressAssignMessage *)p->payload;

				msg->command = DCS_ASSIGN_ADDRESS;
				msg->hardwareID_H = (uint8_t)(dd.hardwareID >> 8);
				msg->hardwareID_L = (uint8_t)(dd.hardwareID);
				msg->generation = Makernet.generation;
				msg->address = newAddress;

				return 1;
			}
		}
		return 0;
	}


	if ( dm->command == DCS_ASSIGN_ADDRESS and
	        Makernet.network.role == Network::slave and
	        Makernet.network.address == ADDR_UNASSIGNED ) {

		if ( p->size < sizeof(DCSAddressAssignMessage) )
			return 0;

		DCSAddressAssignMessage *msg = (DCSAddressAssignMessage *)p->payload;

		uint16_t hardwareID = (uint16_t)msg->hardwareID_H << 8 | (uint16_t)msg->hardwareID_L;

		if ( hardwareID != Makernet.hardwareID )
			return 0;

		uint8_t newAddress = msg->address;

		DPR( dDCS, "DCS: Accepting assignment of address=" );
		DPR( dDCS, newAddress );
		DLN( dDCS );

		Makernet.network.address = newAddress;

		return 0;
	}
	return 0;
}


int DeviceControlService::pollPacket(Packet *p)
{
	// Generate the general poll messages if I am the master

	if ( Makernet.network.role == Network::master )
		if ( pollingTimer.hasPassed() )
		{
			DLN( dDCS, "Time for a polling packet!");
			p->dest = ADDR_BROADCAST;
			p->destPort = 0;
			p->size = 1;
			p->payload[0] = DCS_GENERAL_POLL;
			return 1;
		}

	// If I'm a device waiting for an address assignment, generate a request

	if ( Makernet.network.role == Network::slave &&
	        Makernet.network.address == ADDR_UNASSIGNED )
		if ( pollingTimer.hasPassed() )
		{
			DLN( dDCS, "Time for a request packet!");
			p->dest = ADDR_BROADCAST;
			p->destPort = 0;
			p->size = sizeof( DCSAddressRequestMessage );
			DCSAddressRequestMessage *msg = (DCSAddressRequestMessage *)p->payload;
			msg->command = DCS_REQUEST_ADDRESS;
			msg->deviceType = (uint8_t)Makernet.deviceType;
			msg->hardwareID_H = (uint8_t)(Makernet.hardwareID >> 8);
			msg->hardwareID_L = (uint8_t)(Makernet.hardwareID);

			return 1;
		}

	return 0;
}

void DeviceControlService::loop()
{
// DLN( "Time for a polling packet!");

}



// The Makernet MailboxService is a flexible attribute-based notification and
// synchronization service that can serve a large variety of purposes. The
// core idea is to have a set of numbered mailboxes which remain in sync
// between two different nodes. These mailboxes are registered at
// configuration time, and there is the potential for mutliple different
// mailbox types which can implement custom semantics for network-efficient
// updates. To trigger a mailbox change, simply update the value using the OO
// syntax and the framework will trigger the update and sync packets.
//
// The MailboxService implements the basic routing for a mailbox, but it
// remains the responsibility of the caller to set up the contents of the
// mailboxes at configuration time.

class MailboxService : public Service {

public:
	virtual void initialize();
	virtual int handlePacket( Packet *p );
	virtual int pollPacket( Packet *p );
	virtual void loop();
};

void MailboxService::initialize()
{

}

int MailboxService::handlePacket( Packet *p )
{
	return -1;
}

int MailboxService::pollPacket( Packet *p )
{
	return -1;
}

void MailboxService::loop()
{

}









// UnixMaster implements a Datalink layer for the UNIX testbed. This code is
// not intended to run on MCUs so needs an #if statement surround it.

class UnixMaster : public Datalink {
public:
	virtual void initialize();
	virtual int sendFrame( uint8_t *inBuffer, uint8_t len );
	int loop();
	void processIncomingFrame();
	bool handleSTDIN( char *b, int s );
	typedef void (*UserCommandHandler)(char *cmd, int len );
	UserCommandHandler handleCommand = NULL;

private:
	struct sockaddr_un remote;
	int sock;

	// Buffer for the user command line
	char userCommandBuffer[1000];
	char *bpos = userCommandBuffer;
};


#define SOCK_PATH "/tmp/echo_socket"

void UnixMaster::initialize()
{

	int t, len;

	char str[100];

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	DLN( dDATALINK, "Trying to connect...");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(sock, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}

	DLN( dDATALINK, "Connected.");

}

typedef struct { uint8_t value[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; } ThunkMessage;

ThunkMessage thunk;

// Called by upper layers to send a frame.

int UnixMaster::sendFrame( uint8_t *inBuffer, uint8_t len )
{

	if (send(sock, &len, 1, 0) == -1) {
		perror("send");
		exit(1);
	}

	if (send(sock, inBuffer, len, 0) == -1) {
		perror("send");
		exit(1);
	}

	DPR( dDATALINK, "<<<< (" );
	DPR( dDATALINK, len );
	DPR( dDATALINK, ") ");
	hexPrint( dDATALINK, inBuffer, len );
	DLN( dDATALINK );

	// Emit the "thunk". If we are a master, we need to emulate the idea of
	// opening the bus for responses the same way that I2C works when the
	// master holds the clock line after its finished sending. This happens by
	// emitting a 5 byte FF signature.

	if ( Makernet.network.role == Network::master ) {
		int len = sizeof( thunk );

		if (send(sock, &len, 1, 0) == -1) {
			perror("send");
			exit(1);
		}
		if ( send(sock, (uint8_t *)&thunk, 5, 0) == -1 ) {
			perror("send");
			exit(1);
		}
	}

	return 0;
}

void UnixMaster::processIncomingFrame()
{
	int t;

	uint8_t frameLen;

	t = recv(sock, &frameLen, 1, MSG_WAITALL);

	if ( t <= 0 ) {
		if (t < 0)
			perror("recv");
		else
			printf("Server closed connection\n");
		exit(1);
	}

//	DPF( "Frame len:[%d]\n", frameLen );

	if ((t = recv(sock, frameBuffer, frameLen, MSG_WAITALL)) > 0) {
		frameBuffer[t] = '\0';

		DPR( dDATALINK, ">>>> (" );
		DPR( dDATALINK, t );
		DPR( dDATALINK, ") ");
		hexPrint( dDATALINK, frameBuffer, t );
		DLN( dDATALINK );

		// Intercept a master broadcast "thunk". This emulation is only
		// handled in cases where we are pretending to be a master/slave
		// network and the thunk triggers the poll.

		if ( t == 5 and Makernet.network.role == Network::slave )
			if ( memcmp( &thunk, frameBuffer, 5) == 0 )
			{
				for ( int i = 0 ; i < MAX_MAKERNET_FRAME_LENGTH ; i++ )
					frameBuffer[i] = 0;

				int n = Makernet.network.pollFrame( frameBuffer, n );
				if ( n > 0 )
					sendFrame( frameBuffer, n );

				return;
			}

		// Dispatch the frame up to the higher levels of the network

		if ( t > 0 )
			Makernet.network.handleFrame( frameBuffer, t );
	} else {
		if (t < 0)
			perror("recv");
		else
			printf("Server closed connection\n");
		exit(1);
	}
}

bool UnixMaster::handleSTDIN( char *b, int s )
{
	for ( int i = 0 ; i < s ; i++ )
	{
		if ( b[i] == '\n') {
			b[i] = '\0';
			printf( "*** Command from STDIN: [%s]\n", b );
			if ( handleCommand != NULL )
				handleCommand( b, s );
			return true;
		}
	}

	return false;

}

int UnixMaster::loop()
{
	struct pollfd fds[2];

	const int stdin = 0;

	fds[0].fd = stdin; /* this is STDIN */
	fds[0].events = POLLIN;
	fds[1].fd = sock; /* this is our socket */
	fds[1].events = POLLIN;

	if ( poll( fds, 2, 1000 ) > 0 ) {
		// Handle stdin
		if ( fds[0].revents & POLLIN ) {

			int r = read(stdin, bpos, 1000 + userCommandBuffer - bpos);
			// printf( "**** read from STDIN: [%d] bytes\n", r );
			bpos += r;
			if ( handleSTDIN( userCommandBuffer, bpos - userCommandBuffer )) {
				// DLN( dDATALINK, "reset" );
				bpos = userCommandBuffer;
			}
		}

		// Handle network input
		if ( fds[1].revents & POLLIN )
			processIncomingFrame();
	}
	return 0;
}





//#define MASTER





#ifdef MASTER





int main(void)
{
	FAKEHARDWAREID = 0x8877;

	DeviceControlService dcs;
	Makernet.network.role = Network::master;
	UnixMaster um;

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(DCS_DEFAULT_PORT, &dcs);

	Makernet.initialize();

	startMicrosecondCounter();

	while (1)
	{
		um.loop();
		Makernet.loop();
		updateMicrosecondCounter();
	}
}

#else // SLAVE

int main(void)
{
	DeviceControlService dcs;
	MailboxService ms;
	Makernet.network.role = Network::slave;
	UnixMaster um;

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(0, &dcs);

	Makernet.deviceType = DeviceType::Encoder;

	Makernet.initialize();

	startMicrosecondCounter();

	while (1)
	{
		um.loop();
		Makernet.loop();
		updateMicrosecondCounter();
	}


}

#endif

