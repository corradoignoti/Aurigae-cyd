#pragma once

#include <stdint.h>

// UI accent color theme, changed only from the web config page (not the
// on-device Settings screen). Applied by create_ui()/wifi_splash_screen() at
// startup, so a theme change takes effect after the reboot the web config
// page already triggers for language/provider changes.
enum UiTheme { THEME_BLUE = 0, THEME_RED = 1, THEME_GREEN = 2, THEME_DARK = 3 };
extern UiTheme current_theme;

// The handful of colors ui_main.cpp draws with, one palette per theme.
struct ThemeColors {
  uint32_t bg_top;         // screen background gradient, top / UV+AQI box background
  uint32_t bg_bottom;      // screen background gradient, bottom
  uint32_t box_bg;         // daily/hourly/moon-phase/air-quality box background
  uint32_t text_primary;   // main label color (white)
  uint32_t text_secondary; // feels-like / forecast title
  uint32_t text_tertiary;  // low temp, precip %, uv/aqi desc, clock, sunrise/sunset
};

const ThemeColors &get_theme_colors();
