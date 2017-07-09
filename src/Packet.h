/********************************************************
 ** 
 **  Packet.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#ifndef PACKET_H
#define PACKET_H

#include <Types.h>
#include <Globals.h>

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

#define MAX_PACKET_PAYLOAD_SIZE (MAX_MAKERNET_FRAME_LENGTH - sizeof(PacketHeader))

// The makernet packet header is a common object used to "address" a packet.
// For maximimum effciency and economy of processor time and memory, this
// header is the direct byte order and contents of the first N bytes of all
// makernet messages. Note that the entire makernet message is actually
// sizeof( Packet ) + payload + 1 bytes for CRC

struct Packet : public PacketHeader {
	uint8_t payload[]; // The actual payload
};

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
// This structure defines the essential parameters of a device identity.

struct DeviceProfile
{
	bool connected = false;
	uint16_t hardwareID = HWID_UNASSIGNED;
	DeviceType deviceType = DeviceType::Unassigned;
	uint8_t address = ADDR_UNASSIGNED;
};



#endif