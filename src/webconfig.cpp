#include "webconfig.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "config.h"
#include "globals.h"
#include "localization.h"
#include "weather.h"
#include "theme.h"
#include "util.h"

// Escape a value for safe embedding inside an HTML attribute/text.
static String webconfig_html_escape(const String &s) {
  String out;
  out.reserve(s.length());
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += c;
    }
  }
  return out;
}

// A numeric coordinate string: optional leading '-', digits, at most one '.'.
static bool webconfig_is_valid_coord(const String &s) {
  if (s.length() == 0) return false;
  bool dot = false;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '-' && i == 0) continue;
    if (c == '.' && !dot) { dot = true; continue; }
    if (!isDigit(c)) return false;
  }
  return true;
}

static void webconfig_send_error_page(const String &message) {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aurigae Config</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:2em;max-width:400px}"
    "a{display:inline-block;margin-top:1em}</style></head><body>"
    "<h2>Aurigae</h2><p>" + webconfig_html_escape(message) + "</p>"
    "<a href='/'>Back</a></body></html>";
  webConfigServer.send(400, "text/html; charset=utf-8", html);
}

static void webconfig_send_reboot_page(const String &message) {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aurigae Config</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:2em;max-width:400px}</style></head><body>"
    "<h2>Aurigae</h2><p>" + webconfig_html_escape(message) + "</p>"
    "<p>The browser will automatically try to reconnect in <span id='cnt'>30</span>s.</p>"
    "<script>"
    "var s=30;"
    "var t=setInterval(function(){"
    "s--;document.getElementById('cnt').textContent=s;"
    "if(s<=0){clearInterval(t);window.location.href='/';}"
    "},1000);"
    "</script></body></html>";
  webConfigServer.send(200, "text/html; charset=utf-8", html);

  webConfigServer.client().flush();
  delay(500);
  esp_restart();
}

static void handle_webconfig_root() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aurigae Config</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:2em;max-width:400px}"
    "label{display:block;margin-top:1em;font-weight:bold}"
    "input,select{padding:.5em;width:100%;box-sizing:border-box;margin-top:.3em}"
    "button{margin-top:1.5em;padding:.6em 1.5em}"
    "h2{margin-bottom:0}"
    "hr{margin:2em 0}"
    "#locresults button{display:block;width:100%;text-align:left;margin:.3em 0;padding:.4em}"
    "</style></head><body>"
    "<h2>Aurigae</h2><p>" APP_VERSION "</p>"
    "<form method='POST' action='/save'>"
    "<label for='ntp'>NTP Server</label>"
    "<input type='text' id='ntp' name='ntp' value='" + webconfig_html_escape(ntp_server) + "' maxlength='63' required>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<form method='POST' action='/saveclockformat'>"
    "<label for='clk24'><input type='checkbox' id='clk24' name='clk24' value='1' style='width:auto;margin-right:.5em'"
      + String(use_24_hour ? " checked" : "") + ">Use 24-hour clock</label>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<form method='POST' action='/savetempunit'>"
    "<label for='fahrenheit'><input type='checkbox' id='fahrenheit' name='fahrenheit' value='1' style='width:auto;margin-right:.5em'"
      + String(use_fahrenheit ? " checked" : "") + ">Show temperature in °F (unchecked = °C)</label>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<form method='POST' action='/savelanguage'>"
    "<label for='lang'>Language</label>"
    "<select id='lang' name='lang'>"
    "<option value='0'" + String(current_language == LANG_EN ? " selected" : "") + ">English</option>"
    "<option value='1'" + String(current_language == LANG_ES ? " selected" : "") + ">Español</option>"
    "<option value='2'" + String(current_language == LANG_DE ? " selected" : "") + ">Deutsch</option>"
    "<option value='3'" + String(current_language == LANG_FR ? " selected" : "") + ">Français</option>"
    "<option value='4'" + String(current_language == LANG_IT ? " selected" : "") + ">Italiano</option>"
    "</select>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<hr>"
    "<h2>Appearance</h2>"
    "<form method='POST' action='/savetheme'>"
    "<label for='theme'>Color theme</label>"
    "<select id='theme' name='theme'>"
    "<option value='0'" + String(current_theme == THEME_BLUE ? " selected" : "") + ">Blue (default)</option>"
    "<option value='1'" + String(current_theme == THEME_RED ? " selected" : "") + ">Red</option>"
    "<option value='2'" + String(current_theme == THEME_GREEN ? " selected" : "") + ">Green</option>"
    "<option value='3'" + String(current_theme == THEME_DARK ? " selected" : "") + ">Dark</option>"
    "</select>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<hr>"
    "<h2>Weather Provider</h2>"
    "<form method='POST' action='/saveweatherprovider'>"
    "<label for='wprovider'>Provider</label>"
    "<select id='wprovider' name='provider' onchange=\"document.getElementById('owmkeyrow').style.display=(this.value=='1')?'block':'none';\">"
    "<option value='0'" + String(weather_provider == PROVIDER_OPEN_METEO ? " selected" : "") + ">Open-Meteo (default, no key needed)</option>"
    "<option value='1'" + String(weather_provider == PROVIDER_OPENWEATHER ? " selected" : "") + ">OpenWeather (requires API key)</option>"
    "</select>"
    "<div id='owmkeyrow' style='display:" + String(weather_provider == PROVIDER_OPENWEATHER ? "block" : "none") + "'>"
    "<label for='owmapikey'>OpenWeather API Key</label>"
    "<input type='text' id='owmapikey' name='owmapikey' value='" + webconfig_html_escape(openweather_apikey) + "' maxlength='39'>"
    "</div>"
    "<button type='submit'>Save</button>"
    "</form>"
    "<hr>"
    "<h2>Location</h2>"
    "<label for='locq'>Search for a city</label>"
    "<input type='text' id='locq' placeholder='e.g. Paris, France'>"
    "<button type='button' onclick='searchLoc()'>Search</button>"
    "<div id='locresults'></div>"
    "<form method='POST' action='/savelocation'>"
    "<label for='loclat'>Latitude</label>"
    "<input type='text' id='loclat' name='latitude' value='" + webconfig_html_escape(latitude) + "' required>"
    "<label for='loclon'>Longitude</label>"
    "<input type='text' id='loclon' name='longitude' value='" + webconfig_html_escape(longitude) + "' required>"
    "<label for='locname'>Location name</label>"
    "<input type='text' id='locname' name='location' value='" + webconfig_html_escape(location) + "' maxlength='63' required>"
    "<button type='submit'>Save Location</button>"
    "</form>"
    "<hr>"
    "<h2>Debug</h2>"
    "<a href='/screenshot' target='_blank'>Download screenshot (BMP)</a>"
    "<script>"
    "function searchLoc(){"
    "var q=document.getElementById('locq').value;"
    "if(!q)return;"
    "var box=document.getElementById('locresults');"
    "box.textContent='Searching...';"
    "fetch('/geocode?q='+encodeURIComponent(q)).then(function(r){return r.json();}).then(function(list){"
    "box.innerHTML='';"
    "if(list.length===0){box.textContent='No results.';return;}"
    "list.forEach(function(item){"
    "var label=item.name+(item.admin1?', '+item.admin1:'')+(item.country_code?' ('+item.country_code+')':'');"
    "var b=document.createElement('button');"
    "b.type='button';"
    "b.textContent=label;"
    "b.onclick=function(){"
    "document.getElementById('loclat').value=item.latitude;"
    "document.getElementById('loclon').value=item.longitude;"
    "document.getElementById('locname').value=item.name+(item.admin1?', '+item.admin1:'');"
    "};"
    "box.appendChild(b);"
    "});"
    "}).catch(function(){box.textContent='Search failed.';});"
    "}"
    "</script>"
    "</body></html>";
  webConfigServer.send(200, "text/html; charset=utf-8", html);
}

static void handle_webconfig_save() {
  String val = webConfigServer.hasArg("ntp") ? webConfigServer.arg("ntp") : "";
  val.trim();

  if (val.length() == 0 || val.length() >= sizeof(ntp_server)) {
    webconfig_send_error_page("Invalid NTP server. Nothing saved.");
    return;
  }

  val.toCharArray(ntp_server, sizeof(ntp_server));
  prefs.putString("ntpServer", ntp_server);
  configTime(ntp_utc_offset_seconds, 0, ntp_server, "time.nist.gov");

  webconfig_send_reboot_page("Configuration saved. Rebooting device...");
}

static void handle_webconfig_saveclockformat() {
  use_24_hour = webConfigServer.hasArg("clk24");
  prefs.putBool("use24Hour", use_24_hour);

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aurigae Config</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:2em;max-width:400px}"
    "a{display:inline-block;margin-top:1em}</style></head><body>"
    "<h2>Aurigae</h2><p>Clock format saved.</p>"
    "<a href='/'>Back</a></body></html>";
  webConfigServer.send(200, "text/html; charset=utf-8", html);
}

static void handle_webconfig_savetempunit() {
  use_fahrenheit = webConfigServer.hasArg("fahrenheit");
  prefs.putBool("useFahrenheit", use_fahrenheit);

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aurigae Config</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:2em;max-width:400px}"
    "a{display:inline-block;margin-top:1em}</style></head><body>"
    "<h2>Aurigae</h2><p>Temperature unit saved.</p>"
    "<a href='/'>Back</a></body></html>";
  webConfigServer.send(200, "text/html; charset=utf-8", html);
}

static void handle_webconfig_savelanguage() {
  String val = webConfigServer.hasArg("lang") ? webConfigServer.arg("lang") : "";
  int lang = val.toInt();

  if (val.length() == 0 || lang < LANG_EN || lang > LANG_IT) {
    webconfig_send_error_page("Invalid language. Nothing saved.");
    return;
  }

  current_language = (Language)lang;
  prefs.putUInt("language", current_language);

  webconfig_send_reboot_page("Language saved. Rebooting device...");
}

static void handle_webconfig_savetheme() {
  String val = webConfigServer.hasArg("theme") ? webConfigServer.arg("theme") : "";
  int theme = val.toInt();

  if (val.length() == 0 || theme < THEME_BLUE || theme > THEME_DARK) {
    webconfig_send_error_page("Invalid theme. Nothing saved.");
    return;
  }

  current_theme = (UiTheme)theme;
  prefs.putUInt("theme", current_theme);

  webconfig_send_reboot_page("Theme saved. Rebooting device...");
}

static void handle_webconfig_saveweatherprovider() {
  String val = webConfigServer.hasArg("provider") ? webConfigServer.arg("provider") : "";
  int provider = val.toInt();
  String apikey = webConfigServer.hasArg("owmapikey") ? webConfigServer.arg("owmapikey") : "";
  apikey.trim();

  if (val.length() == 0 || provider < PROVIDER_OPEN_METEO || provider > PROVIDER_OPENWEATHER) {
    webconfig_send_error_page("Invalid weather provider. Nothing saved.");
    return;
  }
  if (provider == PROVIDER_OPENWEATHER && (apikey.length() == 0 || apikey.length() >= sizeof(openweather_apikey))) {
    webconfig_send_error_page("OpenWeather requires a valid API key. Nothing saved.");
    return;
  }

  weather_provider = (WeatherProvider)provider;
  prefs.putUInt("weatherProvider", weather_provider);
  apikey.toCharArray(openweather_apikey, sizeof(openweather_apikey));
  prefs.putString("owmApiKey", openweather_apikey);

  webconfig_send_reboot_page("Weather provider saved. Rebooting device...");
}

// Proxies an Open-Meteo geocoding search so the config page can search for a
// city without exposing the device's UI. Returns a JSON array of
// {name, admin1, country_code, latitude, longitude}.
static void handle_webconfig_geocode() {
  String q = webConfigServer.hasArg("q") ? webConfigServer.arg("q") : "";
  q.trim();

  if (q.length() == 0) {
    webConfigServer.send(200, "application/json; charset=utf-8", "[]");
    return;
  }

  String url = String("https://geocoding-api.open-meteo.com/v1/search?name=") + urlencode(q) + "&count=15";
  String out = "[]";

  HTTPClient http;
  http.begin(url);
  if (http.GET() == HTTP_CODE_OK) {
    DynamicJsonDocument doc(8 * 1024);
    if (!deserializeJson(doc, http.getString())) {
      JsonArray results = doc["results"].as<JsonArray>();
      DynamicJsonDocument outDoc(8 * 1024);
      JsonArray arr = outDoc.to<JsonArray>();
      for (JsonObject item : results) {
        JsonObject o = arr.createNestedObject();
        o["name"] = item["name"];
        o["admin1"] = item["admin1"] | "";
        o["country_code"] = item["country_code"] | "";
        o["latitude"] = item["latitude"];
        o["longitude"] = item["longitude"];
      }
      serializeJson(arr, out);
    }
  }
  http.end();

  webConfigServer.send(200, "application/json; charset=utf-8", out);
}

static void handle_webconfig_savelocation() {
  String lat = webConfigServer.hasArg("latitude") ? webConfigServer.arg("latitude") : "";
  String lon = webConfigServer.hasArg("longitude") ? webConfigServer.arg("longitude") : "";
  String loc = webConfigServer.hasArg("location") ? webConfigServer.arg("location") : "";
  lat.trim();
  lon.trim();
  loc.trim();

  bool valid = webconfig_is_valid_coord(lat) && lat.length() < sizeof(latitude)
    && webconfig_is_valid_coord(lon) && lon.length() < sizeof(longitude)
    && loc.length() > 0;

  if (valid) {
    float latf = lat.toFloat();
    float lonf = lon.toFloat();
    valid = (latf >= -90.0f && latf <= 90.0f && lonf >= -180.0f && lonf <= 180.0f);
  }

  if (!valid) {
    webconfig_send_error_page("Invalid location. Nothing saved.");
    return;
  }

  lat.toCharArray(latitude, sizeof(latitude));
  lon.toCharArray(longitude, sizeof(longitude));
  prefs.putString("latitude", latitude);
  prefs.putString("longitude", longitude);
  prefs.putString("location", loc);

  webconfig_send_reboot_page("Location saved. Rebooting device...");
}

// Streams the current display contents as an uncompressed BMP by reading
// pixels back from the ILI9341's GRAM over SPI (readRectRGB) one row at a
// time — SCREEN_WIDTH*SCREEN_HEIGHT is too big to buffer whole in RAM.
// Uses the standalone `tft` instance (see globals.h); safe to call from here
// since loop() is single-threaded, so this never runs concurrently with
// LVGL's own flush_cb on its separate TFT_eSPI instance.
static void handle_webconfig_screenshot() {
  const uint32_t rowBytes = SCREEN_WIDTH * 3;   // 24-bit BGR, already 4-byte aligned
  const uint32_t dataSize = rowBytes * SCREEN_HEIGHT;
  const uint32_t fileSize = 54 + dataSize;
  const int32_t negHeight = -SCREEN_HEIGHT;     // negative = top-down row order

  uint8_t header[54] = {0};
  header[0] = 'B'; header[1] = 'M';
  header[2] = fileSize & 0xFF; header[3] = (fileSize >> 8) & 0xFF;
  header[4] = (fileSize >> 16) & 0xFF; header[5] = (fileSize >> 24) & 0xFF;
  header[10] = 54;                              // pixel data offset
  header[14] = 40;                              // DIB header size (BITMAPINFOHEADER)
  header[18] = SCREEN_WIDTH & 0xFF; header[19] = (SCREEN_WIDTH >> 8) & 0xFF;
  header[22] = negHeight & 0xFF; header[23] = (negHeight >> 8) & 0xFF;
  header[24] = (negHeight >> 16) & 0xFF; header[25] = (negHeight >> 24) & 0xFF;
  header[26] = 1;                               // planes
  header[28] = 24;                              // bits per pixel
  header[34] = dataSize & 0xFF; header[35] = (dataSize >> 8) & 0xFF;
  header[36] = (dataSize >> 16) & 0xFF; header[37] = (dataSize >> 24) & 0xFF;

  webConfigServer.setContentLength(fileSize);
  webConfigServer.send(200, "image/bmp", "");
  webConfigServer.sendContent((const char *)header, sizeof(header));

  uint8_t rgbRow[SCREEN_WIDTH * 3];  // readRectRGB fills R,G,B per pixel
  uint8_t bmpRow[SCREEN_WIDTH * 3];  // BMP wants B,G,R per pixel
  for (int row = 0; row < SCREEN_HEIGHT; row++) {
    tft.readRectRGB(0, row, SCREEN_WIDTH, 1, rgbRow);
    for (int col = 0; col < SCREEN_WIDTH; col++) {
      bmpRow[col * 3 + 0] = rgbRow[col * 3 + 2];  // B
      bmpRow[col * 3 + 1] = rgbRow[col * 3 + 1];  // G
      bmpRow[col * 3 + 2] = rgbRow[col * 3 + 0];  // R
    }
    webConfigServer.sendContent((const char *)bmpRow, sizeof(bmpRow));
  }
}

void start_webconfig_server() {
  webConfigServer.on("/", HTTP_GET, handle_webconfig_root);
  webConfigServer.on("/save", HTTP_POST, handle_webconfig_save);
  webConfigServer.on("/saveclockformat", HTTP_POST, handle_webconfig_saveclockformat);
  webConfigServer.on("/savetempunit", HTTP_POST, handle_webconfig_savetempunit);
  webConfigServer.on("/savelanguage", HTTP_POST, handle_webconfig_savelanguage);
  webConfigServer.on("/savetheme", HTTP_POST, handle_webconfig_savetheme);
  webConfigServer.on("/saveweatherprovider", HTTP_POST, handle_webconfig_saveweatherprovider);
  webConfigServer.on("/geocode", HTTP_GET, handle_webconfig_geocode);
  webConfigServer.on("/savelocation", HTTP_POST, handle_webconfig_savelocation);
  webConfigServer.on("/screenshot", HTTP_GET, handle_webconfig_screenshot);
  webConfigServer.begin();
}
