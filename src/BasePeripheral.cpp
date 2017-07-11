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

#if CONTROLLER_SUPPORT

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

	for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
		services[i] = NULL;
	}
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
		p->connectedDevice.reset();


		// Issue bus reset to all services if any
		for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
			Service *s = p->services[i];
			if ( s != NULL )
				s->busReset();
		}

		p->busReset();

		// Not sure what else needs to go here..? Do we force a disconnect and reconnect?
		// p->deviceProfile.address = ADDR_UNASSIGNED;
	}
}



void BasePeripheral::busReset()
{
	DLN( dOBJFRAMEWORK, "Base peripheral got a bus request..");
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


int BasePeripheral::registerService( int port, Service* s )
{
	if ( API_CHECK && (port >= NUM_PORTS or port < 1 )) {
		DPR( dOBJFRAMEWORK | dERROR, "Invalid service registration (note, service 0 is reserved for DCS!)");
		return -1101;
	}
	if ( API_CHECK && s == NULL ) {
		DPR( dOBJFRAMEWORK | dERROR, "NULL service registration");
		return -1102;
	}

	s->port = port;
	services[port] = s;
	s->defaultEndpoint = &connectedDevice;
	s->initialize();
	return 1;
}


int BasePeripheral::pollPacket( Packet *p )
{
	DLN( dNETWORK, "BP:pollPacket()");
	for (BasePeripheral *bp = _firstPeripheral; bp != NULL ; bp = bp->_nextPeripheral)
		for ( int i = 0 ; i < NUM_PORTS ; i++ ) {
			Service *s = bp->services[i];
			if ( s != NULL ) {
				int retValue = s->pollPacket(p);
				if ( retValue > 0 )
					return retValue;
				if ( retValue < 0 )
					DPR( dNETWORK, "WARNING: Poll-packet returned negative, something wrong...");
			}
		}
	return 0;
}

// Called by the framework to find the base peripheral corresponding to a
// numeric address, otherwise NULL

BasePeripheral *BasePeripheral::findByAddress( uint8_t address )
{
	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral)
		if ( address == p->connectedDevice.address )
			return p;
	return NULL;
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
		if ( dp->hardwareID == p->connectedDevice.hardwareID )
			return p;

	for (BasePeripheral *p = _firstPeripheral; p != NULL ; p = p->_nextPeripheral)
		if ( p->connectedDevice.connected == 0 and
		        p->_deviceType == dp->deviceType )
			return p;

	return NULL;
}

#endif