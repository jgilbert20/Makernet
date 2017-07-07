#include <UnixArduino.h>
#include <UNIXSocketDatalink.h>
#include <Makernet.h>


int main(void)
{
	DeviceControlService dcs;
	MailboxService ms;
	Makernet.network.role = Network::slave;
	UNIXSocketDatalink um;

	Makernet.network.useDatalink( &um );
	Makernet.network.registerService(0, &dcs);

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