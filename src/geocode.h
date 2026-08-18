#pragma once

// Queries Open-Meteo's geocoding API for `q`, fills geoResults, and refreshes
// the location-search results dropdown. Used by the on-device location
// dialog's keyboard "search" action.
void do_geocode_query(const char *q);
