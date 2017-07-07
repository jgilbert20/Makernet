/********************************************************
 ** 
 **  BasePeripheral.h
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#ifndef BASEPERIPHERAL_H
#define BASEPERIPHERAL_H


#include <Packet.h>
#include <Network.h>
#include <MakernetSingleton.h>


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
	static void initializeAllPeripherals();

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

#endif
