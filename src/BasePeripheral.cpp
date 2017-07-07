/********************************************************
 ** 
 **  BasePeripheral.cpp
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#include <BasePeripheral.h>
#include <Debug.h>


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


// Static

void BasePeripheral::initializeAllPeripherals()
{
	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral)
		p->_init();
}

void BasePeripheral::busResetAllPeripherals()
{
	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral) {
		p->busReset();
		
		// Not sure what else needs to go here..? Do we force a disconnect and reconnect?
		// p->deviceProfile.address = ADDR_UNASSIGNED;
	}
}



void BasePeripheral::busReset()
{

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
		if ( // p->connectedDevice.hardwareID != HWID_UNASSIGNED and
		        dp->hardwareID == p->connectedDevice.hardwareID )
			return p;

	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral)
		if ( p->connectedDevice.connected == 0 and
		        p->_deviceType == dp->deviceType )
			return p;



	return NULL;

}