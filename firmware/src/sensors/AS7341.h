#pragma once
#include "SensorBase.h"
#include <Adafruit_AS7341.h>

// AS7341 driver, reflectance-only: internal LED on, sample against whatever
// the cuvette/sample transmits back at each of the 8 channels. Integration
// time is fixed internally (ATIME/ASTEP below) — only gain, LED current and
// number of samples are exposed to the UI, matching the SmartPad Controller
// layout this project's frontend is based on.
class AS7341Sensor : public SensorBase {
 public:
  bool begin() override;

  const __FlashStringHelper* name() const override;

  uint8_t channelCount() const override { return 8; }
  ChannelInfo channel(uint8_t idx) const override;
  bool readChannels(float* out) override;

  uint8_t gainOptionCount() const override { return 11; }
  const __FlashStringHelper* gainOptionLabel(uint8_t idx) const override;
  void setGainIndex(uint8_t idx) override;

  int ledMinMA() const override { return 4; }
  int ledMaxMA() const override { return 258; }
  void setLedOn(bool on) override;
  void setLedContinuous(int mA) override;

  // Default LED current applied at boot (see AS7341.cpp::begin() — the LED
  // must have a current set BEFORE it's enabled, or it stays dark even
  // though enableLED(true) reports success). Matches the screenshot's
  // default of 60mA.
  static constexpr int DEFAULT_LED_MA = 60;

 private:
  Adafruit_AS7341 sensor_;
  uint8_t gainIdx_ = 4; // 16X, matches the screenshot's default

  // Fixed integration time (not exposed to the UI): (100+1)*(999+1)*2.78us ~= 280ms
  static constexpr uint8_t  ATIME_ = 100;
  static constexpr uint16_t ASTEP_ = 999;
};
