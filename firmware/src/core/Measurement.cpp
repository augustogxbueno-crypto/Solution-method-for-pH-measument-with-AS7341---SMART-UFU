#include "Measurement.h"
#include "SensorManager.h"

namespace Measurement {

uint8_t numSamples = 1;

static const __FlashStringHelper* g_error = nullptr;

const __FlashStringHelper* lastError() { return g_error; }

void setNumSamples(uint8_t n) {
  if (n < 1) n = 1;
  numSamples = n;
}

bool run(ProgressCallback onProgress, float* avgOut) {
  g_error = nullptr;
  SensorBase& sensor = SensorManager::get();

  float sum[CHANNEL_COUNT] = { 0 };
  float sample[CHANNEL_COUNT];
  uint8_t okCount = 0;

  for (uint8_t s = 0; s < numSamples; s++) {
    if (!sensor.readChannels(sample)) continue;
    okCount++;
    for (uint8_t i = 0; i < CHANNEL_COUNT; i++) sum[i] += sample[i];
    if (onProgress) onProgress(s, sample);
  }

  if (okCount == 0) {
    g_error = F("Sensor read failed.");
    return false;
  }

  for (uint8_t i = 0; i < CHANNEL_COUNT; i++) avgOut[i] = sum[i] / okCount;
  return true;
}

} // namespace Measurement
