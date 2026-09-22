#include "TippingBucketConverter.h"

// 0.2794 mm (0.011") per bucket tip, in tenths of a mm.
#define RAIN_TENTHS_MM_PER_TIP 2.794f

uint16_t TippingBucketConverter::tenthsMm(uint32_t tips) {
  float tenths = tips * RAIN_TENTHS_MM_PER_TIP + 0.5f;
  return tenths > 0xFFFF ? 0xFFFF : (uint16_t)tenths;
}
