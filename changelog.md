# Changelog

All notable changes to Aurigae are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/); versions below match the
`APP_VERSION` string shown in the on-device Settings screen / webconfig page
and the corresponding git tags where one exists.

## [2.1.0] - 2026-08-25
### Added
- `/screenshot` debug endpoint on the webconfig page — streams current display contents as an
  uncompressed BMP, read back off the ILI9341 GRAM.

## [2.0.0] - 2026-08-24
### Added
- LICENSE file (GPLv3), with README carve-out for icon assets not covered by it.

### Changed
- Moon phase service URL moved to `MOON_SERVICE_URL` in `config.h`.

## Earlier
- Initial commit and project setup, based on the
  [Aura project](https://github.com/Surrey-Homeware/Aura).
