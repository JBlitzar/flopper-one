#include <Arduino.h>

#define PIN_SCLK 19
#define PIN_MOSI 18
#define PIN_DC   32
#define PIN_RST  33

void bbTransfer(uint8_t d) {
    for (int i = 7; i >= 0; i--) {
        digitalWrite(PIN_MOSI, (d >> i) & 1);
        digitalWrite(PIN_SCLK, HIGH);
        delayMicroseconds(2);
        digitalWrite(PIN_SCLK, LOW);
        delayMicroseconds(2);
    }
}

void bbCmd(uint8_t cmd) {
    digitalWrite(PIN_DC, LOW);
    bbTransfer(cmd);
    digitalWrite(PIN_DC, HIGH);
}

void bbData(uint8_t d) {
    digitalWrite(PIN_DC, HIGH);
    bbTransfer(d);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("=== BIT-BANG DISPLAY TEST ===");

    pinMode(PIN_SCLK, OUTPUT);
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_DC,   OUTPUT);
    pinMode(PIN_RST,  OUTPUT);

    digitalWrite(PIN_SCLK, LOW);
    digitalWrite(PIN_MOSI, LOW);

    // hard reset
    Serial.println("Resetting display...");
    digitalWrite(PIN_RST, HIGH);
    delay(50);
    digitalWrite(PIN_RST, LOW);
    delay(100);
    digitalWrite(PIN_RST, HIGH);
    delay(200);

    bbCmd(0x01); delay(150);
    bbCmd(0x11); delay(500);
    bbCmd(0x3A); bbData(0x55); delay(10);
    bbCmd(0x21); delay(10);
    bbCmd(0x29); delay(100);

    bbCmd(0x2A);
    bbData(0); bbData(0); bbData(0); bbData(239);
    bbCmd(0x2B);
    bbData(0); bbData(0); bbData(0); bbData(239);

    bbCmd(0x2C);
    Serial.println("Filling magenta (~30s)...");
    for (int i = 0; i < 240 * 240; i++) {
        bbData(0xF8);
        bbData(0x1F);
    }
    Serial.println("Done. Should be magenta.");
}

void loop() {
    delay(2000);
    Serial.println("alive");
}
