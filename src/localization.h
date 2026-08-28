#pragma once

// Language support
enum Language { LANG_EN = 0, LANG_ES = 1, LANG_DE = 2, LANG_FR = 3, LANG_IT = 4 };
extern Language current_language;

struct LocalizedStrings {
  const char *temp_placeholder;
  const char *feels_like_temp;
  const char *seven_day_forecast;
  const char *five_day_forecast;
  const char *hourly_forecast;
  const char *today;
  const char *now;
  const char *am;
  const char *pm;
  const char *noon;
  const char *invalid_hour;
  const char *brightness;
  const char *location;
  const char *use_fahrenheit;
  const char *use_24hr;
  const char *save;
  const char *cancel;
  const char *close;
  const char *location_btn;
  const char *reset_wifi;
  const char *reset;
  const char *change_location;
  const char *aura_settings;
  const char *city;
  const char *search_results;
  const char *city_placeholder;
  const char *wifi_config;
  const char *reset_confirmation;
  const char *language_label;
  const char *weekdays[7];
  const char *moonphases;
  const char *days_to_christmas_text;
  const char *merry_christmas_text;
  const char *happy_new_year_text;
  //Moonphases
  const char *new_moon;
  const char *wax_crescent;
  const char *first_quarter;
  const char *wax_gibbous;
  const char *full_moon;
  const char *wan_gibbous;
  const char *last_quarter;
  const char *wan_crescent;
  const char *hum;
  const char *air_quality_page;
  const char *uv_index_label;
  const char *air_quality_label;
  const char *uv_low;
  const char *uv_moderate;
  const char *uv_high;
  const char *uv_very_high;
  const char *uv_extreme;
  const char *aqi_good;
  const char *aqi_fair;
  const char *aqi_moderate;
  const char *aqi_poor;
  const char *aqi_very_poor;
  const char *firmware_update_available;
};

const LocalizedStrings* get_strings();
