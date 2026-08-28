#include "weather.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "globals.h"
#include "localization.h"
#include "christmas.h"
#include "util.h"

void fetch_weather_openmeteo() {
  if (WiFi.status() != WL_CONNECTED) return;

  String url = String("https://api.open-meteo.com/v1/forecast?latitude=")
               + latitude + "&longitude=" + longitude
               + "&current=temperature_2m,apparent_temperature,is_day,weather_code"
               + "&daily=temperature_2m_min,temperature_2m_max,weather_code"
               + "&daily=sunrise,sunset"
               + "&hourly=temperature_2m,precipitation_probability,is_day,weather_code"
               + "&current=relative_humidity_2m"
               + "&current=uv_index"
               + "&forecast_hours=7"
               + "&timezone=auto";

  // Italian locations get ARPAE's higher-resolution ICON-2I regional model
  // added alongside the default best-match model for better local accuracy.
  if (strcmp(country_code, "IT") == 0) {
    url += "&models=italia_meteo_arpae_icon_2i";
  }

  HTTPClient http;
  http.begin(url);

  if (http.GET() == HTTP_CODE_OK) {
    Serial.println("Updated weather from open-meteo: " + url);

    String payload = http.getString();
    DynamicJsonDocument doc(64 * 1024);

    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      float t_now = doc["current"]["temperature_2m"].as<float>();
      float t_ap = doc["current"]["apparent_temperature"].as<float>();
      int code_now = doc["current"]["weather_code"].as<int>();
      int is_day = doc["current"]["is_day"].as<int>();
      float humidity = doc["current"]["relative_humidity_2m"].as<float>();
      float uvi = doc["current"]["uv_index"].as<float>();

      if (use_fahrenheit) {
        t_now = t_now * 9.0 / 5.0 + 32.0;
        t_ap = t_ap * 9.0 / 5.0 + 32.0;
      }
      const LocalizedStrings* strings = get_strings();

      ntp_utc_offset_seconds = doc["utc_offset_seconds"].as<int>();
      configTime(ntp_utc_offset_seconds, 0, ntp_server, "time.nist.gov");
      Serial.print("Updating time from NTP with UTC offset: ");
      Serial.println(ntp_utc_offset_seconds);

      char unit = use_fahrenheit ? 'F' : 'C';
      lv_label_set_text_fmt(lbl_today_temp, "%.0f°%c", t_now, unit);
      lv_label_set_text_fmt(lbl_today_feels_like, "%s %.0f°%c", strings->feels_like_temp, t_ap, unit);
      lv_img_set_src(img_today_icon, choose_image(code_now, is_day));
      lv_label_set_text_fmt(lbl_relative_hum, "%s %.0f% %", strings->hum, humidity);
      //lv_label_set_text_fmt(lbl_humidity, "%s %.0f%%", strings->humidity, humidity);
      lv_label_set_text_fmt(lbl_uv_value, "%.1f", uvi);
      lv_label_set_text(lbl_uv_desc, uv_level_text(uvi, strings));

      JsonArray times = doc["daily"]["time"].as<JsonArray>();
      JsonArray tmin = doc["daily"]["temperature_2m_min"].as<JsonArray>();
      JsonArray tmax = doc["daily"]["temperature_2m_max"].as<JsonArray>();
      JsonArray weather_codes = doc["daily"]["weather_code"].as<JsonArray>();
      JsonArray sunrise_times = doc["daily"]["sunrise"].as<JsonArray>();
      JsonArray sunset_times = doc["daily"]["sunset"].as<JsonArray>();

      // First page always shows a 5-day forecast, regardless of provider;
      // Open-Meteo fetches 7 days of data but only the first 5 rows are shown.
      for (int i = 0; i < 7; i++) {
        if (i < 5) {
          lv_obj_clear_flag(lbl_daily_day[i], LV_OBJ_FLAG_HIDDEN);
          lv_obj_clear_flag(lbl_daily_high[i], LV_OBJ_FLAG_HIDDEN);
          lv_obj_clear_flag(lbl_daily_low[i], LV_OBJ_FLAG_HIDDEN);
          lv_obj_clear_flag(img_daily[i], LV_OBJ_FLAG_HIDDEN);
        } else {
          lv_obj_add_flag(lbl_daily_day[i], LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_flag(lbl_daily_high[i], LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_flag(lbl_daily_low[i], LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_flag(img_daily[i], LV_OBJ_FLAG_HIDDEN);
        }
      }

      for (int i = 0; i < 7; i++) {
        const char *date = times[i];
        int year = atoi(date + 0);
        int mon = atoi(date + 5);
        int dayd = atoi(date + 8);
        int dow = day_of_week(year, mon, dayd);
        const char *dayStr = (i == 0 && current_language != LANG_FR) ? strings->today : strings->weekdays[dow];

        float mn = tmin[i].as<float>();
        float mx = tmax[i].as<float>();
        if (use_fahrenheit) {
          mn = mn * 9.0 / 5.0 + 32.0;
          mx = mx * 9.0 / 5.0 + 32.0;
        }
        lv_label_set_text_fmt(lbl_daily_day[i], "%s", dayStr);
        lv_label_set_text_fmt(lbl_daily_high[i], "%.0f°%c", mx, unit);
        lv_label_set_text_fmt(lbl_daily_low[i], "%.0f°%c", mn, unit);
        lv_img_set_src(img_daily[i], choose_icon(weather_codes[i].as<int>(), (i == 0) ? is_day : 1));
      }

      // === SUNRISE/SUNSET
       // Parsing sunrise-sunset
      const char *sunrise = sunrise_times[1].as<const char*>(); //tomorrow
      const char *sunset = sunset_times[0].as<const char*>(); //today

      int sunrise_hour = atoi(sunrise + 11);
      int sunrise_minute = atoi(sunrise + 14);
      int sunset_hour = atoi(sunset + 11);
      int sunset_minute = atoi(sunset + 14);

      lv_label_set_text_fmt(lbl_sunrise, "%02d:%02d", sunrise_hour, sunrise_minute);
      lv_label_set_text_fmt(lbl_sunset, "%02d:%02d", sunset_hour, sunset_minute);

      JsonArray hours = doc["hourly"]["time"].as<JsonArray>();
      JsonArray hourly_temps = doc["hourly"]["temperature_2m"].as<JsonArray>();
      JsonArray precipitation_probabilities = doc["hourly"]["precipitation_probability"].as<JsonArray>();
      JsonArray hourly_weather_codes = doc["hourly"]["weather_code"].as<JsonArray>();
      JsonArray hourly_is_day = doc["hourly"]["is_day"].as<JsonArray>();

      for (int i = 0; i < 7; i++) {
        const char *date = hours[i];  // "YYYY-MM-DD"
        int hour = atoi(date + 11);
        int minute = atoi(date + 14);
        String hour_name = hour_of_day(hour);

        float precipitation_probability = precipitation_probabilities[i].as<float>();
        float temp = hourly_temps[i].as<float>();
        if (use_fahrenheit) {
          temp = temp * 9.0 / 5.0 + 32.0;
        }

        if (i == 0 && current_language != LANG_FR) {
          lv_label_set_text(lbl_hourly[i], strings->now);
        } else {
          lv_label_set_text(lbl_hourly[i], hour_name.c_str());
        }
        lv_label_set_text_fmt(lbl_precipitation_probability[i], "%.0f%%", precipitation_probability);
        lv_label_set_text_fmt(lbl_hourly_temp[i], "%.0f°%c", temp, unit);
        lv_img_set_src(img_hourly[i], choose_icon(hourly_weather_codes[i].as<int>(), hourly_is_day[i].as<int>()));

        // Bottom-of-page "next 4 hours" strip
        if (i < 4) {
          lv_label_set_text(lbl_next_hours_time[i], hour_name.c_str());
          lv_img_set_src(img_next_hours[i], choose_icon(hourly_weather_codes[i].as<int>(), hourly_is_day[i].as<int>()));
        }
      }


    } else {
      Serial.println("fetch_and_update_weather: JSON parse failed\n");
    }
  } else {
    Serial.println("fetch_and_update_weather: HTTP GET failed");
  }
  http.end();
}

// Air quality (European AQI) from Open-Meteo's separate air-quality API. UV
// index for this provider is fetched alongside the rest in
// fetch_weather_openmeteo() since it's part of the same "current" bucket.
void fetch_openmeteo_air_quality() {
  if (WiFi.status() != WL_CONNECTED) return;

  const LocalizedStrings* strings = get_strings();

  String url = String("https://air-quality-api.open-meteo.com/v1/air-quality?latitude=")
               + latitude + "&longitude=" + longitude + "&current=european_aqi";

  HTTPClient http;
  http.begin(url);
  int code = http.GET();

  if (code == HTTP_CODE_OK) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err == DeserializationError::Ok && !doc["current"]["european_aqi"].isNull()) {
      int aqi = doc["current"]["european_aqi"].as<int>();
      lv_label_set_text_fmt(lbl_aqi_value, "%d", aqi);
      lv_label_set_text(lbl_aqi_desc, eaqi_level_text(aqi, strings));
    } else {
      Serial.printf("fetch_openmeteo_air_quality: JSON parse/field failed (%s): %s\n",
                     err.c_str(), payload.c_str());
      lv_label_set_text(lbl_aqi_value, "--");
      lv_label_set_text(lbl_aqi_desc, "");
    }
  } else {
    Serial.printf("fetch_openmeteo_air_quality: fetch failed, code=%d (%s)\n",
                   code, http.errorToString(code).c_str());
    lv_label_set_text(lbl_aqi_value, "--");
    lv_label_set_text(lbl_aqi_desc, "");
  }
  http.end();
}
