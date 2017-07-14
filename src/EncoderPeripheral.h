/********************************************************
 **
 **  EncoderPeripheral.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/


#ifndef ENCODERPERIPHERAL_H
#define ENCODERPERIPHERAL_H

#include <BasePeripheral.h>
#include <Mailbox.h>
#include <MailboxService.h>

// This is a subclass of the mailbox dictionary. Every peripheral should have
// one so that the mailbox configuration is constant between the peripheral
// code and the object host code. To use it, create member variables for each
// mailbox that could handle values. Then wire them up in the configure()
// function of the implementation.

class EncoderMailboxService : public MailboxService {
public:
	SmallMailbox position   = SmallMailbox(0, "Encoder position");
	SmallMailbox buttonDown = SmallMailbox(0, "Button down");
	SmallMailbox buttonUp   = SmallMailbox(0, "Button up");
	virtual void configure() {
		set( 0, position );
		set( 1, buttonDown );
		set( 2, buttonUp );
	};
};


class EncoderPeripheral : public BasePeripheral, public IMailboxObserver {
public:
	EncoderPeripheral();
	virtual void configure();

	EncoderMailboxService encoderMailboxSvc;

	// From IMailboxObserver
	virtual void onMailboxChange( Mailbox *m, bool wasTriggered );

};


#endif
