#include "firmware_update.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "globals.h"
#include "localization.h"
#include "dialog.h"
#include "power.h"
#include "ui_main.h"
#include "util.h"

// How long the on-screen "update available" notice stays up before
// auto-closing (see showDialog()'s auto_close_ms).
#define FIRMWARE_UPDATE_DIALOG_MS 30000UL

// Strips any non-numeric prefix off APP_VERSION (e.g. "App ver. 2.1.0" ->
// "2.1.0"), mirroring webconfig.cpp's webconfig_current_version(), so it can
// be compared against the manifest's bare "X.Y.Z" with version_is_newer().
static String firmware_current_version() {
  String v(APP_VERSION);
  size_t i = 0;
  while (i < v.length() && !isDigit(v[i])) i++;
  return v.substring(i);
}

void check_for_firmware_update(lv_timer_t *timer) {
  if (WiFi.status() != WL_CONNECTED) return;

  bool available = false;
  HTTPClient http;
  http.begin(FIRMWARE_MANIFEST_URL);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      String latest = doc["version"] | "";
      if (latest.length() > 0) {
        available = version_is_newer(latest, firmware_current_version());
      }
    }
  }
  http.end();

  // Also covers the icon disappearing again once this device catches up
  // (e.g. right after an OTA update, on the next 6h/boot check).
  firmware_update_available = available;
  update_firmware_update_icon();

  if (!available) return;

  if (screen_dimmed) {
    restore_light();
    screen_dimmed = false;
    last_touch_ms = millis();
  }

  const LocalizedStrings *strings = get_strings();
  showDialog(strings->firmware_update_available, FIRMWARE_UPDATE_DIALOG_MS);
}
