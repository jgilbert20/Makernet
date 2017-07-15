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


#ifndef KEYPADPERIPHERAL_H
#define KEYPADPERIPHERAL_H

#include <GenericFunction.h>

#include <BasePeripheral.h>
#include <Mailbox.h>
#include <MailboxService.h>

// This is a subclass of the mailbox dictionary. Every peripheral should have
// one so that the mailbox configuration is constant between the peripheral
// code and the object host code. To use it, create member variables for each
// mailbox that could handle values. Then wire them up in the configure()
// function of the implementation.

class KeypadMailboxService : public MailboxService {
public:
	SmallMailbox event = SmallMailbox(0, "Keypad event");
	virtual void configure() {
		set( 0, event );
	};
};


class KeypadPeripheral : public BasePeripheral, public IMailboxObserver {
public:
	KeypadPeripheral();
	virtual void configure();

	KeypadMailboxService keypadMailboxSvc;

// From IMailboxObserver
	virtual void onMailboxChange( Mailbox *m, bool wasTriggered );

// Callbacks
	typedef void (*KeypadEventHandler)( KeyEvent *e );
	void onKeyEvent( KeypadEventHandler h ) { keyEventHandler = h; };
	KeypadEventHandler keyEventHandler = NULL; 

};


#endif
