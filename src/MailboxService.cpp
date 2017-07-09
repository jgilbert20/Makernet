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
// maintenance fashion.
// A mailbox represents an extremely flexible place where values can be set
// and get by multiple nodes on a network. Any time a mailbox is set, it
// triggers a push to a remote mailbox. Remote mailboxes can easily be used to
// convey all sorts of information and configuration to and from a device.
// What's desirable about this arrangement is that a device can be fully reset
// and then recover its "configuration" when reconnection occurs. Furthermore,
// a mailbox can act as a trigger point, allowing one-shot events like button
// up or down events, to be communicated reliably (and one-time only) to a
// controller.



// To use it, subclass the MailboxService add some logic
// in configure() to add some mailboxes and register them somewhere. Subclassing
// isn't strictly necessary, but is a nice way to provide a clean allocation
// and reference structure for the mailboxes you care about.
//
// When the mailbox service is polled, it looks for mailboxes that have
// locally updated state and generates packets to broadcast the new values.
// The default recipient of the update messages is a DeviceProfile object
// called endPoint. The mailbox service pauses for a configurable delay for
// unackwnloledged packets.
//


#include <MailboxService.h>
#include <Mailbox.h>
#include <Makernet.h>
#include <Debug.h>
#include <Types.h> // needed for NULL apparently!

void MailboxService::initialize()
{

}


int MailboxService::nextPendingMailboxIndex()
{
	for ( int i = 0 ; i < MAX_MAILBOXS_PER_SERVICE ; i++ )
		if ( mailboxes[i] != NULL )
			if ( mailboxes[i]->hasPendingChanges() )
				return i;
	return -11;
}

void MailboxService::pointPacketToEndpoint( Packet *p )
{
	p->src = Makernet.network.address;
	p->dest = endpoint->address;
	p->destPort = PORT_MAILBOX;
}


int MailboxService::pollPacket( Packet *p )
{
	int nextMailbox = nextPendingMailboxIndex();
	if ( nextMailbox > 0 ) {
		pointPacketToEndpoint( p );
		MailboxUpdateMessage *msg = (MailboxUpdateMessage *)p;
		msg->mailbox = nextMailbox;
		int size = mailboxes[nextMailbox]->generateMessage( msg->payload, MAX_MAILBOX_MESSAGE_SIZE );
		if ( size <= 0 ) {
			DPF( dMAILBOX | dERROR, "generateMessage error [%d] on mailbox [%d]", nextMailbox, size );
			return 0;
		}
		p->size = sizeof( MailboxUpdateMessage ) + size;
		return 1;
	}
	return 0;
}


int MailboxService::handlePacket( Packet *p )
{
	if ( p->size <= sizeof(MailboxUpdateMessage) )
		return -123;

	MailboxUpdateMessage *msg = (MailboxUpdateMessage *)p;

	int i = msg->mailbox;
	if( i < MAX_MAILBOXS_PER_SERVICE )
		return -200;

	if ( mailboxes[i] != NULL ) {
		int rsize = mailboxes[i]->handleMessage( msg->payload, p->size - sizeof(MailboxUpdateMessage) );
		if ( rsize <= 0 ) {
			if ( rsize < 0 )
				DPF( dMAILBOX | dERROR, "handleMessage error [%d] on mailbox [%d]", i, rsize );
			return 0;
		}
		pointPacketToEndpoint( p );
		p->size = sizeof( MailboxUpdateMessage ) + rsize;
		return 1;
	}

	return 0;
}


void MailboxService::busReset()
{
	// TBD
}


void MailboxService::set( int index, Mailbox& m )
{
	if ( index < MAX_MAILBOXS_PER_SERVICE )
		mailboxes[index] = &m;
}

Mailbox *MailboxService::get( int index )
{
	if ( index < MAX_MAILBOXS_PER_SERVICE )
		return mailboxes[index];
	return NULL;
}

