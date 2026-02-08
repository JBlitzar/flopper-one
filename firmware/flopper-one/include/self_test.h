#pragma once

#include <Arduino.h>

namespace flopper {

struct SelfTestPins {
  uint8_t dpadUp;
  uint8_t dpadLeft;
  uint8_t dpadRight;
  uint8_t dpadDown;
  uint8_t dpadCenter;
  uint8_t tsopOut;
  uint8_t irLed;
  uint8_t i2cSda;
  uint8_t i2cScl;
};

struct SelfTestOptions {
  bool buttonsActiveLow = true;
  bool enableI2CScan = true;
  uint16_t debounceMs = 20;

  // If true, self-test mode starts when DPAD_CENTER is held during boot.
  bool bootHoldToEnter = true;
  uint16_t bootHoldMs = 750;

  // If true, always run self-test (useful while bringing up hardware).
  bool defaultOn = true;

  constexpr SelfTestOptions() = default;
  constexpr SelfTestOptions(bool buttonsActiveLow_,
                            bool enableI2CScan_,
                            uint16_t debounceMs_,
                            bool bootHoldToEnter_,
                            uint16_t bootHoldMs_,
                            bool defaultOn_)
      : buttonsActiveLow(buttonsActiveLow_),
        enableI2CScan(enableI2CScan_),
        debounceMs(debounceMs_),
        bootHoldToEnter(bootHoldToEnter_),
        bootHoldMs(bootHoldMs_),
        defaultOn(defaultOn_) {}
};

class SelfTest {
public:
  SelfTest(const SelfTestPins& pins, const SelfTestOptions& opts);

  void begin();
  void loop();

  bool isActive() const { return active_; }
  void setActive(bool on) { active_ = on; }

  bool shouldEnterOnBoot() const;

private:
  struct ButtonState {
    const char* name;
    uint8_t pin;
    bool lastStablePressed;
    bool lastRawPressed;
    uint32_t lastChangeMs;
  };

  bool readPressed(uint8_t pin) const;
  void initButton(ButtonState& b);
  void updateButton(ButtonState& b);

  void printBanner();
  void printPeriodicStats();
  void runI2CScan();
  void tickIrLed();
  void tickTsop();
  void tickDisplay();

  SelfTestPins pins_;
  SelfTestOptions opts_;
  bool active_ = false;

  ButtonState buttons_[5];
  uint32_t lastStatsMs_ = 0;
  uint32_t lastIrToggleMs_ = 0;
  bool irState_ = false;

  int lastTsopLevel_ = -1;
  uint32_t lastDisplayMs_ = 0;
};

}
