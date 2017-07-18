#include <Makernet.h>

I2CDatalink um;
EncoderPeripheral enc;

void setup() {
  while ( !Serial );
   Serial.begin(115200);


  Makernet.initialize( DeviceType::Controller );
}

void loop() {
  Makernet.loop();
}
