#pragma once

#include <lvgl.h>

// LVGL touch-input read callback; also resets the antiburn idle timer and
// un-dims the backlight on touch.
void touchscreen_read(lv_indev_t *indev, lv_indev_data_t *data);

// Restores the backlight to default_lcd_brightness.
void restore_light();

// LVGL timer: dims the backlight after ANTIBURN_IDLE_TIMEOUT of no touch.
void antiburn(lv_timer_t *timer);

// LVGL timer: re-asserts the backlight pin mode periodically so ESP32 deep
// sleep doesn't leave the touchscreen unresponsive.
void force_wakeup(lv_timer_t *timer);
