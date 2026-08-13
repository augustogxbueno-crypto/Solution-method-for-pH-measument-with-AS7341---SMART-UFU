// =============================================================================
// core/Protocol.h — the only file that knows about JSON.
// Wire protocol (see README.md):
//   Browser -> Board: get_info, set_gain, set_led, set_samples, measure
//   Board -> Browser: info, ack, progress, result, error
// =============================================================================
#pragma once
#include <Arduino.h>

namespace Protocol {

void sendInfo();
void handleCommand(const char* line);

} // namespace Protocol
