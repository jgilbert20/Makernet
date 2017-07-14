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
	configure();
}

void MailboxService::configure()
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

void MailboxService::attachObserver( IMailboxObserver *obs)
{
	for ( int i = 0 ; i < MAX_MAILBOXS_PER_SERVICE ; i++ )
		if ( mailboxes[i] != NULL )
			mailboxes[i]->observer = obs; 
}

void MailboxService::pointPacketToEndpoint( Packet *p )
{
	if ( defaultEndpoint == NULL ) {
		DLN( dNETWORK, "ERROR: packet cannot be generated properly, endpoint not configured!" );
		return;
	}

	p->clear();
	p->src = Makernet.network.address;
	p->dest = defaultEndpoint->address;
	p->destPort = PORT_MAILBOX;

	DPR( dNETWORK, "Point: ");
	hexPrint( dNETWORK, (uint8_t *)p, 25 );
	DLN( dNETWORK );

	DPF( dNETWORK, "Src = %d\n", p->dest );

}


int MailboxService::pollPacket( Packet *p )
{
	// DLN( dMAILBOX|dPOLL, "POLL: Mailbox poll");
	// Check if we have a valid destination configured

	if ( defaultEndpoint == NULL or defaultEndpoint->address == ADDR_UNASSIGNED ) {
		DLN( dNETWORK, "WARNING: mailbox disabled due to defaultEnpoint not being set" );
		DPF( dNETWORK, "defaultEndpoint null =%d\n", defaultEndpoint == NULL );
		if ( defaultEndpoint != NULL )
			DPF( dNETWORK, "defaultEndpoint addr =%d\n", defaultEndpoint->address );
		return 0;
	}

	int nextMailbox = nextPendingMailboxIndex();
	if ( nextMailbox >= 0 ) {
		DPF( dMAILBOX, "Mailbox changed %d\n", nextMailbox );
		pointPacketToEndpoint( p );
		MailboxUpdateMessage *msg = (MailboxUpdateMessage *)p->payload;
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
		return -4500;

	// Check if we have a valid destination configured

	if ( defaultEndpoint == NULL or defaultEndpoint->address == ADDR_UNASSIGNED ) {
		DPF( dMAILBOX | dERROR, "handleMessage dropped packet, no endpoint configured" );
		return -4502;
	}

	MailboxUpdateMessage *msg = (MailboxUpdateMessage *)p->payload;

	// DPF( dMAILBOX, "Incoming mailbox packet src[%d] size[%d] mailbox[%d]\n", p->src, p->size, msg->mailbox );

	uint8_t i = msg->mailbox;
	if ( i < 0 or i >= MAX_MAILBOXS_PER_SERVICE )
		return -4501;

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
	DLN( dMAILBOX | dRESET, "MailboxService: Bus Reset...");

	for ( int i = 0 ; i < MAX_MAILBOXS_PER_SERVICE ; i++ )
		if ( mailboxes[i] != NULL ) {
			mailboxes[i]->busReset();
			DPF( dMAILBOX, "MailboxService: Reset mailbox %d\n", i );
		}
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

