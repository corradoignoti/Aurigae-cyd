#include "ui_main.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <lvgl.h>
#include "globals.h"
#include "localization.h"
#include "fonts.h"
#include "icons.h"
#include "weather.h"
#include "christmas.h"
#include "ui_settings.h"
#include "theme.h"

// Widgets used only within this file (screen chrome + the three cycled
// forecast boxes + the UV/AQI box); everything else lives in globals.h.
static lv_obj_t *lbl_wifi_status;
static lv_obj_t *lbl_update_icon;
static lv_obj_t *lbl_forecast;
static lv_obj_t *box_daily;
static lv_obj_t *box_next_hours;
static lv_obj_t *box_hourly;
static lv_obj_t *box_moon_phases;
static lv_obj_t *box_air_quality;
static lv_obj_t *box_uv;
static lv_obj_t *box_aqi;
static lv_obj_t *img_sunset;
static lv_obj_t *img_sunrise;

// Page-indicator dots for the daily/hourly/moon-phase/air-quality cycle
// (same order as daily_cb/hourly_cb/moonp_cb/aqi_cb below).
#define NUM_CYCLE_PAGES 4
static lv_obj_t *page_dots[NUM_CYCLE_PAGES];
static int current_cycle_page = 0;

static void update_page_dots() {
  const ThemeColors &theme = get_theme_colors();
  for (int i = 0; i < NUM_CYCLE_PAGES; i++) {
    if (!page_dots[i]) continue;
    bool active = (i == current_cycle_page);
    lv_obj_set_size(page_dots[i], active ? 8 : 6, active ? 8 : 6);
    lv_obj_set_style_bg_color(page_dots[i], lv_color_hex(active ? theme.text_primary : theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_bg_opa(page_dots[i], active ? LV_OPA_COVER : LV_OPA_50, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  }
}

static void screen_event_cb(lv_event_t *e) {
  create_settings_window();
}

static void daily_cb(lv_event_t *e) {
  const LocalizedStrings* strings = get_strings();
  lv_obj_add_flag(box_daily, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(box_next_hours, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(lbl_forecast, strings->hourly_forecast);
  lv_obj_clear_flag(box_hourly, LV_OBJ_FLAG_HIDDEN);
  current_cycle_page = 1;
  update_page_dots();
}

static void hourly_cb(lv_event_t *e) {
  const LocalizedStrings* strings = get_strings();
  //lv_label_set_text(lbl_forecast, strings->seven_day_forecast);
  lv_obj_add_flag(box_hourly, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(lbl_forecast, strings->moonphases);
  lv_obj_clear_flag(box_moon_phases, LV_OBJ_FLAG_HIDDEN);
  current_cycle_page = 2;
  update_page_dots();
}

static void moonp_cb (lv_event_t *e) {
  const LocalizedStrings* strings = get_strings();
  lv_obj_add_flag(box_moon_phases, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(lbl_forecast, strings->air_quality_page);
  lv_obj_clear_flag(box_air_quality, LV_OBJ_FLAG_HIDDEN);
  current_cycle_page = 3;
  update_page_dots();
}

static void aqi_cb (lv_event_t *e) {
  const LocalizedStrings* strings = get_strings();
  lv_obj_add_flag(box_air_quality, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(lbl_forecast, strings->five_day_forecast);
  lv_obj_clear_flag(box_daily, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(box_next_hours, LV_OBJ_FLAG_HIDDEN);
  current_cycle_page = 0;
  update_page_dots();
}

// Fired every PAGE_SLIDESHOW_INTERVAL; when page_slideshow_enabled is on,
// advances the daily/hourly/moon-phase/air-quality cycle exactly as a tap on
// the currently-shown box would (the *_cb handlers ignore their event arg).
static void page_slideshow_cb(lv_timer_t *timer) {
  if (!page_slideshow_enabled) return;
  switch (current_cycle_page) {
    case 0: daily_cb(NULL); break;
    case 1: hourly_cb(NULL); break;
    case 2: moonp_cb(NULL); break;
    case 3: aqi_cb(NULL); break;
  }
}

// Show/hide the WiFi icon in the top-left corner based on connection status.
void update_wifi_status_icon() {
  if (!lbl_wifi_status) return;
  bool connected = (WiFi.status() == WL_CONNECTED);
  bool hidden = lv_obj_has_flag(lbl_wifi_status, LV_OBJ_FLAG_HIDDEN);
  if (connected && hidden) {
    lv_obj_clear_flag(lbl_wifi_status, LV_OBJ_FLAG_HIDDEN);
  } else if (!connected && !hidden) {
    lv_obj_add_flag(lbl_wifi_status, LV_OBJ_FLAG_HIDDEN);
  }
}

// Show/hide the download icon next to the Wi-Fi icon based on
// firmware_update_available; called by check_for_firmware_update().
void update_firmware_update_icon() {
  if (!lbl_update_icon) return;
  if (firmware_update_available) {
    lv_obj_clear_flag(lbl_update_icon, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(lbl_update_icon, LV_OBJ_FLAG_HIDDEN);
  }
}

void wifi_splash_screen() {
  const ThemeColors &theme = get_theme_colors();
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clean(scr);
  lv_obj_set_style_bg_color(scr, lv_color_hex(theme.bg_top), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_grad_color(scr, lv_color_hex(theme.bg_bottom), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  const LocalizedStrings* strings = get_strings();
  lv_obj_t *lbl = lv_label_create(scr);
  lv_label_set_text(lbl, strings->wifi_config);
  lv_obj_set_style_text_font(lbl, get_font_14(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(lbl);
  lv_scr_load(scr);
}

void create_ui() {
  const ThemeColors &theme = get_theme_colors();
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_hex(theme.bg_top), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_grad_color(scr, lv_color_hex(theme.bg_bottom), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  // Trigger settings screen on touch
  lv_obj_add_event_cb(scr, screen_event_cb, LV_EVENT_CLICKED, NULL);

  static lv_style_t default_label_style;
  lv_style_init(&default_label_style);
  lv_style_set_text_color(&default_label_style, lv_color_hex(theme.text_primary));
  lv_style_set_text_opa(&default_label_style, LV_OPA_COVER);

  lbl_wifi_status = lv_label_create(scr);
  lv_label_set_text(lbl_wifi_status, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_font(lbl_wifi_status, &lv_font_montserrat_14, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_wifi_status, lv_color_hex(theme.text_primary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align(lbl_wifi_status, LV_ALIGN_TOP_LEFT, 4, 4);
  lv_obj_add_flag(lbl_wifi_status, LV_OBJ_FLAG_HIDDEN);
  update_wifi_status_icon();

  lbl_update_icon = lv_label_create(scr);
  lv_label_set_text(lbl_update_icon, LV_SYMBOL_DOWNLOAD);
  lv_obj_set_style_text_font(lbl_update_icon, &lv_font_montserrat_14, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_update_icon, lv_color_hex(theme.text_primary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align_to(lbl_update_icon, lbl_wifi_status, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
  lv_obj_add_flag(lbl_update_icon, LV_OBJ_FLAG_HIDDEN);
  update_firmware_update_icon();

  img_today_icon = lv_img_create(scr);
  lv_img_set_src(img_today_icon, &image_partly_cloudy);
  lv_obj_align(img_today_icon, LV_ALIGN_TOP_MID, -64, 4);
  lv_img_set_zoom(img_today_icon, 198);

  lbl_relative_hum = lv_label_create(scr);
  lv_label_set_text(lbl_relative_hum, "Hum. 00 %");
  lv_obj_set_style_text_font(lbl_relative_hum, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_add_style(lbl_relative_hum, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align_to(lbl_relative_hum, img_today_icon, LV_ALIGN_OUT_BOTTOM_MID, 1, -13);


  const LocalizedStrings* strings = get_strings();

  lbl_today_temp = lv_label_create(scr);
  lv_label_set_text(lbl_today_temp, strings->temp_placeholder);
  lv_obj_set_style_text_font(lbl_today_temp, get_font_20(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align(lbl_today_temp, LV_ALIGN_TOP_MID, 45, 25);
  lv_obj_add_style(lbl_today_temp, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  // Small Santa near the temperature, shown/hidden by update_days_to_christmas()
  // once days_to_xmas is known (unknown/0 at this point, before NTP sync).
  img_small_santa_claus = lv_img_create(scr);
  lv_img_set_src(img_small_santa_claus, &icon_santa_claus);
  lv_img_set_zoom(img_small_santa_claus, 61);
  lv_obj_align_to(img_small_santa_claus, lbl_today_temp, LV_ALIGN_OUT_LEFT_MID, 40, -5);
  lv_obj_add_flag(img_small_santa_claus, LV_OBJ_FLAG_HIDDEN);

  lbl_today_feels_like = lv_label_create(scr);
  lv_label_set_text(lbl_today_feels_like, strings->feels_like_temp);
  lv_obj_set_style_text_font(lbl_today_feels_like, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_today_feels_like, lv_color_hex(theme.text_secondary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align(lbl_today_feels_like, LV_ALIGN_TOP_MID, 45, 48);

  lbl_forecast = lv_label_create(scr);
  lv_label_set_text(lbl_forecast, strings->five_day_forecast);
  lv_obj_set_style_text_font(lbl_forecast, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_forecast, lv_color_hex(theme.text_secondary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align(lbl_forecast, LV_ALIGN_TOP_LEFT, 20, 110);

  // Page-indicator dots, same row as lbl_forecast, right side.
  lv_obj_t *dots_row = lv_obj_create(scr);
  lv_obj_remove_style_all(dots_row);
  lv_obj_set_size(dots_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_clear_flag(dots_row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_layout(dots_row, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(dots_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(dots_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(dots_row, 5, LV_PART_MAIN);
  lv_obj_align(dots_row, LV_ALIGN_TOP_RIGHT, -12, 112);
  for (int i = 0; i < NUM_CYCLE_PAGES; i++) {
    page_dots[i] = lv_obj_create(dots_row);
    lv_obj_remove_style_all(page_dots[i]);
    lv_obj_set_style_radius(page_dots[i], LV_RADIUS_CIRCLE, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_bg_opa(page_dots[i], LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_clear_flag(page_dots[i], LV_OBJ_FLAG_SCROLLABLE);
  }
  current_cycle_page = 0;
  update_page_dots();

  box_daily = lv_obj_create(scr);
  lv_obj_set_size(box_daily, 220, 140);
  lv_obj_align(box_daily, LV_ALIGN_TOP_LEFT, 10, 135);
  lv_obj_set_style_bg_color(box_daily, lv_color_hex(theme.box_bg), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_daily, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_daily, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_daily, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_daily, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_daily, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_pad_all(box_daily, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(box_daily, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(box_daily, daily_cb, LV_EVENT_CLICKED, NULL);

  for (int i = 0; i < 7; i++) {
    lbl_daily_day[i] = lv_label_create(box_daily);
    lbl_daily_high[i] = lv_label_create(box_daily);
    lbl_daily_low[i] = lv_label_create(box_daily);
    img_daily[i] = lv_img_create(box_daily);

    lv_obj_add_style(lbl_daily_day[i], &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(lbl_daily_day[i], get_font_16(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_align(lbl_daily_day[i], LV_ALIGN_TOP_LEFT, 2, i * 24);

    lv_obj_add_style(lbl_daily_high[i], &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(lbl_daily_high[i], get_font_16(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_align(lbl_daily_high[i], LV_ALIGN_TOP_RIGHT, 0, i * 24);

    lv_label_set_text(lbl_daily_low[i], "");
    lv_obj_set_style_text_color(lbl_daily_low[i], lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(lbl_daily_low[i], get_font_16(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_align(lbl_daily_low[i], LV_ALIGN_TOP_RIGHT, -50, i * 24);

    lv_img_set_src(img_daily[i], &icon_partly_cloudy);
    lv_img_set_zoom(img_daily[i], 243);
    lv_obj_align(img_daily[i], LV_ALIGN_TOP_LEFT, 72, i * 24);
  }

  // === NEXT 4 HOURS STRIP (bottom of the daily-forecast page) ====
  // Shares box_daily's show/hide lifecycle (toggled in daily_cb/aqi_cb below)
  // since it's only meaningful alongside the daily forecast, not the other
  // cycled boxes (hourly/moon phases/air quality).
  box_next_hours = lv_obj_create(scr);
  lv_obj_set_size(box_next_hours, 220, 40);
  lv_obj_align(box_next_hours, LV_ALIGN_TOP_LEFT, 10, 278);
  lv_obj_set_style_bg_color(box_next_hours, lv_color_hex(theme.box_bg), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_next_hours, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_next_hours, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_next_hours, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_next_hours, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_next_hours, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_pad_all(box_next_hours, 4, LV_PART_MAIN);
  lv_obj_set_layout(box_next_hours, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(box_next_hours, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(box_next_hours, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  for (int i = 0; i < 4; i++) {
    lv_obj_t *col = lv_obj_create(box_next_hours);
    lv_obj_remove_style_all(col);
    lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(col, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    img_next_hours[i] = lv_img_create(col);
    lv_img_set_src(img_next_hours[i], &icon_partly_cloudy);
    lv_img_set_zoom(img_next_hours[i], 243);

    lbl_next_hours_time[i] = lv_label_create(col);
    lv_label_set_text(lbl_next_hours_time[i], "--:--");
    lv_obj_set_style_text_font(lbl_next_hours_time[i], get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_color(lbl_next_hours_time[i], lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  }

  box_hourly = lv_obj_create(scr);
  lv_obj_set_size(box_hourly, 220, 180);
  lv_obj_align(box_hourly, LV_ALIGN_TOP_LEFT, 10, 135);
  lv_obj_set_style_bg_color(box_hourly, lv_color_hex(theme.box_bg), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_hourly, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_hourly, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_hourly, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_hourly, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_hourly, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_pad_all(box_hourly, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(box_hourly, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(box_hourly, hourly_cb, LV_EVENT_CLICKED, NULL);

  for (int i = 0; i < 7; i++) {
    lbl_hourly[i] = lv_label_create(box_hourly);
    lbl_precipitation_probability[i] = lv_label_create(box_hourly);
    lbl_hourly_temp[i] = lv_label_create(box_hourly);
    img_hourly[i] = lv_img_create(box_hourly);

    lv_obj_add_style(lbl_hourly[i], &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(lbl_hourly[i], get_font_16(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_align(lbl_hourly[i], LV_ALIGN_TOP_LEFT, 2, i * 24);

    lv_obj_add_style(lbl_hourly_temp[i], &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(lbl_hourly_temp[i], get_font_16(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_align(lbl_hourly_temp[i], LV_ALIGN_TOP_RIGHT, 0, i * 24);

    lv_label_set_text(lbl_precipitation_probability[i], "");
    lv_obj_set_style_text_color(lbl_precipitation_probability[i], lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_set_style_text_font(lbl_precipitation_probability[i], get_font_16(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
    lv_obj_align(lbl_precipitation_probability[i], LV_ALIGN_TOP_RIGHT, -55, i * 24);

    lv_img_set_src(img_hourly[i], &icon_partly_cloudy);
    lv_img_set_zoom(img_hourly[i], 243);
    lv_obj_align(img_hourly[i], LV_ALIGN_TOP_LEFT, 72, i * 24);
  }

  lv_obj_add_flag(box_hourly, LV_OBJ_FLAG_HIDDEN);

  // === MOON PHASES BOX ====
  box_moon_phases = lv_obj_create(scr);
  lv_obj_set_size(box_moon_phases, 220, 180);
  lv_obj_align(box_moon_phases, LV_ALIGN_TOP_LEFT, 10, 135);
  lv_obj_set_style_bg_color(box_moon_phases, lv_color_hex(theme.box_bg), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_moon_phases, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_moon_phases, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_moon_phases, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_moon_phases, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_moon_phases, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_pad_all(box_moon_phases, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(box_moon_phases, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(box_moon_phases, moonp_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(box_moon_phases, LV_OBJ_FLAG_HIDDEN);

  img_moon_phases = lv_img_create(box_moon_phases);
  lv_img_set_src(img_moon_phases, &icon_6_last_quarter_moon);
  lv_img_set_zoom(img_moon_phases, 243);
  lv_obj_align(img_moon_phases, LV_ALIGN_TOP_MID, 1, 5);

  lbl_moon_phase_desc = lv_label_create(box_moon_phases);
  lv_label_set_text(lbl_moon_phase_desc, "temp. unavailable...");
  lv_obj_set_style_text_font(lbl_moon_phase_desc, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align_to(lbl_moon_phase_desc, img_moon_phases, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
  lv_obj_add_style(lbl_moon_phase_desc, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  // icon_santa_claus
  img_santa_claus = lv_img_create(box_moon_phases);
  lv_img_set_src(img_santa_claus, &icon_santa_claus);
  lv_img_set_zoom(img_santa_claus, 122);
  lv_obj_align_to(img_santa_claus, lbl_moon_phase_desc, LV_ALIGN_OUT_BOTTOM_MID, 0, -20);
  //lv_obj_align(img_santa_claus, LV_ALIGN_CENTER, 0, 10);

  lbl_santa_desc = lv_label_create(box_moon_phases);
  char buffer[32];
  char output[128];
  days_to_xmas = days_to_christmas();
  snprintf(buffer, sizeof(buffer), "%d", days_to_xmas);
  strcpy(output, buffer);
  strcat(output, strings->days_to_christmas_text);
  lv_label_set_text(lbl_santa_desc, output);
  lv_obj_set_style_text_font(lbl_santa_desc, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_align_to(lbl_santa_desc, img_santa_claus, LV_ALIGN_OUT_BOTTOM_MID, 0, -20);
  lv_obj_add_style(lbl_santa_desc, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  // Hidden by default; shown by update_days_to_christmas() once within 200 days of Christmas.
  if (days_to_xmas <= 0 || days_to_xmas > 200) {
    lv_obj_add_flag(img_santa_claus, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lbl_santa_desc, LV_OBJ_FLAG_HIDDEN);
  }

  // === UV INDEX / AIR QUALITY BOX (OpenWeather only) ====
  box_air_quality = lv_obj_create(scr);
  lv_obj_set_size(box_air_quality, 220, 180);
  lv_obj_align(box_air_quality, LV_ALIGN_TOP_LEFT, 10, 135);
  lv_obj_set_style_bg_color(box_air_quality, lv_color_hex(theme.box_bg), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_air_quality, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_air_quality, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_air_quality, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_air_quality, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_air_quality, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_pad_all(box_air_quality, 10, LV_PART_MAIN);
  lv_obj_set_layout(box_air_quality, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(box_air_quality, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(box_air_quality, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_event_cb(box_air_quality, aqi_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(box_air_quality, LV_OBJ_FLAG_HIDDEN);

  // Left half: UV index
  box_uv = lv_obj_create(box_air_quality);
  lv_obj_set_size(box_uv, 95, 160);
  lv_obj_set_style_bg_color(box_uv, lv_color_hex(theme.bg_top), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_uv, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_uv, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_uv, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_uv, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_uv, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_layout(box_uv, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(box_uv, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(box_uv, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(box_uv, 6, LV_PART_MAIN);
  lv_obj_add_flag(box_uv, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(box_uv, LV_OBJ_FLAG_EVENT_BUBBLE);

  lv_obj_t *lbl_uv_title = lv_label_create(box_uv);
  lv_label_set_text(lbl_uv_title, strings->uv_index_label);
  lv_obj_set_style_text_font(lbl_uv_title, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_add_style(lbl_uv_title, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  lbl_uv_value = lv_label_create(box_uv);
  lv_label_set_text(lbl_uv_value, "--");
  lv_obj_set_style_text_font(lbl_uv_value, get_font_20(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_add_style(lbl_uv_value, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  lbl_uv_desc = lv_label_create(box_uv);
  lv_label_set_text(lbl_uv_desc, "");
  lv_obj_set_style_text_font(lbl_uv_desc, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_uv_desc, lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  // Right half: air quality
  box_aqi = lv_obj_create(box_air_quality);
  lv_obj_set_size(box_aqi, 95, 160);
  lv_obj_set_style_bg_color(box_aqi, lv_color_hex(theme.bg_top), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_bg_opa(box_aqi, LV_OPA_COVER, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_radius(box_aqi, 4, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_border_width(box_aqi, 0, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_clear_flag(box_aqi, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(box_aqi, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_layout(box_aqi, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(box_aqi, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(box_aqi, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(box_aqi, 6, LV_PART_MAIN);
  lv_obj_add_flag(box_aqi, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(box_aqi, LV_OBJ_FLAG_EVENT_BUBBLE);

  lv_obj_t *lbl_aqi_title = lv_label_create(box_aqi);
  lv_label_set_text(lbl_aqi_title, strings->air_quality_label);
  lv_obj_set_style_text_font(lbl_aqi_title, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_add_style(lbl_aqi_title, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  lbl_aqi_value = lv_label_create(box_aqi);
  lv_label_set_text(lbl_aqi_value, "--");
  lv_obj_set_style_text_font(lbl_aqi_value, get_font_20(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_add_style(lbl_aqi_value, &default_label_style, ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  lbl_aqi_desc = lv_label_create(box_aqi);
  lv_label_set_text(lbl_aqi_desc, "");
  lv_obj_set_style_text_font(lbl_aqi_desc, get_font_12(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_aqi_desc, lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));

  // Create clock label in the top-right corner
  lbl_clock = lv_label_create(scr);
  lv_obj_set_style_text_font(lbl_clock, get_font_14(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_clock, lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_label_set_text(lbl_clock, "");
  lv_obj_align(lbl_clock, LV_ALIGN_TOP_RIGHT, -10, 5);

  // "Merry Christmas" label, centered between the Wi-Fi icon and the clock.
  // Hidden by default; shown by update_days_to_christmas() only on Christmas day.
  lbl_merry_christmas = lv_label_create(scr);
  lv_obj_set_style_text_font(lbl_merry_christmas, get_font_14(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_merry_christmas, lv_color_hex(theme.text_primary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_label_set_text(lbl_merry_christmas, strings->merry_christmas_text);
  lv_obj_align(lbl_merry_christmas, LV_ALIGN_TOP_MID, 0, 5);
  lv_obj_add_flag(lbl_merry_christmas, LV_OBJ_FLAG_HIDDEN);

  // Add sunrise/sunset info (2 row with 2 icons) after temperature

  // SUNRISE
  img_sunrise = lv_img_create(scr);
  lv_img_set_src(img_sunrise, &icon_sunrise);
  lv_img_set_zoom(img_sunrise, 61);
  lv_obj_align(img_sunrise, LV_ALIGN_TOP_MID, 1, 5);

  lbl_sunrise = lv_label_create(scr);
  lv_obj_set_style_text_font(lbl_sunrise, get_font_14(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_sunrise, lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_label_set_text(lbl_sunrise, "");
  lv_obj_align_to(lbl_sunrise, img_sunrise, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

  // SUNSET
  img_sunset = lv_img_create(scr);
  lv_img_set_src(img_sunset, &icon_sunset);
  lv_img_set_zoom(img_sunset, 61);
  lv_obj_align(img_sunset, LV_ALIGN_TOP_MID, 1, 20);

  lbl_sunset = lv_label_create(scr);
  lv_obj_set_style_text_font(lbl_sunset, get_font_14(), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_obj_set_style_text_color(lbl_sunset, lv_color_hex(theme.text_tertiary), ((lv_style_selector_t)LV_PART_MAIN | LV_STATE_DEFAULT));
  lv_label_set_text(lbl_sunset, "");
  lv_obj_align_to(lbl_sunset, img_sunset, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

  // Slideshow timer (PAGE_SLIDESHOW_INTERVAL): no-op each tick unless page_slideshow_enabled is set.
  lv_timer_create(page_slideshow_cb, PAGE_SLIDESHOW_INTERVAL, NULL);
}
