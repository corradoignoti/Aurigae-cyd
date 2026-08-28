# Changelog

All notable changes to Aurigae are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/); versions below match the
`APP_VERSION` string shown in the on-device Settings screen / webconfig page
and the corresponding git tags where one exists.

## [Unreleased]
### Added
- OTA firmware update: `/update` endpoint on the webconfig page, upload a built `.bin` and the
  device flashes + reboots itself. Required switching `board_build.partitions` from
  `huge_app.csv` (no OTA slot) to `min_spiffs.csv` (dual ~1.9MB OTA slots) — flash usage is now
  ~98% of each slot, so future size growth will need trimming (see 2.1.0 icon/lv_conf work).
- `/checkupdate`: webconfig page polls the public web-flasher's manifest
  (aurigae.fizban.net/firmware/manifest.json) and shows an update banner with a direct
  firmware.bin download link when a newer version is published there.
- Page indicator dots on the cycled forecast boxes (daily/hourly/moon-phase/AQI), showing which
  box is currently displayed.

### Changed
- Firmware size shrunk ~24% via `lv_conf.h` trims + indexed icon assets, clawing back headroom
  in the OTA dual-slot partition scheme (see above).
- Open-Meteo weather fetch now uses the ARPAE ICON-2I model for Italian locations.
- Webconfig OTA section links to aurigae.fizban.net/firmware.html as a prebuilt-firmware.bin
  download source, alongside the existing manifest-driven update banner.

## [2.1.0] - 2026-08-25
### Added
- `/screenshot` debug endpoint on the webconfig page — streams current display contents as an
  uncompressed BMP, read back off the ILI9341 GRAM.
- Next-4-hours forecast strip on main weather view.

### Changed
- 5-day daily forecast now always shown (previously hidden under some conditions).
- Weather icons shrunk 5% to fit new layout.
- Webconfig page restyled with Material UI.

## [2.0.0] - 2026-08-24
### Added
- LICENSE file (GPLv3), with README carve-out for icon assets not covered by it.

### Changed
- Moon phase service URL moved to `MOON_SERVICE_URL` in `config.h`.

## Earlier
- Initial commit and project setup, based on the
  [Aura project](https://github.com/Surrey-Homeware/Aura).
