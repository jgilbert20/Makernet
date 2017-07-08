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

int SmallMailbox::prepareUpdatePacket( uint8_t *buffer, int size )
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

int SmallMailbox::handleAckPacket( uint8_t *buffer, int size )
{
	if ( size < 1)
		return -1;
	if ( buffer == NULL )
		return -2;

	if ( buffer[0] == MAILBOX_OP_FOURBYTE_DEFINITIVE ) {
		synchronized = 1;
	}


	DLN( dMAILBOX, "Ack received for mailbox!");

	return 0;
}

int SmallMailbox::handleUpdatePacket( uint8_t *buffer, int size )
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

MailboxDictionary::MailboxDictionary()
{
	for (int i = 0 ; i < MAX_DICT_ENTRIES ; i++ )
		mailboxes[i] = NULL;
}


void MailboxDictionary::set( int index, Mailbox& m )
{
	if ( index < MAX_DICT_ENTRIES )
		mailboxes[index] = &m;
}

Mailbox *MailboxDictionary::get( int index )
{
	if ( index < MAX_DICT_ENTRIES )
		return mailboxes[index];
	return NULL;
}

Mailbox* MailboxDictionary::nextMailboxWithChanges()
{
	for ( int i = 0 ; i < MAX_DICT_ENTRIES ; i++ )
		if ( mailboxes[i] != NULL )
			if ( mailboxes[i]->hasPendingChanges() )
				return mailboxes[i];
	return NULL;
}

int MailboxDictionary::hasPendingChanges()
{
	return ( nextMailboxWithChanges() != NULL );
}

int MailboxDictionary::nextPendingMailboxIndex()
{
	for ( int i = 0 ; i < MAX_DICT_ENTRIES ; i++ )
		if ( mailboxes[i] != NULL )
			if ( mailboxes[i]->hasPendingChanges() )
				return i;
	return -11;
}

int MailboxDictionary::prepareUpdatePacket( uint8_t *buffer, uint8_t size )
{
	int index = nextPendingMailboxIndex();
	if ( index < 0 )
		return 0;

	if ( size < 11 )
		return -11;
	if ( buffer == NULL )
		return -12;

	Mailbox *m = mailboxes[index];

	buffer[0] = index;
	return 1 + m->prepareUpdatePacket( buffer + 1, size - 1 );

}

int MailboxDictionary::handleUpdatePacket( uint8_t *buffer, uint8_t size )
{
	if ( size < 1 )
		return -8;
	if ( buffer == NULL )
		return -9;

	int index = buffer[0];

	if ( index >= MAX_DICT_ENTRIES )
		return -27;

	Mailbox *m = mailboxes[index];
	if ( m == NULL )
		return -10;

	return 1 + m->handleUpdatePacket( buffer + 1, size - 1 );
}

int MailboxDictionary::handleAckPacket( uint8_t *buffer, uint8_t size )
{
	if ( size < 1 )
		return -11;
	if ( buffer == NULL )
		return -12;

	int index = buffer[0];

	Mailbox *m = mailboxes[index];
	if ( m == NULL )
		return -13;

	return 1 + m->handleAckPacket( buffer + 1, size - 1 );
}

void MailboxDictionary::configure()
{

	DLN( dMAILBOX | dWARNING, "*** WARNING - MAILBOX BASE CONFIGURE CALLED, NO MAILBOXES SET UP");
	return;
}



