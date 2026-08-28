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
#define COUNTRY_CODE_DEFAULT "GB"
#define DEFAULT_CAPTIVE_SSID "Aurigae"
#define NTP_SERVER_DEFAULT "pool.ntp.org"
#define MOON_SERVICE_URL "https://api.aurigae.fizban.net/api/moonphase?date=today"
// esp-web-tools manifest for the public web flasher (https://aurigae.fizban.net/flash.html);
// polled by the webconfig page's /checkupdate endpoint to offer a firmware.bin download when
// its "version" is newer than APP_VERSION below.
#define FIRMWARE_MANIFEST_URL "https://aurigae.fizban.net/firmware/manifest.json"
#define UPDATE_INTERVAL 600000UL        // 10 minutes
#define ANTIBURN_IDLE_TIMEOUT_DEFAULT_MIN 10  // default antiburn idle timeout, in minutes
#define ANTIBURN_IDLE_TIMEOUT_MIN_MIN 10      // webconfig-allowed minimum, in minutes
#define ANTIBURN_IDLE_TIMEOUT_MAX_MIN 30      // webconfig-allowed maximum, in minutes
#define ANTIBURN_CHECK_INTERVAL 30000UL       // how often to check idle time
#define PAGE_SLIDESHOW_INTERVAL 5000UL  // auto-cycle page every 5s when enabled

#define APP_VERSION "App ver. 2.2.0"
