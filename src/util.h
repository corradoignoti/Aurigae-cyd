#pragma once

#include <Arduino.h>

// Formats an hour (0-23) per the current 12/24-hour + locale settings.
String hour_of_day(int hour);

// Percent-encodes a string per RFC 3986 unreserved characters.
String urlencode(const String &str);

// Converts "h:mm:ss AM/PM" to rounded 24h "HH:MM".
String convertTo24hAndRound(const char* time12h);

// Rounds a "H:MM[:SS[ AM/PM]]" time string to the nearest minute.
String roundToMinute(const char* time);
