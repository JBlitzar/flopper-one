#include <Arduino.h>

#define PIN_SCLK 18
#define PIN_MOSI 19
#define PIN_DC   22
#define PIN_RST  21

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

void initAndFill(uint8_t hi, uint8_t lo, const char* color) {
    Serial.printf("--- %s ---\n", color);

    Serial.println("RST HIGH");
    digitalWrite(PIN_RST, HIGH); delay(50);
    Serial.println("RST LOW");
    digitalWrite(PIN_RST, LOW);  delay(100);
    Serial.println("RST HIGH");
    digitalWrite(PIN_RST, HIGH); delay(200);

    Serial.printf("RST pin %d reads: %d\n", PIN_RST, digitalRead(PIN_RST));
    Serial.printf("DC  pin %d reads: %d\n", PIN_DC,  digitalRead(PIN_DC));

    Serial.println("SWRST");    bbCmd(0x01); delay(150);
    Serial.println("SLPOUT");   bbCmd(0x11); delay(500);
    Serial.println("COLMOD");   bbCmd(0x3A); bbData(0x55); delay(10);
    Serial.println("INVON");    bbCmd(0x21); delay(10);
    Serial.println("DISPON");   bbCmd(0x29); delay(100);

    bbCmd(0x2A);
    bbData(0); bbData(0); bbData(0); bbData(239);
    bbCmd(0x2B);
    bbData(0); bbData(0); bbData(0); bbData(239);

    Serial.println("RAMWR start");
    bbCmd(0x2C);
    for (int i = 0; i < 240 * 240; i++) {
        bbData(hi); bbData(lo);
    }
    Serial.printf("Done %s.\n", color);
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

    initAndFill(0xF8, 0x1F, "MAGENTA");
}

void loop() {
    delay(5000);
    initAndFill(0x07, 0xE0, "GREEN");
    delay(5000);
    initAndFill(0xF8, 0x00, "RED");
    delay(5000);
    initAndFill(0x00, 0x1F, "BLUE");
    delay(5000);
    initAndFill(0xF8, 0x1F, "MAGENTA");
}
