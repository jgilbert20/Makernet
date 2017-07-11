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


MailboxService ms;
UNIXSocketDatalink um;
auto sm = SmallMailbox( 0, "Test mailbox" );

DeviceProfile dp;

void handleCommand(char *cmd, int len );

int main(int argc, const char * argv[])
{
	FAKEHARDWAREID = 0x4488;
	Makernet.network.role = Network::slave;

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(PORT_MAILBOX, &ms);

	ms.set( 0, sm );
	ms.endpoint = &dp; 
	ms.endpoint->address = 10;

	Makernet.deviceType = DeviceType::Encoder;

	Makernet.initialize();

	startMicrosecondCounter();

	um.handleCommand = handleCommand;

	while (1)
	{
		um.loop();
		Makernet.loop();
		updateMicrosecondCounter();
	}
}

void handleCommand(char *cmd, int len )
{
	printf( "===--> %s\n", cmd );
	int v = atoi(cmd);
	printf( "Set mailbox to: %i\n", v );
	sm.setLong( v );
}

#endif
