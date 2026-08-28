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

// Aggregated per-day stats built from the 3-hourly /data/2.5/forecast list,
// since the free OpenWeather tier has no daily endpoint.
struct OwmDailyAgg {
  String dateKey;   // "YYYY-MM-DD"
  float tmin = 1e9f;
  float tmax = -1e9f;
  int code = 0;
  int codeHourDist = 99;  // distance from noon of the hour `code` was sampled at; smaller wins
};

// OpenWeather free tier: current weather (/data/2.5/weather) + 5 day / 3 hour
// forecast (/data/2.5/forecast), both usable with a plain free API key (unlike
// One Call 3.0, which requires a separate paid subscription). Daily values are
// aggregated from the 3-hourly buckets; "hourly" slots show 3-hour steps.
void fetch_weather_openweather() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (strlen(openweather_apikey) == 0) {
    Serial.println("fetch_weather_openweather: no API key configured");
    return;
  }

  const LocalizedStrings* strings = get_strings();
  char unit = use_fahrenheit ? 'F' : 'C';

  // === Current conditions ===
  String current_url = String("https://api.openweathermap.org/data/2.5/weather?lat=")
               + latitude + "&lon=" + longitude
               + "&units=metric&appid=" + openweather_apikey;

  HTTPClient http_cur;
  http_cur.begin(current_url);
  int cur_code = http_cur.GET();

  time_t cur_sunrise = 0, cur_sunset = 0;
  bool current_ok = false;

  if (cur_code == HTTP_CODE_OK) {
    String payload = http_cur.getString();
    DynamicJsonDocument doc(4 * 1024);

    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      current_ok = true;

      ntp_utc_offset_seconds = doc["timezone"].as<int>();
      configTime(ntp_utc_offset_seconds, 0, ntp_server, "time.nist.gov");
      Serial.print("Updating time from NTP with UTC offset: ");
      Serial.println(ntp_utc_offset_seconds);

      float t_now = doc["main"]["temp"].as<float>();
      float t_ap = doc["main"]["feels_like"].as<float>();
      float humidity = doc["main"]["humidity"].as<float>();
      int code_now = translate_owm_code_to_wmo(doc["weather"][0]["id"].as<int>());
      time_t cur_dt = doc["dt"].as<time_t>();
      cur_sunrise = doc["sys"]["sunrise"].as<time_t>();
      cur_sunset = doc["sys"]["sunset"].as<time_t>();
      int is_day = (cur_dt >= cur_sunrise && cur_dt < cur_sunset) ? 1 : 0;

      if (use_fahrenheit) {
        t_now = t_now * 9.0 / 5.0 + 32.0;
        t_ap = t_ap * 9.0 / 5.0 + 32.0;
      }

      lv_label_set_text_fmt(lbl_today_temp, "%.0f°%c", t_now, unit);
      lv_label_set_text_fmt(lbl_today_feels_like, "%s %.0f°%c", strings->feels_like_temp, t_ap, unit);
      lv_img_set_src(img_today_icon, choose_image(code_now, is_day));
      lv_label_set_text_fmt(lbl_relative_hum, "%s %.0f% %", strings->hum, humidity);

      time_t sunrise_local = cur_sunrise + ntp_utc_offset_seconds + 86400;  // approx. tomorrow's sunrise
      time_t sunset_local = cur_sunset + ntp_utc_offset_seconds;
      struct tm *sr = gmtime(&sunrise_local);
      int sunrise_hour = sr->tm_hour, sunrise_minute = sr->tm_min;
      struct tm *ss = gmtime(&sunset_local);
      int sunset_hour = ss->tm_hour, sunset_minute = ss->tm_min;
      lv_label_set_text_fmt(lbl_sunrise, "%02d:%02d", sunrise_hour, sunrise_minute);
      lv_label_set_text_fmt(lbl_sunset, "%02d:%02d", sunset_hour, sunset_minute);
    } else {
      Serial.println("fetch_weather_openweather: current JSON parse failed");
    }
  } else {
    Serial.printf("fetch_weather_openweather: current fetch failed, code=%d (%s)\n",
                   cur_code, http_cur.errorToString(cur_code).c_str());
    if (cur_code > 0) Serial.println("response body: " + http_cur.getString());
  }
  http_cur.end();

  if (!current_ok) return;  // no point continuing without a UTC offset / sunrise-sunset baseline

  // === 5 day / 3 hour forecast (daily + hourly boxes) ===
  String forecast_url = String("https://api.openweathermap.org/data/2.5/forecast?lat=")
               + latitude + "&lon=" + longitude
               + "&units=metric&appid=" + openweather_apikey;

  HTTPClient http_fc;
  http_fc.begin(forecast_url);
  int fc_code = http_fc.GET();

  if (fc_code == HTTP_CODE_OK) {
    Serial.println("Updated weather from OpenWeather");

    // Parse straight off the network stream instead of buffering the whole
    // (~15-20KB) body into a String first — cheaper on ESP32's heap and
    // sidesteps HTTPClient::getString() coming back empty on large bodies.
    // Also filter down to only the fields we use: the full response (40
    // entries x ~20 fields incl. coord/wind/clouds/visibility) blows well
    // past what a modest DynamicJsonDocument can hold.
    StaticJsonDocument<256> filter;
    JsonObject filter_item = filter["list"][0].to<JsonObject>();
    filter_item["dt_txt"] = true;
    filter_item["main"]["temp"] = true;
    filter_item["main"]["temp_min"] = true;
    filter_item["main"]["temp_max"] = true;
    filter_item["weather"][0]["id"] = true;
    filter_item["pop"] = true;
    filter_item["sys"]["pod"] = true;

    DynamicJsonDocument doc(16 * 1024);
    DeserializationError err = deserializeJson(doc, http_fc.getStream(), DeserializationOption::Filter(filter));

    if (err == DeserializationError::Ok) {
      JsonArray list = doc["list"].as<JsonArray>();

      // --- Hourly (3-hour step) box: first 7 entries as-is ---
      for (int i = 0; i < 7 && i < (int)list.size(); i++) {
        JsonObject h = list[i];
        // dt_txt is UTC ("YYYY-MM-DD HH:MM:SS"); shift by the offset to get local hour.
        const char *dt_txt = h["dt_txt"] | "1970-01-01 00:00:00";
        int utc_minutes = atoi(dt_txt + 11) * 60 + atoi(dt_txt + 14);
        int local_minutes = ((utc_minutes + ntp_utc_offset_seconds / 60) % 1440 + 1440) % 1440;
        String hour_name = hour_of_day(local_minutes / 60);

        float precipitation_probability = h["pop"].as<float>() * 100.0;
        float temp = h["main"]["temp"].as<float>();
        if (use_fahrenheit) temp = temp * 9.0 / 5.0 + 32.0;
        int wmo_code = translate_owm_code_to_wmo(h["weather"][0]["id"].as<int>());
        const char *pod = h["sys"]["pod"] | "d";
        int h_is_day = (pod[0] == 'd') ? 1 : 0;

        if (i == 0 && current_language != LANG_FR) {
          lv_label_set_text(lbl_hourly[i], strings->now);
        } else {
          lv_label_set_text(lbl_hourly[i], hour_name.c_str());
        }
        lv_label_set_text_fmt(lbl_precipitation_probability[i], "%.0f%%", precipitation_probability);
        lv_label_set_text_fmt(lbl_hourly_temp[i], "%.0f°%c", temp, unit);
        lv_img_set_src(img_hourly[i], choose_icon(wmo_code, h_is_day));

        // Bottom-of-page "next 4 hours" strip
        if (i < 4) {
          lv_label_set_text(lbl_next_hours_time[i], hour_name.c_str());
          lv_img_set_src(img_next_hours[i], choose_icon(wmo_code, h_is_day));
        }
      }

      // --- Daily box: aggregate 3-hourly buckets by calendar date ---
      OwmDailyAgg agg[7];
      int dayCount = 0;
      for (JsonObject h : list) {
        const char *dt_txt = h["dt_txt"] | "";
        if (strlen(dt_txt) < 13) continue;
        String dateKey = String(dt_txt).substring(0, 10);
        int hour = atoi(dt_txt + 11);

        int idx = -1;
        for (int j = 0; j < dayCount; j++) {
          if (agg[j].dateKey == dateKey) { idx = j; break; }
        }
        if (idx < 0) {
          if (dayCount >= 7) continue;
          idx = dayCount++;
          agg[idx].dateKey = dateKey;
        }

        float mn = h["main"]["temp_min"].as<float>();
        float mx = h["main"]["temp_max"].as<float>();
        if (mn < agg[idx].tmin) agg[idx].tmin = mn;
        if (mx > agg[idx].tmax) agg[idx].tmax = mx;

        int hourDist = abs(hour - 12);
        if (hourDist < agg[idx].codeHourDist) {
          agg[idx].codeHourDist = hourDist;
          agg[idx].code = translate_owm_code_to_wmo(h["weather"][0]["id"].as<int>());
        }
      }

      // First page always shows a 5-day forecast; show just 5 daily-box
      // rows and hide the other two (OpenWeather's free tier only covers 5
      // days anyway; Open-Meteo fetches 7 but also caps display at 5).
      const int OWM_DAILY_ROWS = 5;
      for (int i = 0; i < OWM_DAILY_ROWS && i < dayCount; i++) {
        int y, mo, d;
        sscanf(agg[i].dateKey.c_str(), "%d-%d-%d", &y, &mo, &d);
        int dow = day_of_week(y, mo, d);
        const char *dayStr = (i == 0 && current_language != LANG_FR) ? strings->today : strings->weekdays[dow];

        float mn = agg[i].tmin;
        float mx = agg[i].tmax;
        if (use_fahrenheit) {
          mn = mn * 9.0 / 5.0 + 32.0;
          mx = mx * 9.0 / 5.0 + 32.0;
        }
        lv_label_set_text_fmt(lbl_daily_day[i], "%s", dayStr);
        lv_label_set_text_fmt(lbl_daily_high[i], "%.0f°%c", mx, unit);
        lv_label_set_text_fmt(lbl_daily_low[i], "%.0f°%c", mn, unit);
        lv_img_set_src(img_daily[i], choose_icon(agg[i].code, 1));
      }
      for (int i = OWM_DAILY_ROWS; i < 7; i++) {
        lv_obj_add_flag(lbl_daily_day[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lbl_daily_high[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lbl_daily_low[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(img_daily[i], LV_OBJ_FLAG_HIDDEN);
      }
    } else {
      Serial.printf("fetch_weather_openweather: forecast JSON parse failed: %s\n", err.c_str());
    }
  } else {
    Serial.printf("fetch_weather_openweather: forecast fetch failed, code=%d (%s)\n",
                   fc_code, http_fc.errorToString(fc_code).c_str());
    if (fc_code > 0) Serial.println("response body: " + http_fc.getString());
  }
  http_fc.end();
}

// UV index (/data/2.5/uvi) + air quality (/data/2.5/air_pollution), both
// OpenWeather-specific and usable with a plain free API key. Feeds the
// UV/Air Quality box shown after the moon-phases box.
void fetch_openweather_uv_and_air_quality() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (strlen(openweather_apikey) == 0) return;

  const LocalizedStrings* strings = get_strings();

  // === UV Index ===
  String uv_url = String("https://api.openweathermap.org/data/2.5/uvi?lat=")
               + latitude + "&lon=" + longitude + "&appid=" + openweather_apikey;

  HTTPClient http_uv;
  http_uv.begin(uv_url);
  int uv_code = http_uv.GET();

  if (uv_code == HTTP_CODE_OK) {
    StaticJsonDocument<192> doc;
    if (deserializeJson(doc, http_uv.getStream()) == DeserializationError::Ok) {
      float uvi = doc["value"].as<float>();
      lv_label_set_text_fmt(lbl_uv_value, "%.1f", uvi);
      lv_label_set_text(lbl_uv_desc, uv_level_text(uvi, strings));
    } else {
      Serial.println("fetch_openweather_uv_and_air_quality: UV JSON parse failed");
    }
  } else {
    Serial.printf("fetch_openweather_uv_and_air_quality: UV fetch failed, code=%d (%s)\n",
                   uv_code, http_uv.errorToString(uv_code).c_str());
    lv_label_set_text(lbl_uv_value, "--");
    lv_label_set_text(lbl_uv_desc, "");
  }
  http_uv.end();

  // === Air Quality ===
  String aqi_url = String("https://api.openweathermap.org/data/2.5/air_pollution?lat=")
               + latitude + "&lon=" + longitude + "&appid=" + openweather_apikey;

  HTTPClient http_aqi;
  http_aqi.begin(aqi_url);
  int aqi_code = http_aqi.GET();

  if (aqi_code == HTTP_CODE_OK) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, http_aqi.getStream()) == DeserializationError::Ok) {
      int aqi = doc["list"][0]["main"]["aqi"].as<int>();
      lv_label_set_text_fmt(lbl_aqi_value, "%d", aqi);
      lv_label_set_text(lbl_aqi_desc, aqi_level_text(aqi, strings));
    } else {
      Serial.println("fetch_openweather_uv_and_air_quality: air quality JSON parse failed");
    }
  } else {
    Serial.printf("fetch_openweather_uv_and_air_quality: air quality fetch failed, code=%d (%s)\n",
                   aqi_code, http_aqi.errorToString(aqi_code).c_str());
    lv_label_set_text(lbl_aqi_value, "--");
    lv_label_set_text(lbl_aqi_desc, "");
  }
  http_aqi.end();
}
