#include "self_test.h"

#include <Wire.h>

// Optional display support (only compiles if the library exists)
#if __has_include(<TFT_eSPI.h>)
  #include <TFT_eSPI.h>
  #define FLOPPER_HAS_TFT_ESPI 1
#else
  #define FLOPPER_HAS_TFT_ESPI 0
#endif

namespace flopper {

static const char* levelStr(bool pressed) { return pressed ? "pressed" : "released"; }

SelfTest::SelfTest(const SelfTestPins& pins, const SelfTestOptions& opts)
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

bool SelfTest::shouldEnterOnBoot() const {
  if (!opts_.bootHoldToEnter) return false;

  pinMode(pins_.dpadCenter, opts_.buttonsActiveLow ? INPUT_PULLUP : INPUT);
  const uint32_t start = millis();
  while (millis() - start < opts_.bootHoldMs) {
    if (!readPressed(pins_.dpadCenter)) return false;
    delay(10);
  }
  return true;
}

void SelfTest::runI2CScan() {
  if (!opts_.enableI2CScan) return;

#if defined(ARDUINO_ARCH_ESP32)
  Wire.begin(pins_.i2cSda, pins_.i2cScl);
#else
  Wire.begin();
#endif

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

#if FLOPPER_HAS_TFT_ESPI
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
#else
  (void)now;
#endif
}

void SelfTest::begin() {
  active_ = opts_.defaultOn || shouldEnterOnBoot();

  if (!active_) return;

  printBanner();

  pinMode(pins_.irLed, OUTPUT);
  digitalWrite(pins_.irLed, LOW);

  if (pins_.tsopOut != 0xFF) pinMode(pins_.tsopOut, INPUT);
  for (auto& b : buttons_) initButton(b);

  runI2CScan();
  Serial.println("[SELFTEST] ready: press/release buttons and watch serial output");
}

void SelfTest::loop() {
  if (!active_) return;

  for (auto& b : buttons_) updateButton(b);
  tickIrLed();
  tickTsop();
  tickDisplay();
  printPeriodicStats();
}

}
