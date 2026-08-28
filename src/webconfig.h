#pragma once

// Local HTTP config server (port 80): lets a browser on the same network set
// the NTP server, clock format, temperature unit, language, color theme,
// weather provider (+ OpenWeather API key), and location without touching
// the touchscreen UI. Also serves /screenshot, a BMP dump of the current
// display contents read back over SPI, for remote debugging, and /update, an
// OTA firmware upload endpoint (POST a built .bin, flashed via the Update
// library into the inactive OTA slot — see board_build.partitions in
// platformio.ini for the OTA-capable partition table this requires).
// /checkupdate polls FIRMWARE_MANIFEST_URL (config.h) — the same esp-web-tools
// manifest the public web flasher reads — and reports whether it advertises a
// version newer than APP_VERSION, for an update banner on the root page.
void start_webconfig_server();
