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

int main(int argc, const char * argv[])
{
	DeviceControlService dcs;
	MailboxService ms;
	Makernet.network.role = Network::slave;
	UNIXSocketDatalink um;

	EncoderMailboxDictionary emd;
	emd.configure();

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(PORT_DCS, &dcs);

	Makernet.deviceType = DeviceType::Encoder;

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
