#include <Makernet.h>

KeypadMailboxService kms;

#include <Keypad.h>

const byte ROWS = 8;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'F', 'R', '_', '9'},
  {'2', '3', '4', '1'},
  {'6', '7', '8', '5'},
  {'C', '+', '0', 'T'},
  {'P', 'M', '{', 'O'},
  {'E', 'A', 'I', 'B'},

  {'N', 'D', 'Z', 'G'},
  {'K', 'J', 'H', 'L'}

};
byte rowPins[ROWS] = {10, 11, 14, 15, 17, 24, 25, 27 }; //connect to the row pinouts of the kpd
byte colPins[COLS] = {5, 3, 4, 2}; //connect to the column pinouts of the kpd

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
