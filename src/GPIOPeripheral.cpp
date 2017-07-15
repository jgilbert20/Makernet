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

GPIOPeripheral::GPIOPeripheral() :
	BasePeripheral(DeviceType::GPIO)
{
}

void GPIOPeripheral::configure()
{
	registerService( 1, &gpioMailboxSvc );
}

