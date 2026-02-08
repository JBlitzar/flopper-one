#include <Arduino.h>

#include "flopper_pins.h"
#include "self_test.h"

static flopper::SelfTest selfTest(
    flopper::PinSet{
        flopper::pins::DPAD_UP,
        flopper::pins::DPAD_LEFT,
        flopper::pins::DPAD_RIGHT,
        flopper::pins::DPAD_DOWN,
        flopper::pins::DPAD_CENTER,
        flopper::pins::TSOP_OUT,
        flopper::pins::IR_LED,
        flopper::pins::PN532_SDA,
        flopper::pins::PN532_SCL,
    },
    flopper::SelfTestOptions(true, true, 20, true)
);

void setup() {
  Serial.begin(115200);
  delay(200);
  while(!Serial && millis() < 3000);
  delay(1000);
  Serial.printf("init...\n");

   selfTest.begin();
}

void loop() {
    Serial.println("Running...");
  delay(1000);
   selfTest.loop();
}
