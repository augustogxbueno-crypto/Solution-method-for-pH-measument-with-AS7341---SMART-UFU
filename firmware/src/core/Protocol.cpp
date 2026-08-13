#include "Protocol.h"
#include "Utils.h"
#include "SensorManager.h"
#include "Measurement.h"
#include <ArduinoJson.h>

constexpr size_t CMD_DOC_CAPACITY    = JSON_OBJECT_SIZE(3) + 32;
constexpr size_t PROGRESS_DOC_CAPACITY = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(8) + 24;
constexpr size_t RESULT_DOC_CAPACITY   = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 24;
constexpr size_t SMALL_DOC_CAPACITY    = JSON_OBJECT_SIZE(3) + 32;

namespace Protocol {

static void sendJson(JsonDocument& doc) {
  serializeJson(doc, Serial);
  Serial.print('\n');
}

static void sendError(const __FlashStringHelper* msg) {
  StaticJsonDocument<SMALL_DOC_CAPACITY> doc;
  doc["evt"] = F("error");
  doc["msg"] = msg;
  sendJson(doc);
}

static void sendAck(const char* cmd) {
  StaticJsonDocument<SMALL_DOC_CAPACITY> doc;
  doc["evt"] = F("ack");
  doc["cmd"] = cmd;
  sendJson(doc);
}

static void printQuoted(const __FlashStringHelper* s) {
  Serial.print('"');
  Serial.print(s);
  Serial.print('"');
}

static int     g_ledMA   = 60; // matches the screenshot's default
static uint8_t g_gainIdx = 4;  // 16X, matches the screenshot's default

// -----------------------------------------------------------------------------
// "info" — self-description the frontend uses to build the setup panel.
// -----------------------------------------------------------------------------
void sendInfo() {
  SensorBase& sensor = SensorManager::get();

  Serial.print(F("{\"evt\":\"info\",\"sensor\":"));
  printQuoted(sensor.name());

  Serial.print(F(",\"channels\":["));
  for (uint8_t i = 0; i < sensor.channelCount(); i++) {
    if (i) Serial.print(',');
    ChannelInfo ci = sensor.channel(i);
    Serial.print(F("{\"id\":"));
    Serial.print(i);
    Serial.print(F(",\"name\":"));
    printQuoted(ci.name);
    Serial.print(F(",\"color\":"));
    printQuoted(ci.color);
    Serial.print('}');
  }
  Serial.print(']');

  Serial.print(F(",\"gain\":{\"options\":["));
  for (uint8_t i = 0; i < sensor.gainOptionCount(); i++) {
    if (i) Serial.print(',');
    printQuoted(sensor.gainOptionLabel(i));
  }
  Serial.print(F("],\"default\":"));
  Serial.print(g_gainIdx);
  Serial.print('}');

  Serial.print(F(",\"led\":{\"minMA\":"));
  Serial.print(sensor.ledMinMA());
  Serial.print(F(",\"maxMA\":"));
  Serial.print(sensor.ledMaxMA());
  Serial.print(F(",\"default\":"));
  Serial.print(g_ledMA);
  Serial.print('}');

  Serial.print(F("}\n"));
}

static void onProgress(uint8_t sampleIndex, const float* sampleValues) {
  StaticJsonDocument<PROGRESS_DOC_CAPACITY> doc;
  doc["evt"] = F("progress");
  doc["n"]   = sampleIndex + 1;
  JsonObject data = doc.createNestedObject("data");
  char key[3];
  for (uint8_t i = 0; i < Measurement::CHANNEL_COUNT; i++) {
    Utils::smallIndexKey(i, key);
    data[key] = sampleValues[i];
  }
  sendJson(doc);
}

static void runAndSendMeasurement() {
  float avg[Measurement::CHANNEL_COUNT];
  if (!Measurement::run(onProgress, avg)) {
    sendError(Measurement::lastError());
    return;
  }

  SensorBase& sensor = SensorManager::get();
  StaticJsonDocument<RESULT_DOC_CAPACITY> doc;
  doc["evt"]     = F("result");
  doc["n"]       = Measurement::numSamples;
  doc["gain"]    = sensor.gainOptionLabel(g_gainIdx);
  doc["led_ma"]  = g_ledMA;

  JsonObject data = doc.createNestedObject("data");
  char key[3];
  for (uint8_t i = 0; i < Measurement::CHANNEL_COUNT; i++) {
    Utils::smallIndexKey(i, key);
    data[key] = avg[i];
  }

  sendJson(doc);
}

// -----------------------------------------------------------------------------
// Command dispatch
// -----------------------------------------------------------------------------
void handleCommand(const char* line) {
  StaticJsonDocument<CMD_DOC_CAPACITY> doc;
  DeserializationError err = deserializeJson(doc, line);
  if (err) {
    sendError(F("Invalid JSON"));
    return;
  }

  const char* cmd = doc["cmd"] | "";
  SensorBase& sensor = SensorManager::get();

  if (strcmp(cmd, "get_info") == 0) {
    sendInfo();

  } else if (strcmp(cmd, "set_gain") == 0) {
    int idx = doc["idx"] | 0;
    if (idx < 0 || idx >= sensor.gainOptionCount()) { sendError(F("Gain out of range")); return; }
    g_gainIdx = (uint8_t)idx;
    sensor.setGainIndex(g_gainIdx);
    sendAck("set_gain");

  } else if (strcmp(cmd, "set_led") == 0) {
    int mA = doc["current"] | sensor.ledMinMA();
    mA = constrain(mA, sensor.ledMinMA(), sensor.ledMaxMA());
    g_ledMA = mA;
    sensor.setLedContinuous(g_ledMA);
    sensor.setLedOn(true);
    sendAck("set_led");

  } else if (strcmp(cmd, "set_samples") == 0) {
    int n = doc["n"] | 1;
    if (n < 1) n = 1;
    if (n > 255) n = 255;
    Measurement::setNumSamples((uint8_t)n);
    sendAck("set_samples");

  } else if (strcmp(cmd, "measure") == 0) {
    runAndSendMeasurement();

  } else {
    sendError(F("Unknown command"));
  }
}

} // namespace Protocol
