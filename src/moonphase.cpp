#include "moonphase.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "localization.h"
#include "icons.h"
#include "config.h"

void fetch_and_update_moonphase() {
  int phase_id = -1;

  if (WiFi.status() != WL_CONNECTED) return;

  String url = String (MOON_SERVICE_URL);

  HTTPClient http;
  http.begin(url);

  if (http.GET() == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok){
      //String date         = doc["date"].as<String>();
      //float  illumination = doc["illumination_percent"].as<float>();
      //String phase        = doc["phase"].as<String>();
      phase_id     = doc["phase_id"].as<int>();
    } else {
      Serial.println("Error parsing moonphase json");
    }

    const LocalizedStrings* strings = get_strings();

    switch (phase_id) {
      case -1:
        lv_label_set_text(lbl_moon_phase_desc, "- -");
        break;
      case 0:
        lv_label_set_text(lbl_moon_phase_desc, strings->new_moon);
        lv_img_set_src(img_moon_phases, &icon_0_new_moon);
        break;
      case 1:
        lv_label_set_text(lbl_moon_phase_desc, strings->wax_crescent);
        lv_img_set_src(img_moon_phases, &icon_1_waxing_crescent_moon);
        break;
      case 2:
        lv_label_set_text(lbl_moon_phase_desc, strings->first_quarter);
        lv_img_set_src(img_moon_phases, &icon_2_first_quarter_moon);
        break;
      case 3:
        lv_label_set_text(lbl_moon_phase_desc, strings->wax_gibbous);
        lv_img_set_src(img_moon_phases, &icon_3_waxing_gibbbous_moon);
        break;
      case 4:
        lv_label_set_text(lbl_moon_phase_desc, strings->full_moon);
        lv_img_set_src(img_moon_phases, &icon_4_full_moon);
        break;
      case 5:
        lv_label_set_text(lbl_moon_phase_desc, strings->wan_gibbous);
        lv_img_set_src(img_moon_phases, &icon_5_waning_gibbous_moon);
        break;
      case 6:
        lv_label_set_text(lbl_moon_phase_desc, strings->last_quarter);
        lv_img_set_src(img_moon_phases, &icon_6_last_quarter_moon);
        break;
      case 7:
        lv_label_set_text(lbl_moon_phase_desc, strings->wan_crescent);
        lv_img_set_src(img_moon_phases, &icon_7_waning_crescent);
        break;
      default:
        lv_label_set_text(lbl_moon_phase_desc, strings->new_moon);
        lv_img_set_src(img_moon_phases, &icon_0_new_moon);
        break;
    }
  }
}
