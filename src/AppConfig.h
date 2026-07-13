#pragma once

/*
 * AppConfig — Runtime configuration stored in NVS (Non-Volatile Storage).
 *
 * On boot, load() reads saved values from NVS. Any value not found in NVS
 * falls back to the compile-time defaults from include/config.h.
 *
 * The WebAdmin captive portal writes to NVS via save(). After saving, the
 * device reboots and the new values take effect.
 *
 * NVS namespace: "dxcluster"
 */

#include <Arduino.h>

struct AppConfig {
  String wifiSsid;
  String wifiPassword;
  String clusterHost;
  uint16_t clusterPort = 0;
  String callsign;
  String callsignPassword;   // sent when the cluster asks "Password:"
  String postLoginCommand;
  String ntpPrimary;
  String ntpSecondary;
  long tzOffsetSeconds = 0;
  long dstOffsetSeconds = 0;
  uint32_t reconnectDelayMs = 0;
  uint8_t backlightBrightness = 0;

  // Load all values from NVS, applying config.h defaults for missing keys.
  void load();

  // Save all values to NVS.
  void save();

  // Clear all NVS keys for this namespace.
  void reset();

  // True if a non-empty WiFi SSID is stored in NVS.
  bool hasWifiConfig() const;

  // True if a non-empty callsign is stored in NVS.
  bool hasCallsign() const;
};
