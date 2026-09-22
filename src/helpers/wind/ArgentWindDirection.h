#pragma once

#include <stdint.h>

#include "ArgentWindConfig.h"

// Wind direction from the Argent Data wind vane: a vane on WIND_VANE_PIN (a build flag), sampled every
// WIND_VANE_SAMPLE_MS. Headings are read in degrees, 0-359, holding the last valid heading across moments
// the vane reads no position. The resistance-ladder decode is a hardware-independent implementation
// detail, unit tested directly (see ResistiveVaneDecoder.h) rather than through this class.
class ArgentWindDirection {
public:
  static void begin();
  static void loop();   // call every main loop iteration
  static uint16_t read();   // degrees, 0-359, after WIND_VANE_OFFSET_DEG
};
