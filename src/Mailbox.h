/********************************************************
 **
 **  Mailbox.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#ifndef MAILBOX_H
#define MAILBOX_H

#include <Types.h>
#include <Interval.h>

#define MB_ROLE_CONTROLLER 0x1
#define MB_ROLE_DEVICE 0x2

#define MAILBOX_OP_FOURBYTE_DEFINITIVE 0x10

class Mailbox;

class IMailboxObserver {
public:
	virtual void onMailboxChange( Mailbox *m, bool wasTriggered ) = 0;
};




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
// The Mailbox framework has no concept of a connection


class Mailbox {
public:
	uint8_t *contents;
	const char *description;
	uint8_t mailboxSize;
	uint8_t synchronized;
	// 0 if the last update was over net, 1 if last update from API
	uint8_t callerChanged;

	uint8_t flags;

	IMailboxObserver *observer; 

	virtual void busReset();
	virtual void trigger() = 0;
	virtual int generateMessage( uint8_t *buffer, int size ) = 0;
	virtual int handleMessage( uint8_t *buffer, int size ) = 0;
	virtual int hasPendingChanges() = 0;
};



// Candidate for refactor to own subclass

struct KeyEvent {
	enum class Action : uint8_t { PRESSED, HELD, RELEASED, IDLE } action;
	char key;
	char unused1;
	char unused2;
	// operator long() { return (long) * this; } // vodoo, but does it work?
} __attribute__((packed)); // magic to prevent compiler from aligning contents of struct


// Small mailbox is an abstract implementation of a mailbox of a small number
// of bytes like an RGB value or an integer. The mailbox size must allow the
// entire mailbox to be updated in a single packet. It always uses opcode 0 to
// update (transmission of whole value) and it doesn't worry about version
// numbers.

class SmallMailbox : public Mailbox {

public:
	SmallMailbox(uint8_t configFlags, const char *d);

	virtual void busReset();
	virtual void trigger();
	virtual int generateMessage( uint8_t *buffer, int size );
	virtual int handleMessage( uint8_t *buffer, int size );
	virtual int hasPendingChanges();

	// ******* Smallmailbox specific stuff follows here:

//	virtual void configureStorage() = 0;

	// typedef void (*OnChangeHandler)(SmallMailbox *m, bool hasChanged );
	// OnChangeHandler onChange = 0;

	bool changeTrigger; // One shot for change notifications

	Interval retryTimer = Interval(1000);
};




class IntegerMailbox : public SmallMailbox {
public:
	IntegerMailbox(uint8_t configFlags, const char *d);
//	virtual void configureStorage();

	void setLong( uint32_t v );
	uint32_t getLong();
	int32_t getLongSigned();

	// Candidate for refactor to own subclass
	void enqueueEvent( KeyEvent kv );
	KeyEvent *getValueAsKeyEventPtr();

private:
	uint32_t __contents;
};





#endif


