/********************************************************
 **
 **  DeviceControlServce.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#include <DeviceControlService.h>
#include <Debug.h>
#include <BasePeripheral.h>



// Called at configure time
void DeviceControlService::initialize()
{
}




#if CONTROLLER_SUPPORT

// Holds the next available address if there isn't one already Staring
// assignment at 0xA0 for ease of identification in packet dumps.

uint8_t nextAddressToVend = 0xA0;

#endif


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

#if CONTROLLER_SUPPORT
	if ( dm->command == DCS_REQUEST_ADDRESS && Makernet.network.role == Network::master ) {
		DLN( dDCS, "DCS: Req addr");
		if ( p->size > 1 ) {
			DCSAddressRequestMessage *msg = (DCSAddressRequestMessage *)p->payload;
			DeviceType type = (DeviceType)msg->deviceType;
			DPF( dDCS, "DCS: Asked to assign address for type [%d]", type );
			DLN( dDCS );
			if ( Makernet.network.role == Network::master ) {
				DeviceProfile dd;
				dd.hardwareID = (uint16_t)msg->hardwareID_H << 8 | (uint16_t)msg->hardwareID_L;
				dd.deviceType = (DeviceType)msg->deviceType;
				BasePeripheral *proxy = BasePeripheral::findPeripheralObjectForDevice( &dd );
				if ( proxy == NULL ) {
					DPR( dDCS | dWARNING, "DCS-WARN: No proxy BasePeripheral found for device requesting address, dropping packet type=" );
					DLN( dDCS | dWARNING, (int)type );
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
#endif

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

		// Makernet.network.controller.connected = true;
		// Makernet.network.controller.deviceType = DeviceType::Controller;
		// Makernet.network.controller.address = 0;

		Makernet.network.address = newAddress;

		return 0;
	}
	return 0;
}


int DeviceControlService::pollPacket(Packet *p)
{
	// Generate the general poll messages if I am the master

	if ( CONTROLLER_SUPPORT && Makernet.network.role == Network::master )
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

// void DeviceControlService::loop()
// {
// 	// DLN( dALL, "DCS Loop handler");
// }

void DeviceControlService::busReset()
{
	// DLN( dALL, "DCS Loop handler");
}

