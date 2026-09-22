#include "AnemometerPulseConverter.h"

// One anemometer closure per second is 2.4 km/h, in hundredths of a m/s.
#define WIND_HUNDREDTHS_MS_PER_HZ (2.4f / 3.6f * 100)

uint16_t AnemometerPulseConverter::hundredthsMetersPerSecond(uint32_t pulses, uint32_t elapsed_ms) {
  if (elapsed_ms == 0) return 0;
  float speed = pulses * WIND_HUNDREDTHS_MS_PER_HZ * 1000 / elapsed_ms + 0.5f;   // + 0.5 rounds to nearest
  return speed > 0xFFFF ? 0xFFFF : (uint16_t)speed;
}
