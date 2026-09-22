#include "ResistiveVaneDecoder.h"

namespace {

struct VanePosition {
  uint32_t ohms;
  uint16_t heading_tenths;
};

const VanePosition WIND_VANE_POSITIONS[WIND_VANE_POSITION_COUNT] = {
  {   688, 1125 }, {   891,  675 }, {  1000,  900 }, {  1410, 1575 },
  {  2200, 1350 }, {  3140, 2025 }, {  3900, 1800 }, {  6570,  225 },
  {  8200,  450 }, { 14120, 2475 }, { 16000, 2250 }, { 21880, 3375 },
  { 33000,    0 }, { 42120, 2925 }, { 64900, 3150 }, { 120000, 2700 },
};

}  // namespace

uint32_t ResistiveVaneDecoder::positionOhms(int i) { return WIND_VANE_POSITIONS[i].ohms; }
uint16_t ResistiveVaneDecoder::positionHeadingTenths(int i) { return WIND_VANE_POSITIONS[i].heading_tenths; }

// A vane resistance outside this range can't be any position: above it is an open circuit and below it
// is a short.
#define WIND_VANE_MIN_OHMS 344      // half the lowest position (688)
#define WIND_VANE_MAX_OHMS 240000   // twice the highest position (120k)

// Counts an ADC should read for a vane of `ohms` in the divider.
static uint32_t expectedCounts(uint32_t ohms, uint32_t adc_max, uint32_t pullup_ohms) {
  return (adc_max * ohms + (ohms + pullup_ohms) / 2) / (ohms + pullup_ohms);
}

bool ResistiveVaneDecoder::decode(uint32_t counts, uint32_t adc_max, uint32_t pullup_ohms, uint16_t& heading_tenths) {
  if (counts < expectedCounts(WIND_VANE_MIN_OHMS, adc_max, pullup_ohms) ||
      counts > expectedCounts(WIND_VANE_MAX_OHMS, adc_max, pullup_ohms)) {
    return false;
  }

  int best = 0;
  uint32_t best_distance = 0xFFFFFFFF;
  for (int i = 0; i < WIND_VANE_POSITION_COUNT; i++) {
    uint32_t expected = expectedCounts(WIND_VANE_POSITIONS[i].ohms, adc_max, pullup_ohms);
    uint32_t distance = counts > expected ? counts - expected : expected - counts;
    if (distance < best_distance) {
      best_distance = distance;
      best = i;
    }
  }
  heading_tenths = WIND_VANE_POSITIONS[best].heading_tenths;
  return true;
}
