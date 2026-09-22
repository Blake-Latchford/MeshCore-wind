#include "SensorMesh.h"
#include <helpers/sensors/LPPDataHelpers.h>
#include <helpers/wind/ArgentWindSpeed.h>
#include <helpers/wind/ArgentWindDirection.h>
#include <helpers/wind/ArgentRain.h>

#ifdef DISPLAY_CLASS
  #include "UITask.h"
  static UITask ui_task(display);
#endif

class MyMesh : public SensorMesh {
public:
  MyMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms, mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
     : SensorMesh(board, radio, ms, rng, rtc, tables),
       battery_data(12*24, 5*60)    // 24 hours worth of battery data, every 5 minutes
#if ENV_INCLUDE_WIND_SPEED
// For Review: Is the extra 288 bytes per data type relevant?
       , wind_speed_data(12*24, 5*60)
       , wind_gust_data(12*24, 5*60)
#endif
  {
  }

protected:
  /* ========================== custom logic here ========================== */
  Trigger low_batt, critical_batt;
  TimeSeriesData  battery_data;
#if ENV_INCLUDE_WIND_SPEED
  TimeSeriesData  wind_speed_data;
  TimeSeriesData  wind_gust_data;
#endif

  void onSensorDataRead() override {
    float batt_voltage = getVoltage(TELEM_CHANNEL_SELF);

    battery_data.recordData(getRTCClock(), batt_voltage);   // record battery
    alertIf(batt_voltage < 3.4f, critical_batt, HIGH_PRI_ALERT, "Battery is critical!");
    alertIf(batt_voltage < 3.6f, low_batt, LOW_PRI_ALERT, "Battery is low");

#if ENV_INCLUDE_WIND_SPEED
    // For review: Previously there was only one timestamp recorded for battery.
    // Now all 3 recordings have different timestamps. Is that a problem?
    wind_speed_data.recordData(getRTCClock(), ArgentWindSpeed::readSustained() / (float)LPP_WIND_SPEED_MULT);
    wind_gust_data.recordData(getRTCClock(), ArgentWindSpeed::readGust() / (float)LPP_WIND_GUST_MULT);
#endif
  }

  int querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[], int max_num) override {
    int n = 0;
    battery_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[n++], TELEM_CHANNEL_SELF, LPP_VOLTAGE);
#if ENV_INCLUDE_WIND_SPEED
    uint8_t wind_ch = sensors.getNextAvailableChannel();
    wind_speed_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[n++], wind_ch, LPP_WIND_SPEED);
    wind_gust_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[n++], wind_ch, LPP_WIND_GUST);
#endif
    return n;
  }

  bool handleCustomCommand(uint32_t sender_timestamp, char* command, char* reply) override {
    if (strcmp(command, "magic") == 0) {    // example 'custom' command handling
      strcpy(reply, "**Magic now done**");
      return true;   // handled
    }
    if (strcmp(command, "wind") == 0) {    // dump live wind/rain readings, for bench testing
      char* p = reply;
#if ENV_INCLUDE_WIND_SPEED
      p += sprintf(p, "speed=%u gust=%u ", ArgentWindSpeed::readSustained(), ArgentWindSpeed::readGust());
#endif
#if ENV_INCLUDE_WIND_DIRECTION
      p += sprintf(p, "dir=%u ", ArgentWindDirection::read());
#endif
#if ENV_INCLUDE_RAIN
      p += sprintf(p, "rain=%u ", ArgentRain::read());
#endif
      if (p == reply) strcpy(reply, "(no wind sensors configured)");
      return true;   // handled
    }
    return false;  // not handled
  }
  /* ======================================================================= */
};

StdRNG fast_rng;
SimpleMeshTables tables;

MyMesh the_mesh(board, radio_driver, *new ArduinoMillis(), fast_rng, rtc_clock, tables);

void halt() {
  while (1) ;
}

static char command[160];

void setup() {
  Serial.begin(115200);
  delay(1000);

  board.begin();

#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.begin();
#endif

#ifdef DISPLAY_CLASS
  if (display.begin()) {
    display.startFrame();
    display.print("Please wait...");
    display.endFrame();
  }
#endif

  if (!radio_init()) { halt(); }

  fast_rng.begin(radio_driver.getRngSeed());

  FILESYSTEM* fs;
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  fs = &InternalFS;
  IdentityStore store(InternalFS, "");
#elif defined(ESP32)
  SPIFFS.begin(true);
  fs = &SPIFFS;
  IdentityStore store(SPIFFS, "/identity");
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  fs = &LittleFS;
  IdentityStore store(LittleFS, "/identity");
  store.begin();
#else
  #error "need to define filesystem"
#endif
  if (!store.load("_main", the_mesh.self_id)) {
    MESH_DEBUG_PRINTLN("Generating new keypair");
    the_mesh.self_id = radio_new_identity();   // create new random identity
    int count = 0;
    while (count < 10 && (the_mesh.self_id.pub_key[0] == 0x00 || the_mesh.self_id.pub_key[0] == 0xFF)) {  // reserved id hashes
      the_mesh.self_id = radio_new_identity(); count++;
    }
    store.save("_main", the_mesh.self_id);
  }

  Serial.print("Sensor ID: ");
  mesh::Utils::printHex(Serial, the_mesh.self_id.pub_key, PUB_KEY_SIZE); Serial.println();

  command[0] = 0;

  sensors.begin();
#if ENV_INCLUDE_WIND_SPEED
  ArgentWindSpeed::begin();
#endif
#if ENV_INCLUDE_WIND_DIRECTION
  ArgentWindDirection::begin();
#endif
#if ENV_INCLUDE_RAIN
  ArgentRain::begin();
#endif

  the_mesh.begin(fs);

#ifdef DISPLAY_CLASS
  ui_task.begin(the_mesh.getNodePrefs(), FIRMWARE_BUILD_DATE, FIRMWARE_VERSION);
#endif

  // send out initial zero hop Advertisement to the mesh
#if ENABLE_ADVERT_ON_BOOT == 1
  the_mesh.sendSelfAdvertisement(16000, false);
#endif
}

void loop() {
  int len = strlen(command);
  // `command` must stay NUL-terminated within its bounds. If it ever isn't,
  // strlen() above can return >= sizeof(command) and the loop below would then
  // index past the buffer, so clamp defensively.
  if (len >= (int)sizeof(command)) {
    command[0] = 0;
    len = 0;
  }
  while (Serial.available() && len < sizeof(command)-1) {
    char c = Serial.read();
    if (c != '\n') {
      command[len++] = c;
      command[len] = 0;
    }
    Serial.print(c);
  }
  if (len == sizeof(command)-1) {  // buffer full: treat as a completed line
    command[sizeof(command)-2] = '\r';  // place end-of-line marker inside the buffer
    command[sizeof(command)-1] = 0;     // keep the buffer NUL-terminated
  }

  if (len > 0 && command[len - 1] == '\r') {  // received complete line
    command[len - 1] = 0;  // replace newline with C string null terminator
    char reply[160];
    the_mesh.handleCommand(NULL, 0, command, reply);  // NOTE: there is no sender_timestamp via serial!
    if (reply[0]) {
      Serial.print("  -> "); Serial.println(reply);
    }

    command[0] = 0;  // reset command buffer
  }

  board.loop();   // let the board feed its watchdog, run periodic housekeeping

  the_mesh.loop();
  sensors.loop();
#if ENV_INCLUDE_WIND_SPEED
  ArgentWindSpeed::loop();
#endif
#if ENV_INCLUDE_WIND_DIRECTION
  ArgentWindDirection::loop();
#endif
#ifdef DISPLAY_CLASS
  ui_task.loop();
#endif
  rtc_clock.tick();
#ifdef HAS_EXTERNAL_WATCHDOG
  external_watchdog.loop();
#endif
}
