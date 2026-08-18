#pragma once

// Board / display pins and geometry (specific to the ESP32-2432S028R "Cheap
// Yellow Display" board variant).
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS
#define LCD_BACKLIGHT_PIN 21
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

#define LATITUDE_DEFAULT "51.5074"
#define LONGITUDE_DEFAULT "-0.1278"
#define LOCATION_DEFAULT "London"
#define DEFAULT_CAPTIVE_SSID "Aurigae"
#define NTP_SERVER_DEFAULT "pool.ntp.org"
#define MOON_SERVICE_URL "https://api.aurigae.fizban.net/api/moonphase?date=today"
#define UPDATE_INTERVAL 600000UL        // 10 minutes
#define ANTIBURN_IDLE_TIMEOUT 600000UL  // dim after 10 min with no touch
#define ANTIBURN_CHECK_INTERVAL 30000UL // how often to check idle time

#define APP_VERSION "App ver. 2.0.0"
