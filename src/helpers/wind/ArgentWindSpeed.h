#pragma once

#include <stdint.h>

#include "ArgentWindConfig.h"

// Wind speed from the Argent Data anemometer: an interrupt-counted reed switch on WIND_ANEMOMETER_PIN (a
// build flag), ticked into a sliding window once per few seconds to compute a sustained mean and a gust
// peak. The pulse-to-speed conversion and the averaging window are hardware-independent implementation
// details, unit tested directly (see AnemometerPulseConverter.h and BucketRing.h) rather than through
// this class.
class ArgentWindSpeed {
public:
  static void begin();
  static void loop();   // call every main loop iteration
  static uint16_t readSustained();
  static uint16_t readGust();
};
