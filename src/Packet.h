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


// The makernet packet header is a common object used to "address" a packet.
// For maximimum effciency and economy of processor time and memory, this
// header is the direct byte order and contents of the first N bytes of all
// makernet messages. Note that the entire makernet message is actually
// sizeof( Packet ) + payload + 1 bytes for CRC

struct Packet : public PacketHeader {
	uint8_t payload[]; // The actual payload
};


#endif