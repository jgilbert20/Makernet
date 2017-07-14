/********************************************************
 **
 **  EncoderPeripheral.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#include <EncoderPeripheral.h>
#include <Debug.h>

// void EncoderMailboxDictionary::configure()
// {

// 	DPR( dANY, "Configuring encoder dictionary...");
// 	set(0, position);
// 	set(1, buttonDown);
// 	set(2, buttonUp);
// }


EncoderPeripheral::EncoderPeripheral() :
	BasePeripheral(DeviceType::Encoder)
{
}

void EncoderPeripheral::configure()
{
	registerService( 1, &encoderMailboxSvc );

	encoderMailboxSvc.attachObserver( this );
	// encoderMailboxSvc.position.onChange = [](SmallMailbox *m, bool hasChanged ) -> void {
	// 	DPF( dANY, "Got a change!! %d\n", m->getLongSigned() );
	// };
}

void EncoderPeripheral::onMailboxChange( Mailbox *m, bool wasTriggered )
{

}