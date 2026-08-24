#pragma once

// Shared runtime state used across translation units: hardware handles,
// persisted config, network/geocoding scratch state, and the LVGL widget
// pointers that more than one module reads or writes. Widgets touched by a
// single .cpp only are kept as static file-local variables there instead.
#include <lvgl.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "weather.h"  // WeatherProvider

// The TFT_eSPI instance created/init'd in setup(). LVGL drives its own
// separate internal TFT_eSPI instance for drawing (see lv_tft_espi_create());
// this one talks to the same physical SPI bus/display and is used for
// operations LVGL's driver doesn't expose, e.g. reading pixels back for
// webconfig's /screenshot endpoint.
extern TFT_eSPI tft;

extern SPIClass touchscreenSPI;
extern XPT2046_Touchscreen touchscreen;
extern uint32_t draw_buf[DRAW_BUF_SIZE / 4];
extern int x, y, z;

// Preferences
extern Preferences prefs;
extern bool use_fahrenheit;
extern bool use_24_hour;
extern char latitude[16];
extern char longitude[16];
extern String location;
extern DynamicJsonDocument geoDoc;
extern JsonArray geoResults;
extern char ntp_server[64];
extern int ntp_utc_offset_seconds;

// Web config server (NTP server, location)
extern WebServer webConfigServer;

// UI components shared across more than one .cpp
extern lv_obj_t *lbl_today_temp;
extern lv_obj_t *lbl_today_feels_like;
extern lv_obj_t *img_today_icon;
extern lv_obj_t *lbl_uv_value;
extern lv_obj_t *lbl_uv_desc;
extern lv_obj_t *lbl_aqi_value;
extern lv_obj_t *lbl_aqi_desc;
extern lv_obj_t *lbl_daily_day[7];
extern lv_obj_t *lbl_daily_high[7];
extern lv_obj_t *lbl_daily_low[7];
extern lv_obj_t *img_daily[7];
extern lv_obj_t *lbl_hourly[7];
extern lv_obj_t *lbl_precipitation_probability[7];
extern lv_obj_t *lbl_hourly_temp[7];
extern lv_obj_t *img_hourly[7];
extern lv_obj_t *lbl_loc;
extern lv_obj_t *kb;
extern lv_obj_t *lbl_sunrise;
extern lv_obj_t *lbl_sunset;
extern lv_obj_t *img_moon_phases;
extern lv_obj_t *lbl_moon_phase_desc;
extern lv_obj_t *img_santa_claus;
extern lv_obj_t *img_small_santa_claus;
extern lv_obj_t *lbl_santa_desc;
extern lv_obj_t *lbl_relative_hum;
extern lv_obj_t *lbl_merry_christmas;
extern lv_obj_t *lbl_clock;

extern int days_to_xmas;
extern uint32_t default_lcd_brightness;  // used to restore brightness when antiburn dimmed it to low
extern volatile unsigned long last_touch_ms;  // updated on every touch, drives antiburn idle check
extern volatile bool screen_dimmed;           // true while antiburn has blanked the backlight
