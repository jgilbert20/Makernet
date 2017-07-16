/********************************************************
 **
 **  GPIOPeripheral.cpp
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/

#include <GPIOPeripheral.h>
#include <Debug.h>
#include <ArduinoAPI.h>

GPIOPeripheral::GPIOPeripheral() :
	BasePeripheral(DeviceType::GPIO)
{
}

void GPIOPeripheral::configure()
{
	registerService( 1, &gpioMailboxSvc );
}

void GPIOPeripheral::setPinModes( uint32_t modes )
{
	gpioMailboxSvc.pinModes.setLong( modes );
}

void GPIOPeripheral::setPinValues( uint32_t values )
{
	gpioMailboxSvc.pinValues.setLong( values );
}

void GPIOPeripheral::setPWMEnables( uint32_t pwmEnables )
{
	gpioMailboxSvc.pwmEnables.setLong( pwmEnables );
}

void GPIOPeripheral::pinMode( uint8_t pin, int setting )
{

	uint32_t pinModes = gpioMailboxSvc.pinModes.getLong();

	if ( setting == OUTPUT )
		pinModes |= 1 << pin;

	if ( setting == INPUT )
		pinModes &= ~(1 << pin);

	setPinModes( pinModes );
}

void GPIOPeripheral::digitalWrite( uint8_t pin, int setting )
{
	uint32_t pinValues = gpioMailboxSvc.pinValues.getLong();

	if ( setting == HIGH || setting )
		pinValues |= 1 << pin;
	else
		pinValues &= ~(1 << pin);

	setPinValues( pinValues );

}

void GPIOPeripheral::pwmConfig( uint8_t pin, boolean setting )
{
	uint32_t pwmEnables = gpioMailboxSvc.pwmEnables.getLong();

	if ( setting )
		pwmEnables |= 1 << pin;
	else
		pwmEnables &= ~(1 << pin);

	setPWMEnables( pwmEnables );

}