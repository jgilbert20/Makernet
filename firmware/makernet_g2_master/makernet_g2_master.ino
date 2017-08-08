#include <Makernet.h>

I2CDatalink um;

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
  
 
}

void setup() {

  while ( !Serial );
  Serial.begin(115200);
  Serial.println( "Starting up" );

  Makernet.initialize( DeviceType::Controller );

  keypad.onKeyEvent( handleKeyPress );
  pinMode(6, OUTPUT ); 
}

Interval i = Interval(1000);
bool v = false;

void loop() {
  
long m = micros();
digitalWrite(6, HIGH );
  Makernet.loop();
  digitalWrite(6, LOW );
  int t =  micros() - m;
  if( t > 100 )
  Serial.println(t );
}


