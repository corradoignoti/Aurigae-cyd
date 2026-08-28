# Aurigae

Auriage is a simple weather widget that runs on ESP32-2432S028R ILI9341 devices with a 2.8" screen. These devices are sometimes called a "CYD" or Cheap Yellow Display.

Aurigae is based on Aura project: [https://github.com/Surrey-Homeware/Aura](https://github.com/Surrey-Homeware/Aura)

This is the source code for the project. 
You can print a case for the deivce using the Aura's project 3MF file: https://makerworld.com/en/models/1382304-aura-smart-weather-forecast-display

For using the device once flashed (Wi-Fi setup, settings screen, web config page), see
[docs/user-guide.md](docs/user-guide.md).

### Web configuration page

Once on Wi-Fi, device run small built-in web server on port 80. Device IP show on settings screen ("Config: ..."). Open it in browser for:

- NTP server setting
- Location search/change (lat/long + name, via Open-Meteo geocoding)
- Clock format (12h/24h)
- Temperature unit (°C/°F)
- Language
- Weather provider — Open-Meteo (default, no key needed) or OpenWeather (needs free API key,
  5-day/3-hour forecast only; also add UV index / air quality box to main screen)

Saving reboot device to apply.

Page also has OTA firmware update: `/update` endpoint accept upload of built `.bin`, device flash
and reboot into it (unauthenticated — anyone on LAN can flash it). Page checks
aurigae.fizban.net for newer published firmware and shows update banner with direct
firmware.bin link when found.

### License

You can use the aurigae.cpp code here under the terms of the GPL 3.0 license.

The icons are not included in that license. See "Thanks" below for details on the icons.

### How to compile:

This is a [PlatformIO](https://platformio.org/) project built against the
[pioarduino](https://github.com/pioarduino/platform-espressif32) espressif32 platform fork (it tracks newer
arduino-esp32 core releases than the stock PlatformIO espressif32 platform).

1. Install [PlatformIO](https://platformio.org/install) (CLI or the VS Code extension).
1. Open this repo as a PlatformIO project — [platformio.ini](platformio.ini) declares the platform, board
   (`esp32dev`, i.e. a generic ESP32 Dev Module), partition scheme (`min_spiffs.csv`: dual ~1.9MB OTA app
   slots + small unused SPIFFS partition, needed for the webconfig `/update` OTA endpoint), and all library
   dependencies, so no manual library install step is needed.
1. Build & upload:
   ```
   pio run -t upload
   ```
1. Monitor serial output at 115200 baud:
   ```
   pio device monitor
   ```

Source layout:
- `src/` — sketch source (entry point `aurigae.cpp`) plus the generated LVGL image/font assets
  (`icon_*.c`, `image_*.c`, `lv_font_montserrat_latin_*.c`).
- `include/` — `lv_conf.h` (LVGL config) and `User_Setup.h` (TFT_eSPI pin/driver config); both are wired in
  via `build_flags` in [platformio.ini](platformio.ini).
- `tools/extract_unicode_chars.py` — helper to find non-ASCII characters a translation needs, for
  regenerating the LVGL fonts.
- `tools/make_webflasher_zip.sh` — build + package firmware (manifest.json + binary parts) for the
  [esp-web-tools](https://esphome.github.io/esp-web-tools/) browser flasher.
- `assets/` — source icon artwork the generated `.c` files in `src/` are derived from.
- `docs/user-guide.md` — end-user guide (Wi-Fi setup, settings screen, web config page).
- `changelog.md` — release notes per version.

### Libraries required to compile:

Declared as `lib_deps` in [platformio.ini](platformio.ini) — PlatformIO fetches them automatically:

- ArduinoJson 7.4.1
- TFT_eSPI 2.5.43
- WifiManager 2.0.17
- XPT2046_Touchscreen 1.4
- lvgl 9.2.2 (upstream Arduino Library Manager lists 9.2.3, but PlatformIO's registry has no matching
  git tag for that patch — 9.2.2 is config-compatible with this project's `lv_conf.h`)

(`HTTPClient` is bundled with the arduino-esp32 framework itself, no separate dependency needed.)

To show moonphases Aurigae connects to a service hosted on one of my server: it's free and given as-is. You can host the service on your own using this code: [https://github.com/corradoignoti/aurigae_moonphases](https://github.com/corradoignoti/aurigae_moonphases). Change in config.h the value of MOON_SERVICE_URL

### License

Aurigae code is licensed under the [GNU General Public License v3.0](LICENSE).

Exceptions:
- Weather icons (`src/icon_*.c`/`src/image_*.c` derived from `assets/`, sourced from
  [google-weather-icons](https://github.com/mrdarrengriffin/google-weather-icons/tree/main/v2)) are property
  of Google and are **not** covered by the GPLv3 grant — same carve-out as upstream Aura. They're included
  as-is; redistribution rights beyond this project are Google's to grant, not ours.
- Moon phase and Santa Claus icons under `assets/` have no recorded license or credited source
  (provenance unknown); treat them the same as the weather icons above until that's resolved.

### Thanks & Credits

- Weather icons from https://github.com/mrdarrengriffin/google-weather-icons/tree/main/v2
- Thanks to [lvgl](https://lvgl.io/), a great library for UIs on ESP32 devices that made this much easier
- Thanks to [witnessmenow](https://github.com/witnessmenow/)'s [CYD Github repo](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) for dev board reference information
- Double thanks to [witnessmenow](https://github.com/witnessmenow/) for the [ESP32 web flashing tutorial](https://github.com/witnessmenow/ESP-Web-Tools-Tutorial)
- Thanks to [Random Nerd Tutorials](https://randomnerdtutorials.com/) for helpful ESP32 / CYD information, especially with [setting up LVGL](https://randomnerdtutorials.com/esp32-cyd-lvgl-line-chart/)
- Thanks to these sweet libraries that made this possible:
	- [ArduinoJson](https://arduinojson.org/)
	- [HttpClient](https://github.com/amcewen/HttpClient)
	- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)
	- [WifiManager](https://github.com/tzapu/WiFiManager)
	- [XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen)
	- [lvgl](https://lvgl.io/)
