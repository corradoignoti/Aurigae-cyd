# Changelog

All notable changes to Aurigae are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/); versions below match the
`APP_VERSION` string shown in the on-device Settings screen / webconfig page
and the corresponding git tags where one exists.

## [2.0.0] - 2025 (current, `dev`)
### Added
- Color accent theme support (web config only).
- UV index / air quality page for Open-Meteo provider.
- UV index / air quality page for OpenWeather provider.
- OpenWeather as an alternate weather provider (webconfig-selectable), with
  aggregated 5-day daily forecast from its 3-hourly free-tier data.
- "Merry Christmas" / "Happy New Year" banner; big Santa now hides past 200
  days to Christmas.
- Location and clock-format sections in the webconfig page.
- More setting configs exposed via webconfig.
- README and user docs updated for the new provider/config options.

### Changed
- Converted project to PlatformIO (pioarduino espressif32 fork).
- Refactored `aurigae.cpp` into per-concern files under `src/` (see
  Architecture in [CLAUDE.md](CLAUDE.md)).
- Translated all in-code comments to English.
- Removed leftover references to the old MQTT connection feature.
- WiFi icon now shown when a connection is available.

### Fixed
- Antiburn (screen-burn mitigation) bug.

## [1.5.0-beta3]
### Added
- Connection-status label (currently shows WiFi status only).

## [1.5.0-beta2]
### Added
- Button to show the webconfig URL on-device.
### Fixed
- MQTT reconnect bug.

## [1.5.0-beta1]
### Added
- Webconfig page, MQTT functions.
### Changed
- Webconfig page structure reworked.
### Fixed
- Assorted beta-1 bugfixes.

## [1.4.3]
### Added
- Force-wakeup function every 30 min, working around an ESP32 deep-sleep bug
  that leaves the touchscreen unresponsive.

## [1.4.2]
### Added
- LCD antiburn function (blanks backlight after idle).

## [1.4.1]
### Added
- Moon phase page and icon; Santa icon.
- Christmas countdown.
- Sunrise/sunset fetched from the weather API (replacing the old dedicated
  sunrise/sunset API calls).
- Humidity indication.
### Removed
- MQTT alert function (deemed too unstable).

## [1.3.0]
### Added
- MQTT connection to show alerts on screen (later removed in 1.4.x for
  stability).

## [1.2.2]
### Fixed
- Sunrise/sunset time rounding when 24h format is not selected.
- Sunrise/sunset now fetched for today instead of tomorrow.
- Sunrise/sunset time rounded correctly even when 12h format is selected.
- Reverted a function that caused a memory allocation error.

## [1.2.0]
### Added
- New sunrise/sunset time GUI, including position-on-display work.
- Rounding applied to time conversion.
### Changed
- Sunrise/sunset time now shown for tomorrow.

## [1.0.0]
### Added
- Italian localization (i18n).
- App version shown on the Settings screen.
### Fixed
- Day-of-week spelling ("Thurs" → "Thu").
- Italian translation typo.

## Earlier
- Initial commit and project setup, based on the
  [Aura project](https://github.com/Surrey-Homeware/Aura).
