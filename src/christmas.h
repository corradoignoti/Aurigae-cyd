#pragma once

// Gregorian day-of-week (0=Sun) via Zeller-ish table; also used by the
// weather daily-forecast rows to label each day.
int day_of_week(int y, int m, int d);

// Days remaining until the next Dec 25, or -1 if local time isn't available.
int days_to_christmas(void);

// Refreshes days_to_xmas and the Santa/"Merry Christmas"/"Happy New Year" UI.
void update_days_to_christmas();
