#include <Makernet.h>
#include <MakernetEncoderReader.h>
#include <Bounce2.h>

EncoderMailboxService ems;

// Stuff here used for encoder

#define MAKERNET_PINOUT_ENCODER_A 10
#define MAKERNET_PINOUT_ENCODER_B 15
#define MAKERNET_PINOUT_ENCODER_SW 3

#define MAKERNET_PINOUT_ENCODER_RED 4
#define MAKERNET_PINOUT_ENCODER_GREEN 5
#define MAKERNET_PINOUT_ENCODER_BLUE 17

MakernetEncoderReader encoder( MAKERNET_PINOUT_ENCODER_A, MAKERNET_PINOUT_ENCODER_B );
Bounce rotaryButton;

void isr0(void)
{
  noInterrupts();

  // A bit of a hack that takes advantage of the fact that on the SAMD11 all pins are
  // on the same port. This ensures that both are read together.
  long portVal = PORT->Group[g_APinDescription[MAKERNET_PINOUT_ENCODER_A].ulPort].IN.reg;
  boolean pin1 = portVal & (1ul << g_APinDescription[MAKERNET_PINOUT_ENCODER_A].ulPin);
  boolean pin2 = portVal & (1ul << g_APinDescription[MAKERNET_PINOUT_ENCODER_B].ulPin);

  encoder.update(pin1, pin2);

  interrupts();
}

void setup() {
  while ( !Serial );

  encoder.begin();
  pinMode( MAKERNET_PINOUT_ENCODER_SW, INPUT_PULLDOWN );
  rotaryButton.attach(MAKERNET_PINOUT_ENCODER_SW);
    
  attachInterrupt(encoder._pin1, isr0, CHANGE);
  attachInterrupt(encoder._pin2, isr0, CHANGE);

  Serial.begin(115200);

  Makernet.initialize( DeviceType::Encoder, ems );
}

void loop() {
  // Occasionally, the signal is too noisy and by the time the read occurs,
  // the signal has bounced more and it gets the old value.
  // Empirical testing has shown that a check every 500 microseconds seems to
  // fix this issue and does not cause any harm.
  isr0();
  delayMicroseconds(500);

  if ( encoder.hasChanged() )
  {
    int16_t newPos = encoder.getPosition();
    DEBUGSERIAL.print( "@@@@ New encoder position: " );
    DEBUGSERIAL.println( newPos );
    ems.position.setLong( newPos );
  }

  rotaryButton.update();

  if ( rotaryButton.rose() )
  {
    DEBUGSERIAL.println( "@@@@ Encoder pressed" );
    ems.buttonDown.trigger();
  }

  if ( rotaryButton.fell() )
  {
    DEBUGSERIAL.println( "@@@@ Encoder released" );
    ems.buttonUp.trigger();
  }

  Makernet.loop();
}
