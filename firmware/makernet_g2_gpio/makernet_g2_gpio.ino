#include <Makernet.h>
typedef void (*ABCD)( KeyEvent *e );

GPIOMailboxService gms;

#define MAKERNET_GPIOMODULE_NUMPINS  12

uint8_t physicalPinForLogical[] = { 27, 25, 24, 17, 15, 14, 11, 10, 5, 4, 3, 2};
unsigned long fpsLastReset = 0;
unsigned long totalFrames = 0;
uint8_t pwmCounter = 0;

uint8_t pwmDuty[MAKERNET_GPIOMODULE_NUMPINS];
uint32_t pinModes = 0; // 1 = output, 0 = input
uint32_t pinValues = 0; // 0 = LOW, 1 = High
uint32_t pwmEnables = 0; // 0 = disable, 1 = enable

Interval reportInterval(2000);

// FPS 114K with no PWMs (-O3)

void pinModeChange( SmallMailbox *m, bool wasTriggered )
{
  pinModes = ((IntegerMailbox *)m)->getLong();
  DEBUGSERIAL.print( "Setting GPIO mode: 0x" );
  DEBUGSERIAL.println( pinModes , HEX );

  for ( int i = 0 ; i < MAKERNET_GPIOMODULE_NUMPINS ; i++ ) {
    if ( pinModes & (1 << i) )
      pinMode( physicalPinForLogical[i], OUTPUT );
    else
      pinMode( physicalPinForLogical[i], INPUT );
  }
}

void pinValueChange( SmallMailbox *m, bool wasTriggered )
{
  pinValues = ((IntegerMailbox *)m)->getLong();
  DEBUGSERIAL.print( "Setting GPIO values: 0x" );
  DEBUGSERIAL.println( pinValues , HEX );

  for ( int i = 0 ; i < MAKERNET_GPIOMODULE_NUMPINS ; i++ ) {
    if ( (pinModes & pinValues ) & (1 << i) )
      digitalWrite( physicalPinForLogical[i], HIGH );
    else
      digitalWrite( physicalPinForLogical[i], LOW );
  }
}

void pwmEnableChange( SmallMailbox *m, bool wasTriggered )
{
  pwmEnables = ((IntegerMailbox *)m)->getLong();
  DEBUGSERIAL.print( "Setting GPIO mode: 0x" );
  DEBUGSERIAL.println( pwmEnables , HEX );

  for ( int i = 0 ; i < MAKERNET_GPIOMODULE_NUMPINS ; i++ ) {
    if ( (pinModes & pinValues ) & (1 << i) )
      digitalWrite( physicalPinForLogical[i], HIGH );
    else
      digitalWrite( physicalPinForLogical[i], LOW );
  }
}

void pwmStructChange( SmallMailbox *m, bool wasTriggered )
{
  GPIO_PWMSettings settings = ((StructMailbox<GPIO_PWMSettings> *)m)->getValue();
  
  DEBUGSERIAL.print( "Setting PWM mode: 0x" );
  DEBUGSERIAL.println( pwmEnables , HEX );

  for ( int i = 0 ; i < MAKERNET_GPIOMODULE_NUMPINS ; i++ ) {
    pwmDuty[i] = settings.pwm[i];
  }
}



void setup() {
  while ( !Serial );

  gms.pinModes.onChange( pinModeChange );
  gms.pinValues.onChange( pinValueChange );
  gms.pwmEnables.onChange( pwmEnableChange );
  gms.pwmStruct.onChange( pwmStructChange );

  // Tristate everything
  for ( int i = 0 ; i < MAKERNET_GPIOMODULE_NUMPINS ; i++ )
    pinMode( physicalPinForLogical[i], INPUT );

  Serial.begin(115200);

  Makernet.initialize( DeviceType::GPIO, gms );
}


#pragma GCC optimize ("O3")


void loop() {
  totalFrames++;
  pwmCounter++;

if(0)
  for ( int i = 0 ; i < MAKERNET_GPIOMODULE_NUMPINS ; i++ )
    if ( (pinModes & pwmEnables) & (1 << i) ) {
      if ( pwmDuty[i] >= pwmCounter ) {
        digitalWrite( physicalPinForLogical[i], HIGH );
      } else
        digitalWrite( physicalPinForLogical[i], LOW );
    }

  if ( reportInterval.hasPassed() ) {
    DEBUGSERIAL.print( "GPIO Board :: " );
    DEBUGSERIAL.print( "  fps:" );
    DEBUGSERIAL.println( totalFrames * 1000 / (millis() - fpsLastReset), DEC );
    fpsLastReset = millis();
    totalFrames = 0;
  }
  Makernet.loop();
}


