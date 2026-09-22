#pragma once

#include <stdint.h>

#include "ArgentWindConfig.h"

// Rainfall from the Argent Data tipping-bucket rain gauge: an interrupt-counted gauge on WIND_RAIN_PIN (a
// build flag). No loop() — the ISR does all the work, and reading just converts the running tip count.
// The tip-to-depth conversion is a hardware-independent implementation detail, unit tested directly (see
// TippingBucketConverter.h) rather than through this class.
class ArgentRain {
public:
  static void begin();
  static uint16_t read();   // 0.1 mm, stops at 6553.5 mm
};
