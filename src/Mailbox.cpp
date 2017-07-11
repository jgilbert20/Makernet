/********************************************************
 **
 **  Mailbox.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#include <Mailbox.h>
#include <Types.h>
#include <Debug.h>
#include <Util.h>
#include <Globals.h>

#include <strings.h>

void Mailbox::reset()
{
	// firstInvalidByte = 0;
	// lastInvalidByte = size;
	// version++;
	synchronized = 1;
	if ( contents != NULL and size > 0 )
		memset( contents, 0, size ) ;
}

SmallMailbox::SmallMailbox(uint8_t configFlags, const char *d)
{
	contents = (uint8_t *)(void *)&__contents;
	flags = configFlags;
	size = sizeof( __contents );
	description = d;
	reset();
}

void SmallMailbox::reset()
{
	Mailbox::reset();
}

void SmallMailbox::trigger()
{
	synchronized = 0;
}

int SmallMailbox::hasPendingChanges()
{
	return !synchronized;
}

struct SmallMailboxMessage {
	enum class Command : uint8_t { SEND_VALUE, ACK_VALUE } command;
	uint32_t value;
} __attribute__((packed)); // magic to prevent compiler from aligning contents of struct

// Called by the framework when we've indicated a pending message is
// available. Our job is to transmit any unacknowledged values

int SmallMailbox::generateMessage( uint8_t *buffer, int size )
{
	if (API_CHECK and ( size < sizeof( SmallMailboxMessage ) ))
		return -1;
	if (API_CHECK and ( buffer == NULL ))
		return -2;

	auto *msg = reinterpret_cast<SmallMailboxMessage *>(buffer);

	msg->command = SmallMailboxMessage::Command::SEND_VALUE;
	msg->value = __contents;

	return sizeof( SmallMailboxMessage );
}

int SmallMailbox::handleMessage( uint8_t *buffer, int size )
{
	if (API_CHECK and ( size < sizeof( SmallMailboxMessage ) ))
		return -1;
	if (API_CHECK and ( buffer == NULL ))
		return -2;

	auto *msg = reinterpret_cast<SmallMailboxMessage *>(buffer);

	if ( msg->command == SmallMailboxMessage::Command::SEND_VALUE ) {
		__contents = msg->value;
		synchronized = 1;

		DPR( dMAILBOX, "&&&& Mailbox value recv: [");
		DPR( dMAILBOX, description );
		DPR( dMAILBOX, "] updated over network to: [");
		hexPrint( dMAILBOX, reinterpret_cast<uint8_t *>(contents), 4 );
		DPR( dMAILBOX, "] as ui32: [");
		DPR( dMAILBOX, __contents );
		DLN( dMAILBOX, "]");

		msg->command = SmallMailboxMessage::Command::ACK_VALUE;

		return (sizeof( SmallMailboxMessage ));
	}

	if ( msg->command == SmallMailboxMessage::Command::ACK_VALUE ) {

		if ( msg->value == __contents )
			synchronized = 1;
		else
			DPR( dMAILBOX | dWARNING, "&&&& Mailbox ACK incorrect, not clearing sync flag" );

		DPR( dMAILBOX, "&&&& Mailbox value acknowledgement: [");
		DPR( dMAILBOX, description );
		DPR( dMAILBOX, "] updated over network to: [");
		hexPrint( dMAILBOX, reinterpret_cast<uint8_t *>(contents), 4 );
		DPR( dMAILBOX, "] as ui32: [");
		DPR( dMAILBOX, __contents );
		DLN( dMAILBOX, "]");

		return 0;
	}

	return 1;
}

void SmallMailbox::setLong( uint32_t v )
{
	__contents = v;

	synchronized = 0;

	DPR( dMAILBOX, "&&&& MailboxChange: [");
	DPR( dMAILBOX, description );
	DPR( dMAILBOX, "] set to: [");
	hexPrint( dMAILBOX, contents, 4 );
	DPR( dMAILBOX, "] as long: [");
	DPR( dMAILBOX, __contents );
	DLN( dMAILBOX, "]");

}

uint32_t SmallMailbox::getLong()
{
	return ( __contents );
}
