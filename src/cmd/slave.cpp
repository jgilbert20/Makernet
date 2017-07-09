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

DeviceControlService dcs;
MailboxService ms;
UNIXSocketDatalink um;
auto sm = SmallMailbox( 0, "Test mailbox" );

DeviceProfile dp;

int main(int argc, const char * argv[])
{
	FAKEHARDWAREID = 0x4488;
	Makernet.network.role = Network::slave;

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(PORT_DCS, &dcs);
	Makernet.network.registerService(PORT_MAILBOX, &ms);

	ms.endpoint = &dp; 
	ms.endpoint->address = 10;

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
