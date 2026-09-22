#pragma once

#include <stdint.h>

// Decodes the Argent Data wind vane's resistance-ladder divider: eight reed switches with resistors, 16
// positions (a magnet can close two at once), read as the lower leg of a voltage divider. Headings are in
// tenths of a degree, since the 22.5 degree steps aren't whole degrees. Pulled out of ArgentWindDirection
// so the lookup table and decode logic can be unit tested directly (see
// test_resistive_vane_decoder.cpp) without exposing them through ArgentWindDirection.h.

#define WIND_VANE_POSITION_COUNT 16

class ResistiveVaneDecoder {
public:
  // Resistance and heading of vane position `i` (0 <= i < WIND_VANE_POSITION_COUNT), ordered by
  // ascending resistance. For inspecting or verifying the table; production code goes through decode()
  // instead.
  static uint32_t positionOhms(int i);
  static uint16_t positionHeadingTenths(int i);

  // Map an ADC reading of the vane divider (`adc_max` full scale, `pullup_ohms` to full-scale voltage)
  // to a heading in tenths of a degree, taking the nearest position. Returns false, leaving
  // `heading_tenths` alone, for an open circuit (all reeds open while the magnet moves between two, or
  // the vane unplugged) or a short. Needs adc_max <= 14 bits to fit in 32 bits.
  static bool decode(uint32_t counts, uint32_t adc_max, uint32_t pullup_ohms, uint16_t& heading_tenths);
};
