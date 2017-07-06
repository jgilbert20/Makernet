#ifndef DCS_H
#define DCS_H

#include <Types.h>
#include <Service.h>
#include <Packet.h>
#include <Interval.h>

class DeviceControlService : public Service {

public:
	virtual void initialize();
	virtual int handlePacket( Packet *p );
	virtual int pollPacket( Packet *p );
	virtual void loop();



private:

	Interval pollingTimer = Interval(1000);

};



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


#endif
