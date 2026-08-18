#pragma once

#include <lvgl.h>

LV_FONT_DECLARE(lv_font_montserrat_latin_12);
LV_FONT_DECLARE(lv_font_montserrat_latin_14);
LV_FONT_DECLARE(lv_font_montserrat_latin_16);
LV_FONT_DECLARE(lv_font_montserrat_latin_20);
LV_FONT_DECLARE(lv_font_montserrat_latin_42);

// Font selection based on language
inline const lv_font_t* get_font_12() {
  return &lv_font_montserrat_latin_12;
}

inline const lv_font_t* get_font_14() {
  return &lv_font_montserrat_latin_14;
}

inline const lv_font_t* get_font_16() {
  return &lv_font_montserrat_latin_16;
}

inline const lv_font_t* get_font_20() {
  return &lv_font_montserrat_latin_20;
}

inline const lv_font_t* get_font_42() {
  return &lv_font_montserrat_latin_42;
}
