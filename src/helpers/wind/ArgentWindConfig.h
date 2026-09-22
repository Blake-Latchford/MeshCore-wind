#pragma once

// Which parts of the Argent Data wind/rain assembly a board wires up, derived from which pins the
// variant's platformio.ini defines — no separate opt-in flag to keep in sync with the pins, and no way
// for a flag to be set with its pin missing. ENV_INCLUDE_WIND is the umbrella, on if any one part is;
// code shared across all three (the combined telemetry reply) stays behind it, while each part's own
// logic stays behind its own macro.

#define ENV_INCLUDE_WIND_SPEED defined(WIND_ANEMOMETER_PIN)
#define ENV_INCLUDE_WIND_DIRECTION defined(WIND_VANE_PIN)
#define ENV_INCLUDE_RAIN defined(WIND_RAIN_PIN)

#define ENV_INCLUDE_WIND (ENV_INCLUDE_WIND_SPEED || ENV_INCLUDE_WIND_DIRECTION || ENV_INCLUDE_RAIN)
