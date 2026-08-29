# Aurigae User Guide

Aurigae is weather / moon-phase / countdown widget for ESP32 "Cheap Yellow Display" (CYD) devices.
This guide cover using device after it flashed — no build/dev info here (see [README.md](../README.md)
for that).

## First setup (Wi-Fi)

On first boot (or after Wi-Fi reset), device can't connect and open captive portal:

1. On phone/computer, connect to Wifi network named **Aurigae**.
2. Portal page open automatically (or browse to `192.168.4.1`).
3. Pick your home Wi-Fi network, enter password, save.
4. Device reboot and connect. Screen show weather once connected.

Device remember credential after that — reconnect automatic on power-up.

## Main screen

Main screen show current weather for saved location, plus one of several boxes:

- **Daily forecast** (default)
- **Hourly forecast**
- **Moon phase**
- **UV index / Air quality** — only when weather provider is OpenWeather (see web config page below)

Tap the box to cycle: daily → hourly → moon phase → back to daily (Open-Meteo), or daily → hourly →
moon phase → UV index / air quality → back to daily (OpenWeather). Dots top-right of box title show
current page; active dot bigger, others dim. Web config page can turn on auto-cycle (every 5s) instead
of tapping — see below.

Tap anywhere else on screen (outside the box) to open **Settings**.

A download icon appear next to Wi-Fi icon (top bar) when new firmware available — device also pop a
30-second dialog saying so (wake screen first if dimmed). Icon clear once device update. Go to web
config page to install (see **OTA firmware update** below).

## Settings screen

Reach it by tapping main screen (outside forecast box).

| Control | What it do |
|---|---|
| Brightness slider | Set backlight level, saved immediately |
| °F switch | Toggle Fahrenheit / Celsius |
| 24hr switch | Toggle 24-hour / 12-hour clock |
| Location | Show current location name; tap **Change Location** to search new one |
| Language dropdown | English, Español, Deutsch, Français, Italiano — UI update immediately |
| Reset Wifi (red button) | Confirm dialog, then erase saved Wi-Fi and reboot into setup portal (see above) |
| Config: `<IP address>` | Device's address on your network — open in browser for web config page (below) |
| Close | Save settings and return to main screen |

### Change Location

From Settings, tap **Change Location** to open search dialog. Type a place name (on-screen keyboard
appear), pick match from dropdown result. New coordinates take effect right away.

## Web configuration page

Once connected to Wi-Fi, device also run small web server. Settings screen show its address as
`Config: <IP address>` — open that address in any browser on same network for:

- **NTP server** — change time-sync server (default work for most, change only if you know you need to)
- **Clock format** — 12h/24h toggle
- **Temperature unit** — °C/°F toggle
- **Language** — English, Español, Deutsch, Français, Italiano
- **Location** — same search-and-save as on-device dialog, from browser instead
- **Weather provider** — Open-Meteo (default, no key needed) or OpenWeather (needs free API key from
  [openweathermap.org](https://openweathermap.org/api)). OpenWeather's free tier only give 5-day
  forecast, so daily box show 5 rows instead of 7 when picked. Picking OpenWeather also add a fourth
  box to main screen tap-cycle, split in two: UV index (left) and air quality (right).
- **Auto-cycle pages every 5s** — checkbox; when on, main screen page (daily/hourly/moon-phase/AQI)
  auto-advance every 5s instead of needing tap. Applies right away, no reboot.
- **Screen-burn protection idle timeout** — dropdown, 10–30 min in 5-min step, default 10. Backlight
  dim after this many minute idle. Applies right away, no reboot.

Saving NTP server, language, location, or weather provider reboot device to apply change. Clock format,
temperature unit, auto-cycle, and idle timeout apply without reboot.

### OTA firmware update

Web config page root also show firmware status: up-to-date, update-available (with download link for
`firmware.bin`), or check-failed. To update: download `firmware.bin`, then upload it via the form on
same page — device flash it and reboot into new version. Failed upload/verify leave running firmware
untouched.

## Screen behavior / idle dimming

To reduce burn-in risk on the touchscreen, backlight auto-dim after idle timeout (10 min default,
adjustable 10–30 min via web config page — see above). Touching screen restore previous brightness.
Backlight also force back on every 30 minutes regardless (a workaround for ESP32 deep-sleep otherwise
leaving touch unresponsive) — this is expected, not a bug.

Device also check for new firmware at boot and every 6h — see **Main screen** above for what you'll see.

## Troubleshooting

- **Stuck on captive-portal / no weather showing**: reconnect to `Aurigae` Wifi network, redo Wi-Fi
  setup (see above).
- **Wrong location / weather**: open Settings → Change Location (or use web config page) and search
  again.
- **Screen unresponsive after long idle**: wait — force-wakeup timer restore backlight within 30 min, or
  power-cycle device.
- **Need to switch networks**: Settings → Reset Wifi → confirm → device reboot into setup portal.
