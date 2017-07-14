/********************************************************
 **
 **  MakernetSingleton.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#include <MakernetSingleton.h>
#include <Debug.h>
#include <ArduinoAPI.h>
#include <BasePeripheral.h>
#include <MailboxService.h>
#include <I2CDatalink.h>

#ifdef ARDUINO
I2CDatalink i2cDatalink;
#endif 

void _Makernet::initialize( DeviceType d, MailboxService &ms )
{
	ms.defaultEndpoint = &controller;
	ms.defaultEndpoint->address = ADDR_CONTROLLER;

	network.registerService( PORT_MAILBOX, &ms );

	initialize( d );
}

// Framework users should call this once during program setup.

void _Makernet::initialize( DeviceType d )
{
	DLN( dOBJFRAMEWORK, "**** Makernet framework init");
	generation = getRandomNumber16();
	hardwareID = getHardwareID();

	Makernet.deviceType = d;

	if ( d == DeviceType::Controller ) {
		Makernet.network.address = ADDR_CONTROLLER;
		Makernet.network.role = Network::master;
	}
	else
	{
		Makernet.network.address = ADDR_UNASSIGNED;
		Makernet.network.role = Network::slave;
	}

#ifdef ARDUINO
	Makernet.network.useDatalink( &i2cDatalink );
#endif

	network.initialize();

#if CONTROLLER_SUPPORT
	BasePeripheral::initializeAllPeripherals();
#endif
	// Issue our first bus reset, the first one goes internally
	busReset();

	if ( CONTROLLER_SUPPORT and network.role == Network::master )
		issueBusReset();

}

// busReset() is a special verb that is intended to reset all state around the
// entire network. When this is called, every object should assume all
// information it has about other devices in the network is potentially
// wrong.. clear buffers, interrupt work in progress, and wait for new
// synchronization data to restore state. This is called exactly once at the
// very end of initialization (so its guarnenteed to be called at the start of
// the program), and then again when requested by the controller over the
// network.

void _Makernet::busReset()
{
	DLN( dRESET, "Makernet: Full bus reset" );
	network.busReset();

#if CONTROLLER_SUPPORT
	BasePeripheral::busResetAllPeripherals();
#endif

}

void _Makernet::issueBusReset()
{
	DLN( dRESET, "Issuing global bus reset to network!" );

	network.issueBusReset();
}

void _Makernet::loop()
{
	if ( reportingInterval.hasPassed() ){
//		DPF( dSTATUSMSG, "+++ STATUS: hwID[%d] type[%d] gen[%d] millis=[%u]\n", hardwareID, deviceType, generation, millis() );

		DPR( dSTATUSMSG, "+++ STATUS: hwID[");
		DPR( dSTATUSMSG, hardwareID );
		DPR( dSTATUSMSG, "] millis=[");
		DPR( dSTATUSMSG, millis() );
		DPR( dSTATUSMSG, "] netAddress=[");
		DPR( dSTATUSMSG, network.address );
		DPR( dSTATUSMSG, "]");
		DLN( dSTATUSMSG );

	}

	network.loop();
}



// Singleton object
_Makernet Makernet;

