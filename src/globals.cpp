#include "globals.h"

TFT_eSPI tft = TFT_eSPI();

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
int x, y, z;

Preferences prefs;
bool use_fahrenheit = false;
bool use_24_hour = false;
char latitude[16] = LATITUDE_DEFAULT;
char longitude[16] = LONGITUDE_DEFAULT;
String location = String(LOCATION_DEFAULT);
DynamicJsonDocument geoDoc(8 * 1024);
JsonArray geoResults;
char ntp_server[64] = NTP_SERVER_DEFAULT;
int ntp_utc_offset_seconds = 0;

WebServer webConfigServer(80);

lv_obj_t *lbl_today_temp;
lv_obj_t *lbl_today_feels_like;
lv_obj_t *img_today_icon;
lv_obj_t *lbl_uv_value;
lv_obj_t *lbl_uv_desc;
lv_obj_t *lbl_aqi_value;
lv_obj_t *lbl_aqi_desc;
lv_obj_t *lbl_daily_day[7];
lv_obj_t *lbl_daily_high[7];
lv_obj_t *lbl_daily_low[7];
lv_obj_t *img_daily[7];
lv_obj_t *lbl_hourly[7];
lv_obj_t *lbl_precipitation_probability[7];
lv_obj_t *lbl_hourly_temp[7];
lv_obj_t *img_hourly[7];
lv_obj_t *lbl_loc;
lv_obj_t *kb;
lv_obj_t *lbl_sunrise;
lv_obj_t *lbl_sunset;
lv_obj_t *img_moon_phases;
lv_obj_t *lbl_moon_phase_desc;
lv_obj_t *img_santa_claus;
lv_obj_t *img_small_santa_claus;
lv_obj_t *lbl_santa_desc;
lv_obj_t *lbl_relative_hum;
lv_obj_t *lbl_merry_christmas;
lv_obj_t *lbl_clock;

int days_to_xmas;
uint32_t default_lcd_brightness;
volatile unsigned long last_touch_ms = 0;
volatile bool screen_dimmed = false;
