/********************************************************
 **
 **  MailboxService.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/


#ifndef MAILBOXSERIVCE_H
#define MAILBOXSERIVCE_H

#include <Packet.h>
#include <Service.h>
#include <Mailbox.h>

// The Makernet MailboxService is a flexible attribute-based notification and
// synchronization service that can serve a large variety of purposes. The
// core idea is to have a set of numbered mailboxes which remain in sync
// between two different nodes. These mailboxes are registered at
// configuration time, and there is the potential for mutliple different
// mailbox types which can implement custom semantics for network-efficient
// updates. To trigger a mailbox change, simply update the value using the OO
// syntax and the framework will trigger the update and sync packets.
//
// The MailboxService implements the basic routing for a mailbox, but it
// remains the responsibility of the caller to set up the contents of the
// mailboxes at configuration time.

#define MAX_MAILBOXS_PER_SERVICE 16

struct MailboxUpdateMessage {
	uint8_t mailbox;
	uint8_t payload[];
};

#define MAX_MAILBOX_MESSAGE_SIZE (MAX_PACKET_PAYLOAD_SIZE - sizeof(MailboxUpdateMessage))

class MailboxService : public Service {

public:
	virtual void initialize();
	virtual void configure();
	virtual int handlePacket( Packet *p );
	virtual int pollPacket( Packet *p );
	// virtual void loop();
	virtual void busReset();

	Mailbox *mailboxes[MAX_MAILBOXS_PER_SERVICE];

	// Setters and getters
	void set( int index, Mailbox& m );
	Mailbox *get( int index );

private:
	int nextPendingMailboxIndex();
	void pointPacketToEndpoint( Packet *p );

// 	MailboxDictionary *mailboxDict;
};



#endif
