#pragma once

/*
 * DxClusterClient — Wi-Fi + telnet client for a DXSpider (or compatible)
 * DX cluster node.
 *
 * Lifecycle:
 *   1. connectWifi(timeoutMs)  — try to connect to Wi-Fi (returns false on timeout)
 *   2. syncClock()             — NTP sync UTC
 *   3. loop() called every iteration:
 *        - reconnect Wi-Fi if dropped
 *        - (re)connect telnet to the cluster
 *        - read lines, detect login prompt, send callsign
 *        - detect "password:" prompt, send callsign password
 *        - parse DX de ... spot lines and push them into a ring buffer
 *
 * Spots are exposed via spotCount() / spotAt() for the UI layer.
 */

#include <Arduino.h>
#include <WiFi.h>

struct DxSpot;

class DxClusterClient {
 public:
  DxClusterClient(size_t maxSpots);

  // Configuration. Set before calling connectWifi().
  void setWifi(const String &ssid, const String &password);
  void setCluster(const String &host, uint16_t port, const String &callsign,
                  const String &callsignPassword = "",
                  const String &postLoginCommand = "");
  void setNtp(const String &primary, const String &secondary,
              long tzOffsetSeconds = 0, long daylightOffsetSeconds = 0);
  void setReconnectDelayMs(uint32_t ms);

  // Attempt Wi-Fi connect. Returns true on success, false on timeout.
  bool connectWifi(uint32_t timeoutMs = 30000);

  // Non-blocking NTP sync attempt. Returns true if clock is set.
  bool syncClock();

  // Call from loop(). Handles Wi-Fi keep-up, telnet connect, line reading.
  void loop();

  // True if currently connected to the cluster telnet.
  bool isConnected();

  // Send a raw command string to the cluster (e.g. "sh/dx").
  // Clears the response buffer first. No-op if not connected.
  void sendCommand(const String &command);

  // Spot ring buffer. Index 0 is the newest.
  size_t spotCount() const;
  const DxSpot *spotAt(size_t index) const;

  // Response (non-spot text) ring buffer. Filled with cluster output
  // that isn't a DX spot line — e.g. sh/wwv, sh/u, sh/c output.
  size_t responseCount() const;
  const String &responseLineAt(size_t index) const;
  void clearResponse();

  // UTC clock as "HH:MMZ".
  void updateClockText(char *out, size_t outLen) const;

 private:
  void ensureWifi();
  void tryConnectCluster();
  void readCluster();
  void handleLine(String line);
  void sendLogin();
  void sendPassword();
  void sendPostLogin();
  void pushSpot(const DxSpot &spot);
  void pushResponse(const String &line);

  String _ssid;
  String _password;
  String _host;
  uint16_t _port = 0;
  String _callsign;
  String _callsignPassword;
  String _postLogin;
  String _ntpPrimary;
  String _ntpSecondary;
  long _tzOffset = 0;
  long _dstOffset = 0;
  uint32_t _reconnectDelayMs = 8000;

  WiFiClient _client;
  String _line;
  uint32_t _lastConnectAttemptMs = 0;
  uint32_t _connectedAtMs = 0;
  uint32_t _lastReceivedMs = 0;
  bool _loginSent = false;
  bool _passwordSent = false;
  bool _postLoginSent = false;

  DxSpot *_spots = nullptr;
  size_t _maxSpots = 0;
  size_t _spotCount = 0;

  // Response (non-spot text) ring buffer.
  static constexpr size_t MAX_RESPONSE_LINES = 24;
  String _responseLines[MAX_RESPONSE_LINES];
  size_t _responseCount = 0;
};
