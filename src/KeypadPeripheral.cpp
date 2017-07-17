/********************************************************
 **
 **  KeypadPeripheral.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#include <KeypadPeripheral.h>
#include <Debug.h>

#include <GenericFunction.h>

KeypadPeripheral::KeypadPeripheral() :
	BasePeripheral(DeviceType::Keypad)
{
}


void KeypadPeripheral::configure()
{
	registerService( 1, &keypadMailboxSvc );
	keypadMailboxSvc.attachObserver( this );
	// keypadMailboxSvc.event.onChange = [=](SmallMailbox (*m), bool hasChanged ) {
	// 	DPF( dANY, "Got a change!! %d\n", m->getLongSigned() );
	// };
}

// Registered callback. wasTriggered tells us that a specific "new" item was
// put in the mailbox for us as opposed to a new value simply being
// synchronized after a network reset.

void KeypadPeripheral::onMailboxChange( Mailbox *m, bool wasTriggered )
{

	DPF( dANY, "Got a mailbox change\n");

	if( m == &keypadMailboxSvc.event ) {
		IntegerMailbox *km = (IntegerMailbox *)m;
		if( wasTriggered )
			if( keyEventHandler )
				keyEventHandler( km->getValueAsKeyEventPtr() );
	}

}