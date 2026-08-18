#pragma once

#include <lvgl.h>

// Opens the "Change Location" window (city search + save/cancel). Registered
// as the settings window's "Change Location" button handler.
void change_location_event_cb(lv_event_t *e);

void create_location_dialog();

// Rebuilds the location-search results dropdown from geoResults; called by
// do_geocode_query() after a search completes.
void populate_results_dropdown();

// On-screen keyboard READY/CANCEL handler, shared with the location dialog's
// city textarea; registered on `kb` itself in create_settings_window().
void kb_event_cb(lv_event_t *e);
