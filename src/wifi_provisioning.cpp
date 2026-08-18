#include "wifi_provisioning.h"
#include <Arduino.h>
#include <WiFiManager.h>
#include <lvgl.h>
#include "ui_main.h"

static void flush_wifi_splashscreen(uint32_t ms = 200) {
  uint32_t start = millis();
  while (millis() - start < ms) {
    lv_timer_handler();
    delay(5);
  }
}

void apModeCallback(WiFiManager *mgr) {
  wifi_splash_screen();
  flush_wifi_splashscreen();
}
