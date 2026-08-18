#include "christmas.h"
#include <Arduino.h>
#include <time.h>
#include <lvgl.h>
#include "globals.h"
#include "localization.h"

int day_of_week(int y, int m, int d) {
  static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// Calculate days to Christmas
int days_to_christmas(void) {

    struct tm now_tm;
    if (!getLocalTime(&now_tm)) {
      Serial.print("days_to_christmas: !!!Can't get local time!!!");
      return -1;
    }

    // Today midnight (localour)
    struct tm today = {0};
    today.tm_year = now_tm.tm_year;
    today.tm_mon = now_tm.tm_mon;
    today.tm_mday = now_tm.tm_mday;
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    today.tm_isdst = -1; // let mktime calculate DST

    time_t today_mid = mktime(&today);
    if (today_mid == (time_t)-1) return -1;

    // 25 dec. of this year
    struct tm christmas = {0};
    christmas.tm_year = now_tm.tm_year;
    christmas.tm_mon = 11; // 0=jan ... 11=dec
    christmas.tm_mday = 25;
    christmas.tm_hour = 0;
    christmas.tm_min = 0;
    christmas.tm_sec = 0;
    christmas.tm_isdst = -1;

    time_t christmas_mid = mktime(&christmas);
    if (christmas_mid == (time_t)-1) return -1;

    // This year Christmas passed, let's calc days to next one
    if (christmas_mid < today_mid) {
        christmas.tm_year += 1;
        christmas_mid = mktime(&christmas);
        if (christmas_mid == (time_t)-1) return -1;
    }

    double diff = difftime(christmas_mid, today_mid);
    int days = (int)(diff / 86400.0);

    return days;
}

void update_days_to_christmas() {
  char buffer[32];
  char output[128];
  days_to_xmas = days_to_christmas();

  const LocalizedStrings* strings = get_strings();
  snprintf(buffer, sizeof(buffer), "%d", days_to_xmas);
  strcpy(output, buffer);
  strcat(output, strings->days_to_christmas_text);
  lv_label_set_text(lbl_santa_desc, output);

  if (img_small_santa_claus) {
    if (days_to_xmas > 0 && days_to_xmas <= 30) {
      lv_obj_clear_flag(img_small_santa_claus, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(img_small_santa_claus, LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (img_santa_claus) {
    if (days_to_xmas > 0 && days_to_xmas <= 200) {
      lv_obj_clear_flag(img_santa_claus, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(img_santa_claus, LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (lbl_santa_desc) {
    if (days_to_xmas > 0 && days_to_xmas <= 200) {
      lv_obj_clear_flag(lbl_santa_desc, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(lbl_santa_desc, LV_OBJ_FLAG_HIDDEN);
    }
  }

  // It's Christmas let's celebrate it!
  // On the first day of the year show a "Happy new year message"
  if (lbl_merry_christmas) {
    struct tm now_tm;
    bool is_new_year_day = getLocalTime(&now_tm) && now_tm.tm_mon == 0 && now_tm.tm_mday == 1;

    if (is_new_year_day) {
      lv_label_set_text(lbl_merry_christmas, strings->happy_new_year_text);
      lv_obj_align(lbl_merry_christmas, LV_ALIGN_TOP_MID, 0, 5);
      lv_obj_clear_flag(lbl_merry_christmas, LV_OBJ_FLAG_HIDDEN);
    } else if (days_to_xmas == 0) {
      lv_label_set_text(lbl_merry_christmas, strings->merry_christmas_text);
      lv_obj_align(lbl_merry_christmas, LV_ALIGN_TOP_MID, 0, 5);
      lv_obj_clear_flag(lbl_merry_christmas, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(lbl_merry_christmas, LV_OBJ_FLAG_HIDDEN);
    }
  }
}
