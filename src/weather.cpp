#include "weather.h"

WeatherProvider weather_provider = PROVIDER_OPEN_METEO;
char openweather_apikey[40] = "";

void fetch_and_update_weather() {
  if (weather_provider == PROVIDER_OPENWEATHER) {
    fetch_weather_openweather();
    fetch_openweather_uv_and_air_quality();
  } else {
    fetch_weather_openmeteo();
    fetch_openmeteo_air_quality();
  }
}
