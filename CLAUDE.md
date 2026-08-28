# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Aurigae is a weather/moon-phase/countdown widget for ESP32-2432S028R ("Cheap Yellow Display" / CYD) devices
with a 2.8" ILI9341 touchscreen. It's an Arduino sketch (split across several `.cpp`/`.h` files under
`src/`, all compiled and linked together — see Architecture below), not a hosted app — there is no package
manager, build script, or test runner. It's based on the Aura project (https://github.com/Surrey-Homeware/Aura).

## Build / compile

This is a PlatformIO project ([platformio.ini](platformio.ini)) built against the **pioarduino**
espressif32 platform fork (https://github.com/pioarduino/platform-espressif32) rather than the stock
PlatformIO espressif32 platform, to get newer arduino-esp32 core releases.

```
pio run -t upload        # build + flash
pio device monitor        # serial monitor, 115200 baud
```

- Board: `esp32dev` (generic ESP32 Dev Module) — matches the ESP32-2432S028R "Cheap Yellow Display" target.
- `board_build.partitions = min_spiffs.csv` — dual ~1.9MB OTA app slots (app0/app1) + a small unused
  SPIFFS partition, needed for the `/update` OTA endpoint (see webconfig.h below) to have somewhere
  to flash into. Firmware currently runs ~98% of each OTA slot's capacity — thin headroom, watch
  flash usage (`pio run`'s "Flash:" line) as features/assets are added.
- All libraries (ArduinoJson 7.4.1, TFT_eSPI 2.5.43, WifiManager 2.0.17, XPT2046_Touchscreen 1.4,
  lvgl 9.2.2) are declared as `lib_deps` and fetched automatically — no manual library install step.
  (Upstream's Arduino Library Manager lists 9.2.3, but PlatformIO's registry mirrors lvgl/lvgl's git tags,
  which stop at v9.2.2 before jumping to v9.3.0 — 9.2.2 is config-compatible with our lv_conf.h.)
  `HTTPClient` itself comes bundled with the arduino-esp32 framework.
- [include/lv_conf.h](include/lv_conf.h) (LVGL config) and [include/User_Setup.h](include/User_Setup.h)
  (TFT_eSPI pin/driver config) are wired in via `build_flags` (`-D LV_CONF_INCLUDE_SIMPLE=1` and
  `-D USER_SETUP_LOADED=1 -include include/User_Setup.h` respectively) rather than being copied into library
  folders as the Arduino IDE workflow requires.

There is no automated test suite or linter for this project. Verification is: does it compile, and does it
behave correctly on real hardware (or by careful code reading).

`aurigae/aurigae.ino.orig` is a stale backup containing old unresolved merge-conflict markers — not
canonical, don't edit or treat as reference. (It's the only thing left in `aurigae/`; the live sketch now
lives under [src/](src/), entry point [src/aurigae.cpp](src/aurigae.cpp).)

## Architecture

The sketch is split by concern across `src/*.cpp`/`src/*.h` (all compiled and linked into one binary by
PlatformIO — there's no need to declare new files anywhere). Shared state that more than one file touches
(LVGL widget pointers, `Preferences`, network/geocoding scratch state, touch/antiburn flags) lives in
[src/globals.h](src/globals.h)/[globals.cpp](src/globals.cpp) as `extern` declarations; state used by only
one file is kept `static` inside that file instead. Key files:

- [src/aurigae.cpp](src/aurigae.cpp) — `setup()`/`loop()` and the `update_clock()` timer callback. This is
  the thin entry point; almost everything else has moved out.
- [src/config.h](src/config.h) — pin/screen/timing `#define`s and default lat/long/location/NTP values.
- [src/globals.h](src/globals.h) / [globals.cpp](src/globals.cpp) — cross-file shared state (see above).
- [src/fonts.h](src/fonts.h) — `LV_FONT_DECLARE`s + `get_font_NN()` helpers.
- [src/icons.h](src/icons.h) — `LV_IMG_DECLARE`s for every weather/moon-phase/Santa asset.
- [src/localization.h](src/localization.h)/[localization.cpp](src/localization.cpp) — `LocalizedStrings`
  struct + one static instance per language (`strings_en/es/de/fr/it`); `get_strings()` dispatches on the
  `current_language` global. Adding a language means adding a new struct literal with every field populated,
  plus a `Language` enum entry and a dropdown option in `create_settings_window()`.
  [tools/extract_unicode_chars.py](tools/extract_unicode_chars.py) finds the non-ASCII characters a
  translation needs so the LVGL fonts can be regenerated to include them.
- **Display/UI stack**: TFT_eSPI drives the ILI9341 panel; LVGL (v9) owns the widget tree and event loop;
  XPT2046_Touchscreen feeds touch input into LVGL via `touchscreen_read()` ([src/power.h](src/power.h)/
  [power.cpp](src/power.cpp), alongside the antiburn/backlight logic below). `lv_timer_handler()` is pumped
  every iteration of `loop()`.
- **Screens are LVGL object trees built/destroyed on demand**, not separate LVGL "screens":
  [src/ui_main.h](src/ui_main.h)/[ui_main.cpp](src/ui_main.cpp)'s `create_ui()` builds the main weather
  view; tapping it opens the settings window ([src/ui_settings.h](src/ui_settings.h)/
  [ui_settings.cpp](src/ui_settings.cpp)'s `create_settings_window()`); from there, "Change Location" opens
  [src/ui_location.h](src/ui_location.h)/[ui_location.cpp](src/ui_location.cpp)'s
  `create_location_dialog()`. Within the main view, tapping the daily-forecast box cycles to the
  hourly-forecast box, then moon-phase, then UV/air-quality, then back (`daily_cb`/`hourly_cb`/`moonp_cb`/
  `aqi_cb` in ui_main.cpp, toggling `LV_OBJ_FLAG_HIDDEN` on the corresponding box). A generic single-button
  modal (`showDialog()`) lives in [src/dialog.h](src/dialog.h)/[dialog.cpp](src/dialog.cpp).
- **Config/persistence**: ESP32 `Preferences` (NVS), namespace `"weather"`, stores lat/long, location name,
  brightness, °F/°C, 12/24hr, language, weather provider, and OpenWeather API key. Loaded once in `setup()`,
  written back when settings close (or immediately on save, for the webconfig-only weather-provider setting
  — [src/webconfig.h](src/webconfig.h)/[webconfig.cpp](src/webconfig.cpp)).
- **OTA update**: the webconfig page's `/update` endpoint (unauthenticated, like the rest of
  webconfig — anyone on the LAN can flash it) accepts a POSTed `.bin` and writes it into the
  inactive OTA slot via the `Update` library (`handle_webconfig_update_upload`/`_done` in
  webconfig.cpp), rebooting into it on success. Requires the OTA-capable partition table set in
  `platformio.ini` (see above) — falls back to leaving the running firmware untouched on any
  write/verify failure. `/checkupdate` (`handle_webconfig_checkupdate`) polls
  `FIRMWARE_MANIFEST_URL` (config.h — the esp-web-tools manifest behind the public web flasher at
  aurigae.fizban.net/flash.html) and, if its `version` is newer than `APP_VERSION`
  (`version_is_newer()` in util.h/cpp), the root page shows a banner linking straight to the
  matching `firmware.bin`.
- **Wi-Fi provisioning**: `WiFiManager` auto-connects to saved credentials or falls back to a captive AP named
  by `DEFAULT_CAPTIVE_SSID` ("Aurigae"); [src/wifi_provisioning.h](src/wifi_provisioning.h)/
  [wifi_provisioning.cpp](src/wifi_provisioning.cpp)'s `apModeCallback()` shows a splash screen while in AP
  mode. Settings screen has a "Reset Wi-Fi" flow (`reset_wifi_event_handler` in ui_settings.cpp → confirm
  dialog → `wm.resetSettings()` + `esp_restart()`).
- **Data sources**, both polled on boot and every `UPDATE_INTERVAL` (10 min) from `loop()`:
  - [src/weather.h](src/weather.h)/[weather.cpp](src/weather.cpp)'s `fetch_and_update_weather()` —
    dispatches on `weather_provider` (webconfig-only setting, not on the on-device Settings screen) to one
    of two backends, both driving the same current/daily/hourly UI and (re)syncing the device clock via NTP
    using the UTC offset the API returns:
    - [src/weather_openmeteo.cpp](src/weather_openmeteo.cpp) (default) — Open-Meteo forecast API, 7-day
      daily forecast, plus `fetch_openmeteo_air_quality()`.
    - [src/weather_openweather.cpp](src/weather_openweather.cpp) — OpenWeather free tier
      (`/data/2.5/weather` + `/data/2.5/forecast`, requires an API key set via the webconfig page); only a
      3-hourly/5-day forecast is available on the free tier, so daily values are aggregated from 3-hour
      buckets and only 5 daily-box rows are shown (the other two hidden via `LV_OBJ_FLAG_HIDDEN`, un-hidden
      again if Open-Meteo is fetched later), plus `fetch_openweather_uv_and_air_quality()`.
    - `translate_owm_code_to_wmo()` and the weather-code → asset mapping (`choose_icon()`/`choose_image()`,
      switching on WMO codes with day/night variants) live in
      [src/weather_icons.cpp](src/weather_icons.cpp), declared in weather.h.
  - [src/moonphase.h](src/moonphase.h)/[moonphase.cpp](src/moonphase.cpp)'s `fetch_and_update_moonphase()` —
    a separate custom API (`api.aurigae.fizban.net`) returning a `phase_id` (0–7) mapped to icon + localized
    label.
  - [src/geocode.h](src/geocode.h)/[geocode.cpp](src/geocode.cpp)'s `do_geocode_query()` — Open-Meteo
    geocoding API, used by the location-search dialog.
- [src/christmas.h](src/christmas.h)/[christmas.cpp](src/christmas.cpp) — `day_of_week()` (also used to
  label daily-forecast rows), `days_to_christmas()`, and `update_days_to_christmas()` (Santa/"Merry
  Christmas"/"Happy New Year" UI).
- [src/util.h](src/util.h)/[util.cpp](src/util.cpp) — `hour_of_day()`, `urlencode()`, and two unused-but-
  preserved time-formatting helpers (`convertTo24hAndRound()`, `roundToMinute()`).
- **Screen-burn mitigation** ([src/power.h](src/power.h)/[power.cpp](src/power.cpp)): `antiburn` timer
  blanks the backlight after 10 min idle; `restore_light()` un-blanks it (called on touch interactions);
  `force_wakeup` timer forces the backlight pin back on every 30 min because ESP32 deep sleep otherwise
  leaves the touchscreen unresponsive (see git history).

## Assets

- `src/icon_*.c`, `src/image_*.c` — LVGL image descriptors (`lv_img_dsc_t`) for weather conditions, moon
  phases, sun, Santa Claus; generated from source art, declared via `LV_IMG_DECLARE` in
  [src/icons.h](src/icons.h) and consumed by `choose_icon`/`choose_image` in
  [src/weather_icons.cpp](src/weather_icons.cpp). Compiled alongside everything else in `src/` because the
  whole directory is one PlatformIO build target.
- `src/lv_font_montserrat_latin_*.c` — generated LVGL bitmap fonts at fixed sizes (12/14/16/20/42), declared
  via `LV_FONT_DECLARE` and selected through `get_font_NN()` helpers.
- `assets/` — source icon artwork (Moon icons, Santa Claus SVGs) that the generated `.c` files in `src/` are
  derived from; not itself compiled into the sketch.
- `tools/extract_unicode_chars.py` — dev helper (not part of the build) that scans a given source file
  (usually [src/localization.cpp](src/localization.cpp), where the translated strings live) for non-ASCII
  characters a translation needs, for regenerating the LVGL fonts with the right glyph coverage.

Screen geometry is hardcoded to 240×320 (`SCREEN_WIDTH`/`SCREEN_HEIGHT`) and touch pins/calibration
(`XPT2046_*` defines, the `map()` calls in `touchscreen_read()`) are specific to this CYD board variant.
