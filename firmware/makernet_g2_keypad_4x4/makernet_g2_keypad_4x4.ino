#include <Makernet.h>

KeypadMailboxService kms;

#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = { 
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2, 3, 4, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {15, 14, 11, 10}; //connect to the column pinouts of the keypad

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long loopCount;
unsigned long startTime;
String msg;


void setup() {

  Serial.begin(115200);

  Makernet.initialize( DeviceType::Keypad, kms );

  loopCount = 0;
  startTime = millis();
  msg = "";
}


void loop() {
  Makernet.loop();
  loopCount++;
  if ( (millis() - startTime) > 5000 ) {
    Serial.print("Average loops per second = ");
    Serial.println(loopCount / 5);
    startTime = millis();
    loopCount = 0;
  }

  // Fills kpd.key[ ] array with up-to 10 active keys.
  // Returns true if there are ANY active keys.
  if (kpd.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        KeyEvent kv;
        kv.action = KeyEvent::Action::HELD;
        kv.key = kpd.key[i].kchar;
        switch (kpd.key[i].kstate) {
          case PRESSED: kv.action = KeyEvent::Action::PRESSED; break;
          case HOLD: kv.action = KeyEvent::Action::HELD; break;
          case RELEASED: kv.action = KeyEvent::Action::RELEASED; break;
          case IDLE: kv.action = KeyEvent::Action::IDLE; break;
        }
        kms.event.enqueueEvent( kv );

        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED: 
            msg = " PRESSED.";
            break;
          case HOLD:
            msg = " HOLD.";
            break;
          case RELEASED:
            msg = " RELEASED.";
            break;
          case IDLE:
            msg = " IDLE.";
        }



        Serial.print("Key ");
        Serial.print(kpd.key[i].kchar);
        Serial.println(msg);
      }
    }
  }
}  // End loop
