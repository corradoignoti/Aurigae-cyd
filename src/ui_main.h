#pragma once

// Builds the main weather view (temperature, forecast boxes, sunrise/sunset,
// clock, Wi-Fi/Christmas indicators). Tapping the screen opens the settings
// window; tapping the forecast box cycles daily -> hourly -> moon phases ->
// UV/air quality -> daily.
void create_ui();

// Full-screen "connect to configure" message shown while WiFiManager's
// captive AP is up.
void wifi_splash_screen();

// Shows/hides the top-left Wi-Fi icon based on connection status; called
// every loop() iteration.
void update_wifi_status_icon();

// Shows/hides the top-bar download icon (next to the Wi-Fi icon) based on
// firmware_update_available (globals.h); called by check_for_firmware_update().
void update_firmware_update_icon();
