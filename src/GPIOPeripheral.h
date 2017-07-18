/********************************************************
 **
 **  GPIOPeripheral.h
 **
 **  Part of the Makernet framework by Jeremy Gilbert
 **
 **  License: GPL 3
 **  See footer for copyright and license details.
 **
 ********************************************************/


#ifndef GPIOPERIPHERAL_H
#define GPIOPERIPHERAL_H

#include <BasePeripheral.h>
#include <Mailbox.h>
#include <MailboxService.h>

struct GPIO_PWMSettings {
	uint8_t pwm[16];
};

// This is a subclass of the mailbox dictionary. Every peripheral should have
// one so that the mailbox configuration is constant between the peripheral
// code and the object host code. To use it, create member variables for each
// mailbox that could handle values. Then wire them up in the configure()
// function of the implementation.

class GPIOMailboxService : public MailboxService {
public:
	IntegerMailbox pinModes   = IntegerMailbox( 0, "pinModes" );
	IntegerMailbox pinValues  = IntegerMailbox( 0, "pinValues" );
	IntegerMailbox pwmEnables = IntegerMailbox( 0, "pwmEnables" );
	StructMailbox<GPIO_PWMSettings> pwmStruct = StructMailbox<GPIO_PWMSettings>( 0, "pwmSettings" );
	virtual void configure() {
		set( 0, pinModes );
		set( 1, pinValues );
		set( 2, pwmEnables );
	};
};


class GPIOPeripheral : public BasePeripheral {
public:
	GPIOPeripheral();
	virtual void configure();

	GPIOMailboxService gpioMailboxSvc;

// From IMailboxObserver
// for later
//	virtual void onMailboxChange( Mailbox *m, bool wasTriggered );

	// Next block defines the actual GPIO interface
	void setPinModes( uint32_t modes );
	void setPinValues( uint32_t values );
	void setPWMEnables( uint32_t pwmEnables );
	void setPWM( uint8_t pin, uint8_t duty );

	// Shortcuts to match expected Arduino API
	void pinMode( uint8_t pin, int setting );
	void digitalWrite( uint8_t pin, int setting );
	void pwmConfig( uint8_t pin, boolean setting );
};


#endif
