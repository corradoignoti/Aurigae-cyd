#pragma once

// Local HTTP config server (port 80): lets a browser on the same network set
// the NTP server, clock format, temperature unit, language, color theme,
// weather provider (+ OpenWeather API key), and location without touching
// the touchscreen UI. Also serves /screenshot, a BMP dump of the current
// display contents read back over SPI, for remote debugging.
void start_webconfig_server();
