#include <MakernetSingleton.h>
#include <Debug.h>
#include <ArduinoAPI.h>

void _Makernet::initialize()
{
	DLN( dOBJFRAMEWORK, "**** Makernet framework init");
	generation = random();
	hardwareID = getHardwareID();
	network.initialize();
//	BasePeripheral::initializeAllPeripherals();
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

