#include "power.h"
#include <Arduino.h>
#include <XPT2046_Touchscreen.h>
#include "config.h"
#include "globals.h"

//Resume LCD brightness
void restore_light(){
  analogWrite(LCD_BACKLIGHT_PIN, default_lcd_brightness);
}

//Checked periodically; dims backlight only after real idle time, not on a blind schedule
void antiburn(lv_timer_t *timer) {
  if (!screen_dimmed && millis() - last_touch_ms >= antiburn_idle_timeout_ms) {
    analogWrite(LCD_BACKLIGHT_PIN, 0);
    screen_dimmed = true;
  }
}

// ESP32 deep sleep otherwise leaves the touchscreen unresponsive; re-assert pin mode
// periodically without undoing a deliberate antiburn dim.
void force_wakeup(lv_timer_t *timer) {
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  // pinMode() detaches the pin from LEDC/analogWrite, leaving backlight level
  // undefined (usually snaps HIGH) — reassert whichever state we're actually in.
  if (screen_dimmed) {
    analogWrite(LCD_BACKLIGHT_PIN, 0);
  } else {
    restore_light();
  }
}

void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();

    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;

    last_touch_ms = millis();
    if (screen_dimmed) {
      restore_light();
      screen_dimmed = false;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
