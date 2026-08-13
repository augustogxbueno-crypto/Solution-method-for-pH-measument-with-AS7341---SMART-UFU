// =============================================================================
// core/SensorManager.h — this project always compiles the AS7341, so unlike
// the original multi-sensor tutorial series there's no build-flag switch.
// =============================================================================
#pragma once
#include "../sensors/AS7341.h"

namespace SensorManager {

inline SensorBase& get() {
  static AS7341Sensor instance;
  return instance;
}

} // namespace SensorManager
