


#include <EncoderPeripheral.h>
#include <Debug.h>

void EncoderMailboxDictionary::configure()
{
	
	DPR( dANY, "Configuring encoder dictionary...");
	set(0, position);
	set(1, buttonDown);
	set(2, buttonUp);
}


EncoderPeripheral::EncoderPeripheral() :
	BasePeripheral(DeviceType::Encoder)
{
}

void EncoderPeripheral::configure()
{
	 encoderDictionary.configure();
}