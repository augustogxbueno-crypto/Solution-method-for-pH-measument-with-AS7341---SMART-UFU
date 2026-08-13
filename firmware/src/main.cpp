// =============================================================================
// pH measure with AS7341 — main.cpp
// -----------------------------------------------------------------------------
// Reflectance-only AS7341 firmware for a cuvette-based colorimetric pH
// reader. Brings up Serial/I2C/the AS7341, reads incoming Web Serial
// commands and hands them to Protocol::handleCommand(). See README.md for
// the wire protocol and wiring.
//
// Backend adapted from the IoT-Low-cost-Adafruit-Sensors-Tutorial- project
// (same AS7341 driver + Web Serial JSON protocol), trimmed to reflectance
// mode only. Frontend rebuilt to match the SmartPad Controller UI.
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include "core/SensorManager.h"
#include "core/Protocol.h"

constexpr uint8_t LINE_BUFFER_SIZE = 96;
char    lineBuffer[LINE_BUFFER_SIZE];
uint8_t lineLen = 0;

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { /* wait for USB port (native USB boards) */ }

  Wire.begin();

  if (!SensorManager::get().begin()) {
    while (true) {
      if (Serial.available()) Serial.read();
      Serial.println(F("{\"evt\":\"error\",\"msg\":\"AS7341 not found. Check I2C wiring.\"}"));
      delay(1000);
    }
  }
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      lineBuffer[lineLen] = '\0';
      if (lineLen > 0) Protocol::handleCommand(lineBuffer);
      lineLen = 0;
    } else if (c != '\r') {
      if (lineLen < LINE_BUFFER_SIZE - 1) {
        lineBuffer[lineLen++] = c;
      } else {
        lineLen = 0; // line too long: drop it instead of overrunning memory
      }
    }
  }
}
