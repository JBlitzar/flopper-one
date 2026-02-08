#include "self_test.h"

#include <Wire.h>


#include <TFT_eSPI.h>
#include <Adafruit_PN532.h>


namespace flopper {

static const char* levelStr(bool pressed) { return pressed ? "pressed" : "released"; }

SelfTest::SelfTest(const PinSet& pins, const SelfTestOptions& opts)
    : pins_(pins), opts_(opts) {
  buttons_[0] = {"UP", pins_.dpadUp, false, false, 0};
  buttons_[1] = {"LEFT", pins_.dpadLeft, false, false, 0};
  buttons_[2] = {"RIGHT", pins_.dpadRight, false, false, 0};
  buttons_[3] = {"DOWN", pins_.dpadDown, false, false, 0};
  buttons_[4] = {"CENTER", pins_.dpadCenter, false, false, 0};
}

bool SelfTest::readPressed(uint8_t pin) const {
  const int v = digitalRead(pin);
  if (opts_.buttonsActiveLow) return v == LOW;
  return v == HIGH;
}

void SelfTest::initButton(ButtonState& b) {
  pinMode(b.pin, opts_.buttonsActiveLow ? INPUT_PULLUP : INPUT);
  const bool p = readPressed(b.pin);
  b.lastStablePressed = p;
  b.lastRawPressed = p;
  b.lastChangeMs = millis();
}

void SelfTest::updateButton(ButtonState& b) {
  const uint32_t now = millis();
  const bool raw = readPressed(b.pin);

  if (raw != b.lastRawPressed) {
    b.lastRawPressed = raw;
    b.lastChangeMs = now;
  }

  if ((now - b.lastChangeMs) >= opts_.debounceMs && raw != b.lastStablePressed) {
    b.lastStablePressed = raw;
    Serial.printf("[SELFTEST][BTN] %s %s @%lums\n", b.name, levelStr(raw), (unsigned long)now);
  }
}

void SelfTest::printBanner() {
  Serial.println();
  Serial.println("[SELFTEST] ===== flopper-one hardware self-test =====");
  Serial.printf("[SELFTEST] build: %s %s\n", __DATE__, __TIME__);

#if defined(ARDUINO_ARCH_ESP32)
  Serial.printf("[SELFTEST] chip rev: %d\n", ESP.getChipRevision());
  Serial.printf("[SELFTEST] cpu freq: %u MHz\n", (unsigned)ESP.getCpuFreqMHz());
#endif
}



void SelfTest::runI2CScan() {
  if (!opts_.enableI2CScan) return;

  Serial.printf("[SELFTEST][I2C] scanning (SDA=%u SCL=%u) ...\n", pins_.i2cSda, pins_.i2cScl);
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    const uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.printf("[SELFTEST][I2C] found device @ 0x%02X\n", addr);
      found++;
    }
    delay(2);
  }
  if (found == 0) Serial.println("[SELFTEST][I2C] no devices found");
}

void SelfTest::tickPn532() {
  static bool inited = false;
  static bool present = false;
  static Adafruit_PN532 nfc(0xFF, 0xFF, &Wire); // I2C mode; IRQ/RESET optional

  static uint8_t lastUid[7] = {0};
  static uint8_t lastUidLen = 0;
  static uint32_t lastSeenMs = 0;
  static uint32_t lastPollMs = 0;

  const uint32_t now = millis();
  if (now - lastPollMs < 200) return;
  lastPollMs = now;

  if (!inited) {
    inited = true;
    nfc.begin();

    const uint32_t version = nfc.getFirmwareVersion();
    if (!version) {
      Serial.println("[SELFTEST][PN532] not detected (check wiring/power/I2C pins)");
      return;
    }

    present = true;
    Serial.printf("[SELFTEST][PN532] found chip PN5%02lX\n", (unsigned long)((version >> 24) & 0xFF));
    Serial.printf("[SELFTEST][PN532] firmware %lu.%lu\n",
                  (unsigned long)((version >> 16) & 0xFF),
                  (unsigned long)((version >> 8) & 0xFF));
    nfc.SAMConfig();
    nfc.setPassiveActivationRetries(0xFF);
    Serial.println("[SELFTEST][PN532] waiting for ISO14443A tags...");
  }

  if (!present) return;

  uint8_t uid[7] = {0};
  uint8_t uidLen = 0;
  const bool ok = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 10);
  if (!ok) return;

  const bool same = (uidLen == lastUidLen) && (memcmp(uid, lastUid, uidLen) == 0);
  if (same && (now - lastSeenMs) < 1000) return;

  memcpy(lastUid, uid, uidLen);
  lastUidLen = uidLen;
  lastSeenMs = now;

  Serial.printf("[SELFTEST][PN532] tag UID (%u bytes): ", uidLen);
  nfc.PrintHex(uid, uidLen);

  if (uidLen == 4) {
    uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t data[16];
    const bool authed = nfc.mifareclassic_AuthenticateBlock(uid, uidLen, 4, 0, keya);
    if (authed && nfc.mifareclassic_ReadDataBlock(4, data)) {
      Serial.print("[SELFTEST][PN532] Mifare Classic block 4: ");
      nfc.PrintHexChar(data, 16);
    } else {
      Serial.println("[SELFTEST][PN532] Mifare Classic auth/read failed (maybe different keys or not Classic)");
    }
  } else if (uidLen == 7) {
    uint8_t page[4];
    if (nfc.ntag2xx_ReadPage(4, page)) {
      Serial.print("[SELFTEST][PN532] NTAG page 4: ");
      nfc.PrintHexChar(page, 4);
    }
  }

}

void SelfTest::tickIrLed() {
  const uint32_t now = millis();
  if (now - lastIrToggleMs_ < 500) return;
  lastIrToggleMs_ = now;

  irState_ = !irState_;
  digitalWrite(pins_.irLed, irState_ ? HIGH : LOW);
  Serial.printf("[SELFTEST][IR] LED %s\n", irState_ ? "on" : "off");
}

void SelfTest::tickTsop() {
  if (pins_.tsopOut == 0xFF) return;
  const int lvl = digitalRead(pins_.tsopOut);
  if (lastTsopLevel_ < 0) {
    lastTsopLevel_ = lvl;
    return;
  }
  if (lvl != lastTsopLevel_) {
    lastTsopLevel_ = lvl;
    Serial.printf("[SELFTEST][IR] TSOP level=%d @%lums\n", lvl, (unsigned long)millis());
  }
}

void SelfTest::printPeriodicStats() {
  const uint32_t now = millis();
  if (now - lastStatsMs_ < 2000) return;
  lastStatsMs_ = now;

#if defined(ARDUINO_ARCH_ESP32)
  Serial.printf("[SELFTEST][SYS] uptime=%lums free_heap=%u\n", (unsigned long)now, (unsigned)ESP.getFreeHeap());
#else
  Serial.printf("[SELFTEST][SYS] uptime=%lums\n", (unsigned long)now);
#endif
}

void SelfTest::tickDisplay() {
  const uint32_t now = millis();
  if (now - lastDisplayMs_ < 1000) return;
  lastDisplayMs_ = now;


  static bool inited = false;
  static TFT_eSPI tft;
  if (!inited) {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("SELF TEST", 10, 10);
    inited = true;
    Serial.println("[SELFTEST][DISP] TFT_eSPI initialized");
  }

  tft.fillRect(10, 40, 220, 30, TFT_BLACK);
  tft.drawString(String("ms ") + String(now), 10, 40);

}

void SelfTest::begin() {
  active_ = true;//


  if (!active_) return;

  printBanner();

  pinMode(pins_.irLed, OUTPUT);
  digitalWrite(pins_.irLed, LOW);

  if (pins_.tsopOut != 0xFF) pinMode(pins_.tsopOut, INPUT);
  for (auto& b : buttons_) initButton(b);

#if defined(ARDUINO_ARCH_ESP32)
  Wire.begin(pins_.i2cSda, pins_.i2cScl);
#else
  Wire.begin();
#endif

  runI2CScan();
  tickPn532();
  Serial.println("[SELFTEST] ready: press/release buttons and watch serial output");
}

void SelfTest::loop() {

  if (!active_) return;

  for (auto& b : buttons_) updateButton(b);
  tickPn532();
  tickIrLed();
  tickTsop();
  tickDisplay();
  printPeriodicStats();
}

}
