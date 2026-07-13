#pragma once

/*
 * WebAdmin — Captive-portal configuration web UI for the ESP32.
 *
 * When activated, the ESP32 starts a Wi-Fi AP + DNS captive portal + HTTP
 * server. Anyone who connects to the AP and opens any URL is redirected to
 * the config page at http://192.168.4.1/
 *
 * The config page presents a form with:
 *   - Wi-Fi SSID (with live scan dropdown)
 *   - Wi-Fi Password
 *   - Callsign
 *   - Callsign Password (optional — sent when the cluster asks "Password:")
 *   - DX Cluster Host
 *   - DX Cluster Port
 *   - Post-login command (optional, advanced)
 *
 * On submit, values are saved to NVS via AppConfig::save(), and the device
 * reboots into normal mode.
 */

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "AppConfig.h"

class WebAdmin {
 public:
  // Start the AP, DNS captive portal, and HTTP server.
  // apName is the visible Wi-Fi network name.
  void begin(AppConfig &config, const String &apName);

  // Call from loop(). Handles DNS + HTTP clients.
  void loop();

  // True after the user submitted the form (caller should reboot).
  bool shouldReboot() const;

  // Stop the AP and servers.
  void stop();

 private:
  void handleRoot();
  void handleSave();
  void handleScan();
  void handleRedirect();
  void handleNotFound();

  // The embedded HTML page. Kept in PROGMEM.
  static const char PAGE_HTML[];

  AppConfig *_config = nullptr;
  String _apName;
  DNSServer _dns;
  WebServer _server;
  bool _reboot = false;
  bool _running = false;
};
