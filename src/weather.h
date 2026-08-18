#pragma once

#include <lvgl.h>
#include "localization.h"

// Weather provider selection
enum WeatherProvider { PROVIDER_OPEN_METEO = 0, PROVIDER_OPENWEATHER = 1 };
extern WeatherProvider weather_provider;
extern char openweather_apikey[40];

// Dispatches on weather_provider; polled on boot and every UPDATE_INTERVAL.
void fetch_and_update_weather();
void fetch_weather_openmeteo();
void fetch_weather_openweather();
void fetch_openweather_uv_and_air_quality();
void fetch_openmeteo_air_quality();

// Maps OpenWeather condition IDs to the WMO weather codes choose_icon()/
// choose_image() switch on, so both providers can drive the same
// icon-selection logic.
int translate_owm_code_to_wmo(int owm_id);

// Weather-code -> asset mapping (WMO codes), day/night variants.
const lv_img_dsc_t *choose_image(int wmo_code, int is_day);
const lv_img_dsc_t *choose_icon(int wmo_code, int is_day);

// UV index level per WHO's standard bands (0-2 Low ... 11+ Extreme).
inline const char* uv_level_text(float uvi, const LocalizedStrings* strings) {
  if (uvi < 3) return strings->uv_low;
  if (uvi < 6) return strings->uv_moderate;
  if (uvi < 8) return strings->uv_high;
  if (uvi < 11) return strings->uv_very_high;
  return strings->uv_extreme;
}

// OpenWeather's own Air Quality Index: 1 (Good) .. 5 (Very Poor).
inline const char* aqi_level_text(int aqi, const LocalizedStrings* strings) {
  switch (aqi) {
    case 1: return strings->aqi_good;
    case 2: return strings->aqi_fair;
    case 3: return strings->aqi_moderate;
    case 4: return strings->aqi_poor;
    case 5: return strings->aqi_very_poor;
    default: return "--";
  }
}

// Open-Meteo's European Air Quality Index: 0-20 Good ... 100+ Very Poor
// (its own top two bands, Poor/Very Poor, both collapse into our "very poor").
inline const char* eaqi_level_text(int aqi, const LocalizedStrings* strings) {
  if (aqi <= 20) return strings->aqi_good;
  if (aqi <= 40) return strings->aqi_fair;
  if (aqi <= 60) return strings->aqi_moderate;
  if (aqi <= 80) return strings->aqi_poor;
  return strings->aqi_very_poor;
}
