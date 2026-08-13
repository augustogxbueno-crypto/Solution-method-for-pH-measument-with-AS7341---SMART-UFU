// =============================================================================
// SensorBase.h — minimal contract for the AS7341, reflectance-only.
// -----------------------------------------------------------------------------
// Trimmed down from the original multi-sensor SensorBase (which also
// supported discrete/binary LEDs, formula/preset integration time, and
// extra sensor-specific params). This project only ever compiles the
// AS7341 in continuous-LED reflectance mode, so all of that is gone —
// see the original IoT-Low-cost-Adafruit-Sensors-Tutorial- repo if you
// need the full generic version back.
// =============================================================================
#pragma once
#include <Arduino.h>

struct ChannelInfo {
  const __FlashStringHelper* name;   // e.g. "415nm"
  const __FlashStringHelper* color;  // hex string, used only for the UI swatch
};

class SensorBase {
 public:
  virtual ~SensorBase() {}

  virtual bool begin() = 0;
  virtual const __FlashStringHelper* name() const = 0;

  virtual uint8_t channelCount() const = 0;
  virtual ChannelInfo channel(uint8_t idx) const = 0;
  virtual bool readChannels(float* out) = 0; // fills out[0..channelCount()-1]

  // ---- Gain (discrete list, e.g. AS7341's 0.5X..512X) ----
  virtual uint8_t gainOptionCount() const = 0;
  virtual const __FlashStringHelper* gainOptionLabel(uint8_t idx) const = 0;
  virtual void setGainIndex(uint8_t idx) = 0;

  // ---- LED (continuous current, in mA) ----
  virtual int ledMinMA() const = 0;
  virtual int ledMaxMA() const = 0;
  virtual void setLedOn(bool on) = 0;
  virtual void setLedContinuous(int mA) = 0;
};
