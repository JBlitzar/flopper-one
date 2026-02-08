#include <Arduino.h>
#define TSOP_OUT_PORT = 23 // GPIO 23 to TSOP IR reciever
#define DPAD_UP_PORT = 14
#define DPAD_LEFT_PORT = 27
#define DPAD_RIGHT_PORT = 12
#define DPAD_DOWN_PORT = 26
#define DPAD_CENTER_PORT = 25
#define PN532_SDA = 17 // PN532 nfc/rfid module
#define PN532_SCL = 15
#define SD7789_SCL = 18 // SD7789 display 1.3"
#define SD7789_SDA = 19
#define IR_LED_PORT = 16 // just an ir led hooked up to gpio16

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}