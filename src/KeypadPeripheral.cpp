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

KeypadPeripheral::KeypadPeripheral() :
	BasePeripheral(DeviceType::Keypad)
{
}

void KeypadPeripheral::configure()
{
	registerService( 1, &keypadMailboxSvc );
	// keypadMailboxSvc.event.onChange = [=](SmallMailbox (*m), bool hasChanged ) {
	// 	DPF( dANY, "Got a change!! %d\n", m->getLongSigned() );
	// };
}