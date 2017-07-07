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

void _Makernet::initialize()
{
	DLN( dOBJFRAMEWORK, "**** Makernet framework init");
	generation = getRandomNumber16();
	hardwareID = getHardwareID();
	network.initialize();
//	BasePeripheral::initializeAllPeripherals();
}

void _Makernet::loop()
{
	if ( reportingInterval.hasPassed() )
		DPF( dSTATUSMSG, "+++ STATUS +++ hwID[%d] type[%d] gen[%d] millis=[%lu]\n", hardwareID, deviceType, generation, millis() );

	network.loop();
}



// Singleton object
_Makernet Makernet;

