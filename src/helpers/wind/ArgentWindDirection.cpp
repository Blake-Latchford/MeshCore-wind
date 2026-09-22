#include "ArgentWindDirection.h"

#include "ResistiveVaneDecoder.h"

/* ------------------------------ Hardware -------------------------------- */

#if ENV_INCLUDE_WIND_DIRECTION

#include <Arduino.h>

#ifndef WIND_VANE_PULLUP_OHMS
  #define WIND_VANE_PULLUP_OHMS 10000      // from 3V3_S to the vane, ADC on the junction
#endif
#ifndef WIND_VANE_OFFSET_DEG
  #define WIND_VANE_OFFSET_DEG 0           // add to correct for how the vane is mounted, 0-359
#endif
#ifndef WIND_VANE_SAMPLE_MS
  #define WIND_VANE_SAMPLE_MS 3000         // how often the vane ADC is read
#endif
#define WIND_VANE_ADC_BITS 12

// Heading of the last valid reading, tenths of a degree. Reads 0 (north) until the first valid reading,
// and keeps the last heading once the vane stops reading validly — only ever written on a valid decode.
static uint16_t g_heading_tenths = 0;
static uint32_t g_last_sample_ms = 0;

static uint32_t readVaneCounts() {
#ifdef NRF52_PLATFORM
  // Read against VDD, not the internal reference: the divider is fed from 3V3_S, so measuring against
  // the same rail makes the reading independent of the rail voltage. The reference is global and the
  // battery read relies on the default, so put it back.
  analogReference(AR_VDD4);
  analogReadResolution(WIND_VANE_ADC_BITS);
  uint32_t counts = analogRead(WIND_VANE_PIN);
  analogReference(AR_DEFAULT);
  return counts;
#else
  return 0;
#endif
}

void ArgentWindDirection::begin() {
  pinMode(WIND_VANE_PIN, INPUT);
  g_last_sample_ms = millis();
}

void ArgentWindDirection::loop() {
  uint32_t now = millis();
  if (now - g_last_sample_ms < WIND_VANE_SAMPLE_MS) return;
  g_last_sample_ms = now;

  uint16_t heading_tenths;
  if (ResistiveVaneDecoder::decode(readVaneCounts(), (1u << WIND_VANE_ADC_BITS) - 1, WIND_VANE_PULLUP_OHMS, heading_tenths)) {
    g_heading_tenths = heading_tenths;
  }
}

uint16_t ArgentWindDirection::read() {
  return ((g_heading_tenths + 5) / 10 + WIND_VANE_OFFSET_DEG) % 360;
}

#endif  // ENV_INCLUDE_WIND_DIRECTION
