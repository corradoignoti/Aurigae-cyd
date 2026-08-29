#include "webconfig.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include <Update.h>
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

// Dark "developer console" shared stylesheet for every webconfig page:
// permanently-dark surfaces, monospace-leaning type, bordered panels, and a
// switch-styled checkbox. Reused across the root/error/info/reboot pages so
// the style lives in one place instead of five near-duplicate <style>
// blocks. Deliberately dark-only (no prefers-color-scheme fallback) to read
// as a console rather than a themed app.
static const char WEBCONFIG_HEAD_CSS[] =
  "<style>"
  ":root{--md-primary:#7c9eff;--md-on-primary:#0d1117;--md-surface:#161b22;--md-on-surface:#e6edf3;--md-outline:#30363d;--md-surface-variant:#21262d}"
  "*{box-sizing:border-box}"
  "body{font-family:ui-monospace,'SF Mono','Segoe UI Mono',Consolas,monospace;background:#0d1117;margin:0;padding:2em 1em;color:var(--md-on-surface)}"
  ".card{background:var(--md-surface);border:1px solid var(--md-outline);max-width:480px;margin:0 auto;padding:1.6em;border-radius:10px;box-shadow:0 4px 18px rgba(0,0,0,.35)}"
  ".head-row{display:flex;align-items:baseline;justify-content:space-between;gap:.8em;flex-wrap:wrap}"
  "h1{font-family:-apple-system,'Segoe UI',sans-serif;font-size:1.4em;margin:0 0 .1em;color:#fff}"
  ".version{color:var(--md-outline);font-size:.8em;margin:0 0 1.2em}"
  ".badge{font-family:-apple-system,'Segoe UI',sans-serif;font-size:.72em;font-weight:700;padding:.3em .7em;border-radius:20px;border:1px solid;white-space:nowrap}"
  ".badge-neutral{color:#8b949e;border-color:var(--md-outline)}"
  ".badge-ok{color:#3fb950;border-color:#26382b;background:#132118}"
  ".badge-warn{color:#e3b341;border-color:#3d3320;background:#211c10}"
  "h2.section{font-family:-apple-system,'Segoe UI',sans-serif;font-size:.78em;text-transform:uppercase;letter-spacing:.07em;color:var(--md-primary);margin:1.6em 0 .7em;font-weight:700;padding-bottom:.5em;border-bottom:1px solid var(--md-outline)}"
  "label{display:block;font-family:-apple-system,'Segoe UI',sans-serif;font-size:.85em;color:#8b949e;margin:1em 0 .3em}"
  "input[type=text],select{width:100%;padding:.65em .8em;border:1px solid var(--md-outline);border-radius:6px;font-size:.95em;background:#0d1117;color:var(--md-on-surface);font-family:inherit}"
  "input[type=text]:focus,select:focus{outline:none;border:2px solid var(--md-primary);padding:calc(.65em - 1px) calc(.8em - 1px)}"
  "button{font-family:-apple-system,'Segoe UI',sans-serif;cursor:pointer}"
  ".btn{margin-top:1.3em;padding:.6em 1.4em;background:var(--md-primary);color:var(--md-on-primary);border:none;border-radius:6px;font-size:.85em;font-weight:700}"
  ".btn:hover{filter:brightness(1.1)}"
  ".btn-outline{background:transparent;color:var(--md-primary);border:1px solid var(--md-outline)}"
  ".switch-row{display:flex;align-items:center;justify-content:space-between;gap:1em;margin-top:1.1em;cursor:pointer;font-family:-apple-system,'Segoe UI',sans-serif;font-size:.9em}"
  "input[type=checkbox]{appearance:none;-webkit-appearance:none;width:40px;height:22px;min-width:40px;border-radius:11px;background:var(--md-outline);position:relative;cursor:pointer;transition:background .2s;margin:0}"
  "input[type=checkbox]:checked{background:var(--md-primary)}"
  "input[type=checkbox]::before{content:'';position:absolute;top:2px;left:2px;width:18px;height:18px;border-radius:50%;background:#0d1117;transition:transform .2s;box-shadow:0 1px 2px rgba(0,0,0,.5)}"
  "input[type=checkbox]:checked::before{transform:translateX(18px)}"
  "hr{border:none;border-top:1px dashed var(--md-outline);margin:1.6em 0}"
  ".theme-picker{display:flex;gap:1.4em;flex-wrap:wrap;margin-top:.8em}"
  ".theme-option{display:flex;flex-direction:column;align-items:center;gap:.4em;cursor:pointer}"
  ".theme-option input{position:absolute;opacity:0;pointer-events:none}"
  ".theme-option .swatch{display:block;width:40px;height:40px;border-radius:50%;border:3px solid transparent;box-shadow:0 1px 3px rgba(0,0,0,.5);transition:border-color .15s,transform .15s}"
  ".theme-option input:checked+.swatch{border-color:var(--md-primary);transform:scale(1.08)}"
  ".theme-option .name{font-family:-apple-system,'Segoe UI',sans-serif;font-size:.78em;color:#8b949e}"
  ".map-frame{width:100%;height:220px;border:1px solid var(--md-outline);border-radius:6px;margin-top:1em;display:block;filter:invert(1) hue-rotate(180deg)}"
  "#locresults button{display:block;width:100%;text-align:left;margin:.4em 0;padding:.65em .9em;background:var(--md-surface-variant);color:var(--md-on-surface);border:1px solid var(--md-outline);border-radius:6px;font-size:.9em;font-family:inherit}"
  "#locresults button:hover{border-color:var(--md-primary)}"
  "a.link{display:inline-block;margin-top:.3em;color:var(--md-primary);text-decoration:none;font-weight:600}"
  "a.link:hover{text-decoration:underline}"
  ".hint{color:#8b949e;font-family:-apple-system,'Segoe UI',sans-serif;font-size:.82em;margin-top:.3em}"
  "</style>";

// Wraps body markup in the shared <head>/card shell every webconfig page uses.
static String webconfig_page_shell(const String &bodyInner) {
  return "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Aurigae Config</title>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    + String(WEBCONFIG_HEAD_CSS) +
    "</head><body><div class='card'>" + bodyInner + "</div></body></html>";
}

// Shared body for the small "message + Back link" pages (error and info).
static String webconfig_message_body(const String &message) {
  return "<h1>Aurigae</h1><p>" + webconfig_html_escape(message) + "</p>"
    "<a class='link' href='/'>Back</a>";
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
  webConfigServer.send(400, "text/html; charset=utf-8", webconfig_page_shell(webconfig_message_body(message)));
}

// 200 OK version of the message page, used by the small "X saved." confirmations.
static void webconfig_send_info_page(const String &message) {
  webConfigServer.send(200, "text/html; charset=utf-8", webconfig_page_shell(webconfig_message_body(message)));
}

static void webconfig_send_reboot_page(const String &message) {
  String body = "<h1>Aurigae</h1><p>" + webconfig_html_escape(message) + "</p>"
    "<p class='hint'>The browser will automatically try to reconnect in <span id='cnt'>30</span>s.</p>"
    "<script>"
    "var s=30;"
    "var t=setInterval(function(){"
    "s--;document.getElementById('cnt').textContent=s;"
    "if(s<=0){clearInterval(t);window.location.href='/';}"
    "},1000);"
    "</script>";
  webConfigServer.send(200, "text/html; charset=utf-8", webconfig_page_shell(body));

  webConfigServer.client().flush();
  delay(500);
  esp_restart();
}

// <option> list for the antiburn-timeout <select>, 5-minute steps between
// ANTIBURN_IDLE_TIMEOUT_MIN_MIN and ANTIBURN_IDLE_TIMEOUT_MAX_MIN, current value selected.
static String webconfig_antiburn_options() {
  uint32_t currentMin = antiburn_idle_timeout_ms / 60000UL;
  String opts;
  for (int m = ANTIBURN_IDLE_TIMEOUT_MIN_MIN; m <= ANTIBURN_IDLE_TIMEOUT_MAX_MIN; m += 5) {
    opts += "<option value='" + String(m) + "'" + String((uint32_t)m == currentMin ? " selected" : "") + ">"
      + String(m) + "</option>";
  }
  return opts;
}

static void handle_webconfig_root() {
  String body = "<div class='head-row'><h1>Aurigae</h1>"
    "<span class='badge badge-neutral' id='verbadge'>Checking&hellip;</span></div>"
    "<p class='version'>" APP_VERSION "</p>"
    "<h2 class='section'>General</h2>"
    "<form method='POST' action='/save'>"
    "<label for='ntp'>NTP Server</label>"
    "<input type='text' id='ntp' name='ntp' value='" + webconfig_html_escape(ntp_server) + "' maxlength='63' required>"
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<form method='POST' action='/saveclockformat'>"
    "<label for='clk24' class='switch-row'><span>Use 24-hour clock</span>"
    "<input type='checkbox' id='clk24' name='clk24' value='1'"
      + String(use_24_hour ? " checked" : "") + "></label>"
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<form method='POST' action='/savetempunit'>"
    "<label for='fahrenheit' class='switch-row'><span>Show temperature in &deg;F (unchecked = &deg;C)</span>"
    "<input type='checkbox' id='fahrenheit' name='fahrenheit' value='1'"
      + String(use_fahrenheit ? " checked" : "") + "></label>"
    "<button type='submit' class='btn'>Save</button>"
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
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<form method='POST' action='/saveslideshow'>"
    "<label for='slideshow' class='switch-row'><span>Auto-cycle pages every 5s</span>"
    "<input type='checkbox' id='slideshow' name='slideshow' value='1'"
      + String(page_slideshow_enabled ? " checked" : "") + "></label>"
    "<p class='hint'>Automatically rotates through daily forecast, hourly forecast, moon phase, "
      "and UV/air quality every 5 seconds instead of waiting for a tap.</p>"
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<form method='POST' action='/saveantiburn'>"
    "<label for='antiburn'>Screen-burn protection: dim backlight after idle (minutes)</label>"
    "<select id='antiburn' name='antiburnMin'>" + webconfig_antiburn_options() + "</select>"
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<hr>"
    "<h2 class='section'>Appearance</h2>"
    "<form method='POST' action='/savetheme'>"
    "<label>Color theme</label>"
    "<div class='theme-picker'>"
    "<label class='theme-option'>"
    "<input type='radio' name='theme' value='0'" + String(current_theme == THEME_BLUE ? " checked" : "") + ">"
    "<span class='swatch' style='background:#4c8cb9'></span><span class='name'>Blue</span>"
    "</label>"
    "<label class='theme-option'>"
    "<input type='radio' name='theme' value='1'" + String(current_theme == THEME_RED ? " checked" : "") + ">"
    "<span class='swatch' style='background:#b9524c'></span><span class='name'>Red</span>"
    "</label>"
    "<label class='theme-option'>"
    "<input type='radio' name='theme' value='2'" + String(current_theme == THEME_GREEN ? " checked" : "") + ">"
    "<span class='swatch' style='background:#4cb95e'></span><span class='name'>Green</span>"
    "</label>"
    "<label class='theme-option'>"
    "<input type='radio' name='theme' value='3'" + String(current_theme == THEME_DARK ? " checked" : "") + ">"
    "<span class='swatch' style='background:#2b2f38'></span><span class='name'>Dark</span>"
    "</label>"
    "</div>"
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<hr>"
    "<h2 class='section'>Weather Provider</h2>"
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
    "<button type='submit' class='btn'>Save</button>"
    "</form>"
    "<hr>"
    "<h2 class='section'>Location</h2>"
    "<label for='locq'>Search for a city</label>"
    "<input type='text' id='locq' placeholder='e.g. Paris, France'>"
    "<button type='button' class='btn btn-outline' onclick='searchLoc()'>Search</button>"
    "<div id='locresults'></div>"
    "<form method='POST' action='/savelocation'>"
    "<label for='loclat'>Latitude</label>"
    "<input type='text' id='loclat' name='latitude' value='" + webconfig_html_escape(latitude) + "' required>"
    "<label for='loclon'>Longitude</label>"
    "<input type='text' id='loclon' name='longitude' value='" + webconfig_html_escape(longitude) + "' required>"
    "<label for='locname'>Location name</label>"
    "<input type='text' id='locname' name='location' value='" + webconfig_html_escape(location) + "' maxlength='63' required>"
    "<iframe id='locmap' class='map-frame' loading='lazy'></iframe>"
    "<button type='submit' class='btn'>Save Location</button>"
    "</form>"
    "<hr>"
    "<h2 class='section'>Firmware Update</h2>"
    "<p class='hint' id='updatestatus'>Checking for updates&hellip;</p>"
    "<div id='updatebanner' style='display:none;background:var(--md-surface-variant);"
      "padding:.8em 1em;border-radius:8px;margin-bottom:1em'>"
    "<strong>Update available: v<span id='ulatest'></span></strong>"
    "<p class='hint'>You have v<span id='ucurrent'></span>. "
    "<a class='link' id='udownload' href='#'>Download firmware.bin</a>, then upload it below, or "
    "<a class='link' href='https://aurigae.fizban.net/flash.html' target='_blank'>flash it directly in your browser</a>.</p>"
    "</div>"
    "<form method='POST' action='/update' enctype='multipart/form-data'"
      " onsubmit=\"return confirm('Flash this firmware file? The device will reboot when done. Do not power it off during the update.');\">"
    "<label for='fwfile'>Firmware .bin file</label>"
    "<input type='file' id='fwfile' name='firmware' accept='.bin' required>"
    "<button type='submit' class='btn'>Upload &amp; Flash</button>"
    "</form>"
    "<p class='hint'>Build with <code>pio run</code>; upload"
      " <code>.pio/build/esp32-cyd/firmware.bin</code>. Or download the latest"
      " <code>firmware.bin</code> from "
      "<a class='link' href='https://aurigae.fizban.net/firmware.html' target='_blank'>aurigae.fizban.net/firmware.html</a>.</p>"
    "<hr>"
    "<h2 class='section'>Debug</h2>"
    "<a class='link' href='/screenshot' target='_blank'>Download screenshot (BMP)</a>"
    "<script>"
    "function updateMap(lat,lon){"
    "var la=parseFloat(lat),lo=parseFloat(lon);"
    "if(isNaN(la)||isNaN(lo))return;"
    "var d=0.02;"
    "var bbox=(lo-d)+','+(la-d)+','+(lo+d)+','+(la+d);"
    "document.getElementById('locmap').src='https://www.openstreetmap.org/export/embed.html?bbox='+bbox+'&marker='+la+','+lo;"
    "}"
    "updateMap(document.getElementById('loclat').value,document.getElementById('loclon').value);"
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
    "updateMap(item.latitude,item.longitude);"
    "};"
    "box.appendChild(b);"
    "});"
    "}).catch(function(){box.textContent='Search failed.';});"
    "}"
    "fetch('/checkupdate').then(function(r){return r.json();}).then(function(d){"
    "var status=document.getElementById('updatestatus');"
    "var badge=document.getElementById('verbadge');"
    "if(d.error||!d.current){"
    "status.innerHTML=\"Could not check for updates. Compare your version above against "
      "<a class='link' href='https://aurigae.fizban.net/flash.html' target='_blank'>"
      "aurigae.fizban.net/flash.html</a> manually.\";"
    "badge.className='badge badge-neutral';badge.textContent='Check failed';"
    "return;"
    "}"
    "if(d.updateAvailable){"
    "status.textContent='Update available: v'+d.latest+' (you have v'+d.current+').';"
    "document.getElementById('ulatest').textContent=d.latest;"
    "document.getElementById('ucurrent').textContent=d.current;"
    "document.getElementById('udownload').href=d.url;"
    "document.getElementById('updatebanner').style.display='block';"
    "badge.className='badge badge-warn';badge.textContent='Update available';"
    "}else{"
    "status.textContent='Firmware is up to date (v'+d.current+').';"
    "badge.className='badge badge-ok';badge.textContent='Up to date';"
    "}"
    "}).catch(function(){"
    "document.getElementById('updatestatus').innerHTML=\"Could not check for updates. Compare your "
      "version above against <a class='link' href='https://aurigae.fizban.net/flash.html' "
      "target='_blank'>aurigae.fizban.net/flash.html</a> manually.\";"
    "document.getElementById('verbadge').className='badge badge-neutral';"
    "document.getElementById('verbadge').textContent='Check failed';"
    "});"
    "</script>";
  webConfigServer.send(200, "text/html; charset=utf-8", webconfig_page_shell(body));
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

  webconfig_send_info_page("Clock format saved.");
}

static void handle_webconfig_savetempunit() {
  use_fahrenheit = webConfigServer.hasArg("fahrenheit");
  prefs.putBool("useFahrenheit", use_fahrenheit);

  webconfig_send_info_page("Temperature unit saved.");
}

static void handle_webconfig_saveslideshow() {
  page_slideshow_enabled = webConfigServer.hasArg("slideshow");
  prefs.putBool("pageSlideshow", page_slideshow_enabled);

  webconfig_send_info_page("Page slideshow setting saved.");
}

static void handle_webconfig_saveantiburn() {
  String val = webConfigServer.hasArg("antiburnMin") ? webConfigServer.arg("antiburnMin") : "";
  int minutes = val.toInt();

  if (val.length() == 0 || minutes < ANTIBURN_IDLE_TIMEOUT_MIN_MIN || minutes > ANTIBURN_IDLE_TIMEOUT_MAX_MIN) {
    webconfig_send_error_page("Invalid screen-burn protection timeout. Nothing saved.");
    return;
  }

  antiburn_idle_timeout_ms = (uint32_t)minutes * 60000UL;
  prefs.putUInt("antiburnMin", (uint32_t)minutes);

  webconfig_send_info_page("Screen-burn protection timeout saved.");
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
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      JsonArray results = doc["results"].as<JsonArray>();
      JsonDocument outDoc;
      JsonArray arr = outDoc.to<JsonArray>();
      for (JsonObject item : results) {
        JsonObject o = arr.add<JsonObject>();
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

// Streaming upload callback for /update: fed one HTTPUpload chunk at a time
// as the multipart body arrives, writing each chunk straight into the
// inactive OTA slot via the Update library. UPDATE_SIZE_UNKNOWN is used
// because WebServer doesn't expose the multipart part's declared size here;
// Update tracks the free OTA slot size itself and fails the write once it's
// exceeded. Errors are only logged here — the outcome is reported to the
// browser afterwards by handle_webconfig_update_done, once the upload (and
// this callback) has fully finished.
static void handle_webconfig_update_upload() {
  HTTPUpload &upload = webConfigServer.upload();

  switch (upload.status) {
    case UPLOAD_FILE_START:
      Serial.printf("OTA update starting: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
      break;
    case UPLOAD_FILE_WRITE:
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
      break;
    case UPLOAD_FILE_END:
      if (Update.end(true)) {
        Serial.printf("OTA update received: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      break;
    case UPLOAD_FILE_ABORTED:
      Update.abort();
      Serial.println("OTA update aborted.");
      break;
  }
}

// Fires once the /update request (and handle_webconfig_update_upload above)
// has fully completed. Update.hasError() reflects the outcome of the whole
// begin/write/end sequence, including a bad or truncated .bin — in that case
// the previous firmware is untouched and still boots normally.
static void handle_webconfig_update_done() {
  if (Update.hasError()) {
    webconfig_send_error_page("Firmware update failed. Nothing was flashed; device is unchanged.");
    return;
  }

  webconfig_send_reboot_page("Firmware updated. Rebooting device...");
}

// Strips any non-numeric prefix off APP_VERSION (e.g. "App ver. 2.1.0" -> "2.1.0")
// so it can be compared against the manifest's bare "X.Y.Z" with version_is_newer().
static String webconfig_current_version() {
  String v(APP_VERSION);
  size_t i = 0;
  while (i < v.length() && !isDigit(v[i])) i++;
  return v.substring(i);
}

// Polls FIRMWARE_MANIFEST_URL (the same esp-web-tools manifest the public web
// flasher at aurigae.fizban.net/flash.html reads) and reports whether it
// advertises a firmware newer than APP_VERSION, plus a direct download link
// for the app image, for the update-status line on the root page.
// `current` is always included so the page can display it even when the
// check itself fails; `error` is set on any network/parse failure so the
// page can tell "checked, up to date" apart from "couldn't check" and point
// the user at the flash.html page to compare versions manually.
static void handle_webconfig_checkupdate() {
  String current = webconfig_current_version();
  bool ok = false;
  String latest, fwUrl;

  HTTPClient http;
  http.begin(FIRMWARE_MANIFEST_URL);
  int code = http.GET();
  if (code == HTTP_CODE_OK) {
    JsonDocument doc;
    if (!deserializeJson(doc, http.getString())) {
      latest = doc["version"] | "";
      if (latest.length() > 0) {
        // Find the app-image part (offset 0x10000, same as this project's own
        // OTA slot offset) so the download link matches whatever the
        // manifest's build actually names it, rather than assuming "firmware.bin".
        String fwPath = "firmware.bin";
        JsonArray builds = doc["builds"].as<JsonArray>();
        if (builds.size() > 0) {
          JsonArray parts = builds[0]["parts"].as<JsonArray>();
          for (JsonObject part : parts) {
            long offset = part["offset"] | -1;
            if (offset == 0x10000) {
              fwPath = part["path"] | "firmware.bin";
              break;
            }
          }
        }

        String base = FIRMWARE_MANIFEST_URL;
        fwUrl = base.substring(0, base.lastIndexOf('/') + 1) + fwPath;
        ok = true;
      }
    }
  }
  http.end();

  JsonDocument outDoc;
  outDoc["current"] = current;
  outDoc["error"] = !ok;
  if (ok) {
    outDoc["latest"] = latest;
    outDoc["updateAvailable"] = version_is_newer(latest, current);
    outDoc["url"] = fwUrl;
  } else {
    outDoc["updateAvailable"] = false;
  }

  String out;
  serializeJson(outDoc, out);
  webConfigServer.send(200, "application/json; charset=utf-8", out);
}

void start_webconfig_server() {
  webConfigServer.on("/", HTTP_GET, handle_webconfig_root);
  webConfigServer.on("/save", HTTP_POST, handle_webconfig_save);
  webConfigServer.on("/saveclockformat", HTTP_POST, handle_webconfig_saveclockformat);
  webConfigServer.on("/savetempunit", HTTP_POST, handle_webconfig_savetempunit);
  webConfigServer.on("/saveslideshow", HTTP_POST, handle_webconfig_saveslideshow);
  webConfigServer.on("/saveantiburn", HTTP_POST, handle_webconfig_saveantiburn);
  webConfigServer.on("/savelanguage", HTTP_POST, handle_webconfig_savelanguage);
  webConfigServer.on("/savetheme", HTTP_POST, handle_webconfig_savetheme);
  webConfigServer.on("/saveweatherprovider", HTTP_POST, handle_webconfig_saveweatherprovider);
  webConfigServer.on("/geocode", HTTP_GET, handle_webconfig_geocode);
  webConfigServer.on("/savelocation", HTTP_POST, handle_webconfig_savelocation);
  webConfigServer.on("/screenshot", HTTP_GET, handle_webconfig_screenshot);
  webConfigServer.on("/update", HTTP_POST, handle_webconfig_update_done, handle_webconfig_update_upload);
  webConfigServer.on("/checkupdate", HTTP_GET, handle_webconfig_checkupdate);
  webConfigServer.begin();
}
