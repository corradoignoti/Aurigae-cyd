#include "util.h"
#include "localization.h"
#include "globals.h"

String hour_of_day(int hour) {
  const LocalizedStrings* strings = get_strings();
  if(hour < 0 || hour > 23) return String(strings->invalid_hour);

  if (use_24_hour) {
    if (hour < 10)
      return String("0") + String(hour);
    else
      return String(hour);
  } else {
    if(hour == 0)   return String("12") + strings->am;
    if(hour == 12)  return String(strings->noon);

    bool isMorning = (hour < 12);
    String suffix = isMorning ? strings->am : strings->pm;

    int displayHour = hour % 12;

    return String(displayHour) + suffix;
  }
}

String urlencode(const String &str) {
  String encoded = "";
  char buf[5];
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    // Unreserved characters according to RFC 3986
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else {
      // Percent-encode others
      sprintf(buf, "%%%02X", (unsigned char)c);
      encoded += buf;
    }
  }
  return encoded;
}

// Converts from 12h to 24h and round to the minute
String convertTo24hAndRound(const char* time12h) {
  int hour, minute, second;
  char ampm[3]; // "AM" o "PM"

  sscanf(time12h, "%d:%d:%d %2s", &hour, &minute, &second, ampm);

  if (strcmp(ampm, "AM") == 0) {
    if (hour == 12) hour = 0;  // 12 AM -> 00
  } else { // PM
    if (hour != 12) hour += 12; // 1 PM -> 13 ... 11 PM -> 23
  }

  // Round the seconds
  if (second >= 30) {
    minute++;
    if (minute == 60) {
      minute = 0;
      hour++;
      if (hour == 24) hour = 0;
    }
  }

  char buffer[6];
  sprintf(buffer, "%02d:%02d", hour, minute); // HH:MM

  return String(buffer);
}

String roundToMinute(const char* time) {
  int hour, minute, second = 0;
  char ampm[3] = "";

  int fields = sscanf(time, "%d:%d:%d %2s", &hour, &minute, &second, ampm);

  if (fields < 2) return String("Invalid");
  if (fields == 2) sscanf(time, "%d:%d", &hour, &minute);

  if (second >= 30) {
    minute++;
    if (minute == 60) {
      minute = 0;
      hour++;
      if (ampm[0]) { // formato 12h
        if (hour == 12) {
          if (strcmp(ampm, "AM") == 0) strcpy(ampm, "PM");
          else strcpy(ampm, "AM");
        } else if (hour > 12) {
          hour = 1;
        }
      } else { // formato 24h
        if (hour == 24) hour = 0;
      }
    }
  }

  char buffer[12];
  if (ampm[0])
    sprintf(buffer, "%02d:%02d %s", hour, minute, ampm);
  else
    sprintf(buffer, "%02d:%02d", hour, minute);

  return String(buffer);
}

bool version_is_newer(const String &a, const String &b) {
  int ai = 0, bi = 0;
  while (ai < (int)a.length() && !isDigit(a[ai])) ai++;
  while (bi < (int)b.length() && !isDigit(b[bi])) bi++;

  while (ai < (int)a.length() || bi < (int)b.length()) {
    int an = 0, bn = 0;
    while (ai < (int)a.length() && isDigit(a[ai])) { an = an * 10 + (a[ai] - '0'); ai++; }
    while (bi < (int)b.length() && isDigit(b[bi])) { bn = bn * 10 + (b[bi] - '0'); bi++; }
    if (an != bn) return an > bn;
    while (ai < (int)a.length() && a[ai] != '.') ai++;
    while (bi < (int)b.length() && b[bi] != '.') bi++;
    if (ai < (int)a.length()) ai++;
    if (bi < (int)b.length()) bi++;
  }
  return false;
}
