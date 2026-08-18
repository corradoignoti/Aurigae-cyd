#pragma once

class WiFiManager;

// WiFiManager AP-mode callback: shows the "connect to configure" splash
// while the captive AP is up.
void apModeCallback(WiFiManager *mgr);
