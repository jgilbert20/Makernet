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

UNIXSocketDatalink um;

void handleCommand(char *cmd, int len );

int main(int argc, const char * argv[])
{
	FAKEHARDWAREID = 0x8877;
	Makernet.network.useDatalink( &um );

	Makernet.initialize( DeviceType::Controller );

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
	encoder.encoderMailboxSvc.position.setLong( v );
}


#endif