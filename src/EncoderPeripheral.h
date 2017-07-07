#ifndef ENCODERPERIPHERAL_H
#define ENCODERPERIPHERAL_H


#include <BasePeripheral.h>
#include <Mailbox.h>

// This is a subclass of the mailbox dictionary. Every peripheral should have
// one so that the mailbox configuration is constant between the peripheral
// code and the object host code. To use it, create member variables for each
// mailbox that could handle values. Then wire them up in the configure()
// function of the implementation.

class EncoderMailboxDictionary : public MailboxDictionary {
public:
	virtual void configure();
	SmallMailbox position   = SmallMailbox(DEVICE, "Encoder position");
	SmallMailbox buttonDown = SmallMailbox(DEVICE, "Button down");
	SmallMailbox buttonUp   = SmallMailbox(DEVICE, "Button up");
};


class EncoderPeripheral : public BasePeripheral {
public:
	EncoderPeripheral();
	virtual void configure();

	EncoderMailboxDictionary encoderDictionary;

};


#endif
