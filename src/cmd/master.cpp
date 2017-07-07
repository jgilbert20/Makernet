/********************************************************
 ** 
 **  master.cpp
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/

#ifndef ARDUINO //prevent ArduinoIDE from trying to compile this

#include <UnixArduino.h>
#include <UNIXSocketDatalink.h>
#include <Makernet.h>

EncoderPeripheral encoder;

int main(void)
{
	FAKEHARDWAREID = 0x8877;

	DeviceControlService dcs;
	Makernet.network.role = Network::master;
	UNIXSocketDatalink um;

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(DCS_DEFAULT_PORT, &dcs);

	Makernet.initialize();

	startMicrosecondCounter();

	while (1)
	{
		um.loop();
		Makernet.loop();
		updateMicrosecondCounter();
	}
}

#endif