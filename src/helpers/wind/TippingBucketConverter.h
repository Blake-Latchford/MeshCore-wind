#pragma once

#include <stdint.h>

// Converts tipping-bucket rain gauge tips into rainfall depth. The Argent Data gauge tips once per
// 0.2794 mm (0.011"). Pulled out of ArgentRain so the conversion math can be unit tested directly (see
// test_tipping_bucket_converter.cpp) without exposing it through ArgentRain.h.
class TippingBucketConverter {
public:
  // Cumulative rainfall in 0.1 mm for a bucket tip count. Stops at 6553.5 mm, the most the 16-bit wire
  // field holds.
  static uint16_t tenthsMm(uint32_t tips);
};
