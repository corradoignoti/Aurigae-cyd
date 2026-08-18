#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>
#include <WebServer.h>
#include "esp_system.h"

#include "config.h"
#include "globals.h"
#include "localization.h"
#include "fonts.h"
#include "weather.h"
#include "moonphase.h"
#include "christmas.h"
#include "webconfig.h"
#include "theme.h"
#include "power.h"
#include "wifi_provisioning.h"
#include "ui_main.h"

static void update_clock(lv_timer_t *timer) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  const LocalizedStrings* strings = get_strings();
  char buf[16];
  if (use_24_hour) {
    snprintf(buf, sizeof(buf), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  } else {
    int hour = timeinfo.tm_hour % 12;
    if(hour == 0) hour = 12;
    const char *ampm = (timeinfo.tm_hour < 12) ? strings->am : strings->pm;
    snprintf(buf, sizeof(buf), "%d:%02d%s", hour, timeinfo.tm_min, ampm);
  }
  lv_label_set_text(lbl_clock, buf);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  TFT_eSPI tft = TFT_eSPI();
  tft.init();
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);

  lv_init();

  // Init touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(0);

  lv_display_t *disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  // Load saved prefs
  prefs.begin("weather", false);
  String lat = prefs.getString("latitude", LATITUDE_DEFAULT);
  lat.toCharArray(latitude, sizeof(latitude));
  String lon = prefs.getString("longitude", LONGITUDE_DEFAULT);
  lon.toCharArray(longitude, sizeof(longitude));
  use_fahrenheit = prefs.getBool("useFahrenheit", false);
  location = prefs.getString("location", LOCATION_DEFAULT);
  uint32_t brightness = prefs.getUInt("brightness", 255);
  default_lcd_brightness = brightness; //used as global var to restore LCD brightness when anitburn set it to LOW
  use_24_hour = prefs.getBool("use24Hour", false);
  current_language = (Language)prefs.getUInt("language", LANG_EN);
  String ntp = prefs.getString("ntpServer", NTP_SERVER_DEFAULT);
  ntp.toCharArray(ntp_server, sizeof(ntp_server));
  weather_provider = (WeatherProvider)prefs.getUInt("weatherProvider", PROVIDER_OPEN_METEO);
  current_theme = (UiTheme)prefs.getUInt("theme", THEME_BLUE);
  String owmKey = prefs.getString("owmApiKey", "");
  owmKey.toCharArray(openweather_apikey, sizeof(openweather_apikey));
  analogWrite(LCD_BACKLIGHT_PIN, brightness); //Set brightness

  // Check for Wi-Fi config and request it if not available
  WiFiManager wm;
  wm.setAPCallback(apModeCallback);
  wm.autoConnect(DEFAULT_CAPTIVE_SSID);

  // Start webconfig server (NTP server, location)
  start_webconfig_server();

  //SET TIMERS HERE

  lv_timer_create(update_clock, 1000, NULL);

  // Poll for idle every 30s, dim backlight once ANTIBURN_IDLE_TIMEOUT elapses without a touch
  last_touch_ms = millis();
  lv_timer_create(antiburn, ANTIBURN_CHECK_INTERVAL, NULL);

  // set a 30 min. timer to prevent deep sleep
  lv_timer_create(force_wakeup, 1800000, NULL);

  lv_obj_clean(lv_scr_act());
  create_ui();
  fetch_and_update_weather();
  fetch_and_update_moonphase();
  update_days_to_christmas();
}

void loop() {
  lv_timer_handler();
  static uint32_t last = millis();

  webConfigServer.handleClient();
  update_wifi_status_icon();

  if (millis() - last >= UPDATE_INTERVAL) {
    fetch_and_update_weather();
    fetch_and_update_moonphase();
    update_days_to_christmas();
    last = millis();
  }

  lv_tick_inc(5);
  delay(5);
}
