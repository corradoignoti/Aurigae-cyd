#pragma once

#include <lvgl.h>

// Polls FIRMWARE_MANIFEST_URL (config.h) and compares its "version" against
// APP_VERSION. On finding a newer firmware: wakes the screen if antiburn had
// dimmed it, shows a self-closing dialog (see FIRMWARE_UPDATE_DIALOG_MS in
// firmware_update.cpp), and shows the top-bar download icon
// (update_firmware_update_icon() in ui_main.h). The icon clears itself again
// on the next check once this device's version is no longer older (e.g.
// after the user OTA-updates via the webconfig page).
// `timer` is unused; the default lets it double as both a direct call (boot)
// and an lv_timer_create() periodic callback (every 6h).
void check_for_firmware_update(lv_timer_t *timer = NULL);
