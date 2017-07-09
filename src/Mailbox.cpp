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
	contents = __contents;
	flags = configFlags;
	size = 4;
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

int SmallMailbox::generateMessage( uint8_t *buffer, int size )
{
	if ( size < MAILBOX_SMALLFORMAT_SIZE + 2 )
		return -1;
	if ( buffer == NULL )
		return -2;

	buffer[0] = MAILBOX_OP_FOURBYTE_DEFINITIVE;
	buffer[1] = contents[0];
	buffer[2] = contents[1];
	buffer[3] = contents[2];
	buffer[4] = contents[3];

	return 5;
}

// int SmallMailbox::handleAckPacket( uint8_t *buffer, int size )
// {
// 	if ( size < 1)
// 		return -1;
// 	if ( buffer == NULL )
// 		return -2;

// 	if ( buffer[0] == MAILBOX_OP_FOURBYTE_DEFINITIVE ) {
// 		synchronized = 1;
// 	}


// 	DLN( dMAILBOX, "Ack received for mailbox!");

// 	return 0;
// }

int SmallMailbox::handleMessage( uint8_t *buffer, int size )
{
	if ( size < 5)
		return -1;
	if ( buffer == NULL )
		return -2;

	if ( buffer[0] != MAILBOX_OP_FOURBYTE_DEFINITIVE )
		return -3;

	contents[0] = (uint8_t)(buffer[1]);
	contents[1] = (uint8_t)(buffer[2]);
	contents[2] = (uint8_t)(buffer[3]);
	contents[3] = (uint8_t)(buffer[4]);
	synchronized = 1;

	// Populate response

	buffer[0] = MAILBOX_OP_FOURBYTE_DEFINITIVE;

	DLN( dMAILBOX, "New value received for mailbox!");

	DPR( dMAILBOX, "&&&& MailboxChange: [");
	DPR( dMAILBOX, description );
	DPR( dMAILBOX, "] updated over network to: [");
	hexPrint( dMAILBOX, contents, 4 );
	DLN( dMAILBOX, "]");

	return 1;
}

void SmallMailbox::setLong( uint32_t v )
{
	contents[0] = (uint8_t)(v >> 24);
	contents[1] = (uint8_t)(v >> 16);
	contents[2] = (uint8_t)(v >> 8);
	contents[3] = (uint8_t)(v);
	synchronized = 0;


	DPR( dMAILBOX, "&&&& MailboxChange: [");
	DPR( dMAILBOX, description );
	DPR( dMAILBOX, "] set to: [");
	hexPrint( dMAILBOX, contents, 4 );
	DLN( dMAILBOX, "]");

}

uint32_t SmallMailbox::getLong()
{
	return ( (((uint32_t)contents[0]) << 24 ) |
	         (((uint32_t)contents[1]) << 16 ) |
	         (((uint32_t)contents[2]) << 8  ) |
	         (((uint32_t)contents[3]) ) );
}




