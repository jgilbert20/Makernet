#include <Makernet.h>

I2CDatalink um;
EncoderPeripheral enc;
GPIOPeripheral gpio;
KeypadPeripheral keypad;


void handleKeyPress( KeyEvent *e )
{
  Serial.print( "Client code got KE: [" );
  Serial.print( e->key );
  Serial.print( "] action=" );
  switch ( e->action ) {
    case KeyEvent::Action::PRESSED: Serial.print( "PRESSED" ); break;
    case KeyEvent::Action::HELD: Serial.print( "HELD" ); break;
    case KeyEvent::Action::IDLE: Serial.print( "IDLE" ); break;
    case KeyEvent::Action::RELEASED: Serial.print( "RELEASED" ); break;
  }
  Serial.println();

  bool isDown = e->action == KeyEvent::Action::PRESSED or e->action == KeyEvent::Action::HELD;

  if( e->key >= '1' && e->key <= '9' )
    gpio.digitalWrite( 11 -(e->key-'1'), isDown );
}

void setup() {

  while ( !Serial );
  Serial.begin(115200);
  Serial.println( "Starting up" );

  Makernet.initialize( DeviceType::Controller );

  keypad.onKeyEvent( handleKeyPress );

  for ( int i = 0 ; i < 13 ; i++ )
  {
    gpio.digitalWrite( i, LOW );
    gpio.pinMode( i, OUTPUT );
  }


}

Interval i = Interval(1000);
bool v = false;

void loop() {
  if ( i.hasPassed() ) {
    v = !v;
    gpio.digitalWrite( 8, v );
  }
  Makernet.loop();
}


