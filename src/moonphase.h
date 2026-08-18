#pragma once

// Polls api.aurigae.fizban.net for today's moon phase and updates the
// moon-phase box icon + label. Polled on boot and every UPDATE_INTERVAL.
void fetch_and_update_moonphase();
