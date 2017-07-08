/********************************************************
 ** 
 **  MailboxService.cpp
 ** 
 **  Part of the Makernet framework by Jeremy Gilbert
 ** 
 **  License: GPL 3
 **  See footer for copyright and license details.
 ** 
 ********************************************************/


#include <MailboxService.h>

void MailboxService::initialize()
{

}

int MailboxService::handlePacket( Packet *p )
{
	return -1;
}

int MailboxService::pollPacket( Packet *p )
{
	// if( mailboxDict->hasPendingChanges() ) {
	// 	p->clear();
	// 	p->dest = endpoint->address;
	// 	p->src = Network::address;
	// 	p->destPort = PORT_MAILBOX;
	// 	p->size = mailboxDict->prepareUpdatePacket( p->payload, MAX_MAKERNET_PACKET_LEN );
	// }

	return -1;
}

void MailboxService::loop()
{

}

void MailboxService::busReset()
{

}
