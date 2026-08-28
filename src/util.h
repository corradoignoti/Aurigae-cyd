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

// Compares two dotted "X.Y.Z..." version strings component-by-component
// (missing trailing components treated as 0; non-numeric text before the
// first digit, e.g. "App ver. ", is ignored). Returns true if `a` is
// strictly newer than `b`.
bool version_is_newer(const String &a, const String &b);
