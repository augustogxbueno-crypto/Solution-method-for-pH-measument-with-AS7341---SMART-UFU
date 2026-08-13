// =============================================================================
// core/Measurement.h — averages N raw reads of the AS7341's 8 channels.
// -----------------------------------------------------------------------------
// Simplified from the original tutorial series: no reflectance/absorbance/
// fluorescence mode switch, no log-vs-reference submode, no time-based
// 10-second window. This project only ever does one thing: read the AS7341
// in reflectance mode N times and average, where N is "Number of samples"
// in the UI (matching the SmartPad Controller screenshot this is based on).
// =============================================================================
#pragma once
#include <Arduino.h>

namespace Measurement {

constexpr uint8_t CHANNEL_COUNT = 8;

extern uint8_t numSamples; // "Number of samples" — default 1, like the screenshot

void setNumSamples(uint8_t n);

// Invoked once per sample, so Protocol.cpp can stream progress without
// Measurement knowing anything about JSON.
using ProgressCallback = void (*)(uint8_t sampleIndex, const float* sampleValues);

// Reads numSamples samples and averages them into avgOut[0..CHANNEL_COUNT-1].
// Returns false if the sensor read fails; check lastError() for why.
bool run(ProgressCallback onProgress, float* avgOut);

const __FlashStringHelper* lastError();

} // namespace Measurement
