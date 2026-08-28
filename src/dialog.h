#pragma once

#include <cstdint>

// Generic modal message-box dialog (single OK button), used e.g. to warn
// about forecasted heavy rain from choose_image().
// If auto_close_ms is nonzero, the dialog also closes itself after that many
// milliseconds even without a tap (used for the firmware-update notice).
void showDialog(const char* message, uint32_t auto_close_ms = 0);
