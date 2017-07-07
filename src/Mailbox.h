#ifndef MAILBOX_H
#define MAILBOX_H

#include <Types.h>

#define MB_ROLE_CONTROLLER 0x1
#define MB_ROLE_DEVICE 0x2

#define MAILBOX_OP_FOURBYTE_DEFINITIVE 0x10

// A mailbox represents an extremely flexible place where values can be set
// and get by multiple nodes on a network. Any time a mailbox is set, it
// triggers a push to a remote mailbox. Remote mailboxes can easily be used to
// convey all sorts of information and configuration to and from a device.
// What's desirable about this arrangement is that a device can be fully reset
// and then recover its "configuration" when reconnection occurs. Furthermore,
// a mailbox can act as a trigger point, allowing one-shot events like button
// up or down events, to be communicated reliably (and one-time only) to a
// controller.
//
// The Mailbox class represents a single one of these item places, and Mailbox
// is the abstract interface thus lacks any implementation.
//
// The mailbox framework is a general purpose tool that is completely agnostic
// to packet or frame formats. It could easily exist outside of Makernet. The
// core interface to this "network agnostic" is that its caller will poll it
// for changes, and provide it a place to create a new message of its own
// format. The caller transports that message, treating the contents like a
// black box, to another Mailbox object which then applies the changes and
// brings the items in sync.
//
// The Mailbox and MailboxDictionary framework has no concept of a connection


class Mailbox {
public:
	uint8_t *contents;
	const char *description;
	uint8_t size;
	uint8_t synchronized;

	uint8_t flags;

	virtual void reset();
	virtual void trigger() = 0;
	virtual int prepareUpdatePacket( uint8_t *buffer, int size ) = 0;
	virtual int handleUpdatePacket( uint8_t *buffer, int size ) = 0;
	virtual int handleAckPacket( uint8_t *buffer, int size ) = 0;
	virtual int hasPendingChanges() = 0;
};



// Small mailbox is an implementation of a mailbox of a small number of bytes
// like an RGB value or an integer.  It always uses opcode 0 to update
// (transmission of whole value) and it doesn't worry about version numbers.

#define MAILBOX_SMALLFORMAT_SIZE 4

class SmallMailbox : public Mailbox {

public:
	SmallMailbox(uint8_t configFlags, const char *d);
	uint8_t __contents[4];

	virtual void reset();
	virtual void trigger();
	virtual int prepareUpdatePacket( uint8_t *buffer, int size );
	virtual int handleUpdatePacket( uint8_t *buffer, int size );
	virtual int handleAckPacket( uint8_t *buffer, int size );
	virtual int hasPendingChanges();
	void setLong( uint32_t v );
	uint32_t getLong();
};

// MailboxDictionary implements an array of Mailbox objects and acts as a
// supervisor to each "entry".  The mailbox dictionary uses the same "network
// agnostic" calling interface as a mailbox object. The caller is expected to
// periodically call it for updates, then transmit the packet messages
// unchanged to the remote server where those changes will be applied.

#define MAX_DICT_ENTRIES 16

class MailboxDictionary {
public:
	MailboxDictionary();
	virtual void configure();
	Mailbox *mailboxes[MAX_DICT_ENTRIES];
	void set( int index, Mailbox& m );
	Mailbox *get( int index );
	int hasPendingChanges();
	Mailbox* nextMailboxWithChanges();
	int nextPendingMailboxIndex();
	int prepareUpdatePacket( uint8_t *buffer, uint8_t size );
	int handleUpdatePacket( uint8_t *buffer, uint8_t size );
	int handleAckPacket( uint8_t *buffer, uint8_t size );

};



#endif


