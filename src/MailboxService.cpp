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



// The MailboxService controlls a collection of Mailboxes which can be used to
// transfer and preserve state across the network in an efficient and low-
// maintenance fashion. To use it, instantiate a series of Mailboxes
// somewhere in your code. Each mailbox has an ID, a type and a description



// 
// When the mailbox service is polled, it looks for mailboxes that have updated state
// and generates packets


// A mailbox represents an extremely flexible place where values can be set
// and get by multiple nodes on a network. Any time a mailbox is set, it
// triggers a push to a remote mailbox. Remote mailboxes can easily be used to
// convey all sorts of information and configuration to and from a device.
// What's desirable about this arrangement is that a device can be fully reset
// and then recover its "configuration" when reconnection occurs. Furthermore,
// a mailbox can act as a trigger point, allowing one-shot events like button
// up or down events, to be communicated reliably (and one-time only) to a
// controller.


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


