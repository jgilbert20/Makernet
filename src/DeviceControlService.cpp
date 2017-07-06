#include <DeviceControlService.h>
#include <Debug.h>
#include <BasePeripheral.h>



// Called at configure time
void DeviceControlService::initialize()
{

}



// Address vending logic design
//
// In the current Makernet architecutre, devices only receive IDs when they
// are linked to a controll object on the controller. This is basically the
// entire connection architecture, and devices that cannot be linked will not
// get addresses.
//
// Periodically devices needing addresses emit a REQUEST_ADDRESS packet with
// some meta-data including their unique device ID. The controller on the
// network will assign them an available address by calling into the
// Peripheral framework with the metadata. The peripheral wanting a link will
// return its pointer and the address is assigned. A packet is emitted to make
// the assignment. If the packet is received, the remote device will define
// its network location to the new variable and all further peripheral data
// will now flow to the address issued completing the link. If the packet is
// dropped, subsequent REQUEST_ADDRESS requests will re-establish th



// Holds the next available address if there isn't one already Staring
// assignment at 0xA0 for ease of identification in packet dumps.

uint8_t nextAddressToVend = 0xA0;

// General incoming packet handler.
//
// Negative return values mean an error. Zero return values mean everything is
// fine. Positive return values have meaning TBD (could mean a reply is queued
// in the future.)



int DeviceControlService::handlePacket(Packet *p)
{

	if ( p->size < 1) {
		DLN( dDCS | dERROR, "Runt packet rejected");
		return -300;
	}


	DeviceControlMessage *dm = (DeviceControlMessage *)p->payload;

	DPR( dDCS, "DCS: handle packet, cmd=");
	DLN( dDCS, dm->command );

	if ( dm->command == DCS_REQUEST_ADDRESS && Makernet.network.role == Network::master ) {
		DLN( dDCS, "DCS: Req addr");
		if ( p->size > 1 ) {
			DCSAddressRequestMessage *msg = (DCSAddressRequestMessage *)p->payload;
			DeviceType type = (DeviceType)msg->deviceType;
			DPF( dDCS, "Assign address for type [%d]", type );
			DLN( dDCS );
			if ( Makernet.network.role == Network::master ) {
				DeviceProfile dd;
				dd.hardwareID = (uint16_t)msg->hardwareID_H << 8 | (uint16_t)msg->hardwareID_L;
				dd.deviceType = (DeviceType)msg->deviceType;
				BasePeripheral *proxy = BasePeripheral::findPeripheralObjectForDevice( &dd );
				if ( proxy == NULL ) {
					DPR( dDCS | dWARNING, "No proxy BasePeripheral found, dropping packet" );
					return 0;
				}
				int newAddress = proxy->connectedDevice.address;
				if ( newAddress == ADDR_UNASSIGNED ) {
					DPF( dDCS, "Assigning new address [%d] to uuid [%ld]", nextAddressToVend, proxy->_uuid );
					DLN( dDCS );
					newAddress = nextAddressToVend++;
				}
				proxy->connectedDevice.address = newAddress;
				proxy->connectedDevice.hardwareID = dd.hardwareID;
				proxy->connectedDevice.connected = 1;

				DPF( dDCS, "Link established uuid=[%ld] addr=[%d] hardwareid=[%x]\n",
				     proxy->_uuid,
				     proxy->connectedDevice.address,
				     proxy->connectedDevice.hardwareID
				   );

				// Generate address assignment message

				p->clear(); // clear the packet header

				p->dest = ADDR_BROADCAST;
				p->src = Makernet.network.address;
				p->destPort = DCS_DEFAULT_PORT;
				p->size = sizeof(DCSAddressAssignMessage);

				DCSAddressAssignMessage *msg = (DCSAddressAssignMessage *)p->payload;

				msg->command = DCS_ASSIGN_ADDRESS;
				msg->hardwareID_H = (uint8_t)(dd.hardwareID >> 8);
				msg->hardwareID_L = (uint8_t)(dd.hardwareID);
				msg->generation = Makernet.generation;
				msg->address = newAddress;

				return 1;
			}
		}
		return 0;
	}


	if ( dm->command == DCS_ASSIGN_ADDRESS and
	        Makernet.network.role == Network::slave and
	        Makernet.network.address == ADDR_UNASSIGNED ) {

		if ( p->size < sizeof(DCSAddressAssignMessage) )
			return 0;

		DCSAddressAssignMessage *msg = (DCSAddressAssignMessage *)p->payload;

		uint16_t hardwareID = (uint16_t)msg->hardwareID_H << 8 | (uint16_t)msg->hardwareID_L;

		if ( hardwareID != Makernet.hardwareID )
			return 0;

		uint8_t newAddress = msg->address;

		DPR( dDCS, "DCS: Accepting assignment of address=" );
		DPR( dDCS, newAddress );
		DLN( dDCS );

		Makernet.network.address = newAddress;

		return 0;
	}
	return 0;
}


int DeviceControlService::pollPacket(Packet *p)
{
	// Generate the general poll messages if I am the master

	if ( Makernet.network.role == Network::master )
		if ( pollingTimer.hasPassed() )
		{
			DLN( dDCS, "Time for a polling packet!");
			p->dest = ADDR_BROADCAST;
			p->destPort = 0;
			p->size = 1;
			p->payload[0] = DCS_GENERAL_POLL;
			return 1;
		}

	// If I'm a device waiting for an address assignment, generate a request

	if ( Makernet.network.role == Network::slave &&
	        Makernet.network.address == ADDR_UNASSIGNED )
		if ( pollingTimer.hasPassed() )
		{
			DLN( dDCS, "Time for a request packet!");
			p->dest = ADDR_BROADCAST;
			p->destPort = 0;
			p->size = sizeof( DCSAddressRequestMessage );
			DCSAddressRequestMessage *msg = (DCSAddressRequestMessage *)p->payload;
			msg->command = DCS_REQUEST_ADDRESS;
			msg->deviceType = (uint8_t)Makernet.deviceType;
			msg->hardwareID_H = (uint8_t)(Makernet.hardwareID >> 8);
			msg->hardwareID_L = (uint8_t)(Makernet.hardwareID);

			return 1;
		}

	return 0;
}

void DeviceControlService::loop()
{
// DLN( "Time for a polling packet!");

}

