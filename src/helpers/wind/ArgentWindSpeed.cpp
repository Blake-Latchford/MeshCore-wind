#include "ArgentWindSpeed.h"

#include "AnemometerPulseConverter.h"
#include "BucketRing.h"

/* ------------------------------ Hardware -------------------------------- */

#if ENV_INCLUDE_WIND_SPEED

#include <Arduino.h>

#ifndef WIND_WINDOW_SECS
  #define WIND_WINDOW_SECS 600      // reporting window: sustained = mean, gust = highest bucket mean (WMO: 10 min)
#endif
#define WIND_BUCKET_SECS 3          // anemometer counting period, and the gust averaging period (WMO: 3 s)
#define WIND_WINDOW_BUCKETS (WIND_WINDOW_SECS / WIND_BUCKET_SECS)

static_assert(WIND_WINDOW_SECS % WIND_BUCKET_SECS == 0, "WIND_WINDOW_SECS must be a whole number of buckets");

#ifndef WIND_ANEMOMETER_DEBOUNCE_MS
  #define WIND_ANEMOMETER_DEBOUNCE_MS 5    // caps at 200 Hz, far above the fastest real wind
#endif

// The buckets are aligned rather than a running average, so a gust that straddles two buckets is split
// between them and reads lower than it was.
static BucketRing<WIND_WINDOW_BUCKETS> g_window;
static volatile uint32_t g_pulses = 0;
static volatile uint32_t g_last_pulse_ms = 0;
static uint32_t g_last_tick_ms = 0;
static uint32_t g_last_pulses = 0;

static void onAnemometerPulse() {
  uint32_t now = millis();
  if (now - g_last_pulse_ms >= WIND_ANEMOMETER_DEBOUNCE_MS) {
    g_last_pulse_ms = now;
    g_pulses++;
  }
}

void ArgentWindSpeed::begin() {
  pinMode(WIND_ANEMOMETER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WIND_ANEMOMETER_PIN), onAnemometerPulse, FALLING);
  g_last_tick_ms = millis();
  g_last_pulses = g_pulses;
}

void ArgentWindSpeed::loop() {
  uint32_t now = millis();
  uint32_t elapsed_ms = now - g_last_tick_ms;
  if (elapsed_ms < WIND_BUCKET_SECS * 1000) return;

  uint32_t pulses = g_pulses;
  uint16_t speed = AnemometerPulseConverter::hundredthsMetersPerSecond(pulses - g_last_pulses, elapsed_ms);
  g_window.record(speed, elapsed_ms / (WIND_BUCKET_SECS * 1000));
  g_last_tick_ms = now;
  g_last_pulses = pulses;
}

uint16_t ArgentWindSpeed::readSustained() { return g_window.mean(); }

uint16_t ArgentWindSpeed::readGust() { return g_window.max(); }

#endif  // ENV_INCLUDE_WIND_SPEED
