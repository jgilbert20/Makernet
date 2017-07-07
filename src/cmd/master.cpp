#include <UnixArduino.h>
#include <UNIXSocketDatalink.h>

#include <Makernet.h>

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