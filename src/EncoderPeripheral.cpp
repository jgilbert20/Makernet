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
	 // encoderDictionary.configure();
}