#include "ArgentRain.h"

#include "TippingBucketConverter.h"

/* ------------------------------ Hardware -------------------------------- */

#if ENV_INCLUDE_RAIN

#include <Arduino.h>

#ifndef WIND_RAIN_DEBOUNCE_MS
  #define WIND_RAIN_DEBOUNCE_MS 50
#endif

static volatile uint32_t g_tips = 0;
static volatile uint32_t g_last_tip_ms = 0;

static void onRainTip() {
  uint32_t now = millis();
  if (now - g_last_tip_ms >= WIND_RAIN_DEBOUNCE_MS) {
    g_last_tip_ms = now;
    g_tips++;
  }
}

void ArgentRain::begin() {
  pinMode(WIND_RAIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(WIND_RAIN_PIN), onRainTip, FALLING);
}

uint16_t ArgentRain::read() { return TippingBucketConverter::tenthsMm(g_tips); }

#endif  // ENV_INCLUDE_RAIN
