#ifndef MAKERNET_H
#define MAKERNET_H

#include <Packet.h>
#include <Interval.h>
#include <Network.h>
#include <Service.h>
#include <DeviceControlService.h>
#include <MailboxService.h>

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

extern _Makernet Makernet;


// This structure defines the essential parameters of a device identity.

struct DeviceProfile
{
	bool connected = false;
	uint16_t hardwareID = HWID_UNASSIGNED;
	DeviceType deviceType = DeviceType::Unassigned;
	uint8_t address = ADDR_UNASSIGNED;
};



#endif
