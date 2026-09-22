#pragma once

#include <stdint.h>

// Converts anemometer closures into wind speed. The Argent Data anemometer is a reed switch that closes
// once per second at 2.4 km/h. Pulled out of ArgentWindSpeed so the conversion math can be unit tested
// directly (see test_anemometer_pulse_converter.cpp) without exposing it through ArgentWindSpeed.h.
class AnemometerPulseConverter {
public:
  // Wind speed in 0.01 m/s from `pulses` anemometer closures counted over `elapsed_ms`.
  static uint16_t hundredthsMetersPerSecond(uint32_t pulses, uint32_t elapsed_ms);
};
