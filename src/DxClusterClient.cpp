#include "DxClusterClient.h"
#include "DxSpot.h"
#include <time.h>

DxClusterClient::DxClusterClient(size_t maxSpots) : _maxSpots(maxSpots) {
  if (_maxSpots > 0) {
    _spots = new DxSpot[_maxSpots];
  }
}

void DxClusterClient::setWifi(const String &ssid, const String &password) {
  _ssid = ssid;
  _password = password;
}

void DxClusterClient::setCluster(const String &host, uint16_t port,
                                 const String &callsign,
                                 const String &callsignPassword,
                                 const String &postLoginCommand) {
  _host = host;
  _port = port;
  _callsign = callsign;
  _callsignPassword = callsignPassword;
  _postLogin = postLoginCommand;
}

void DxClusterClient::setNtp(const String &primary, const String &secondary,
                             long tzOffsetSeconds, long daylightOffsetSeconds) {
  _ntpPrimary = primary;
  _ntpSecondary = secondary;
  _tzOffset = tzOffsetSeconds;
  _dstOffset = daylightOffsetSeconds;
}

void DxClusterClient::setReconnectDelayMs(uint32_t ms) {
  _reconnectDelayMs = ms;
}

bool DxClusterClient::connectWifi(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid.c_str(), _password.c_str());
  Serial.print(F("Connecting Wi-Fi"));
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
    if (millis() - start > timeoutMs) {
      Serial.println(F("\nWi-Fi connect timed out"));
      return false;
    }
    if (millis() - start > 10000) {
      WiFi.disconnect();
      WiFi.begin(_ssid.c_str(), _password.c_str());
      start = millis() - 5000;
    }
  }
  Serial.print(F("\nWi-Fi connected: "));
  Serial.println(WiFi.localIP());
  return true;
}

bool DxClusterClient::syncClock() {
  configTime(_tzOffset, _dstOffset, _ntpPrimary.c_str(),
             _ntpSecondary.c_str());
  Serial.print(F("Syncing UTC clock"));
  tm timeInfo;
  for (int i = 0; i < 30; ++i) {
    if (getLocalTime(&timeInfo, 500)) {
      Serial.println(F("\nUTC clock synced"));
      return true;
    }
    Serial.print('.');
  }
  Serial.println(F("\nUTC clock sync timed out; will retry in background"));
  return false;
}

void DxClusterClient::ensureWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Wi-Fi dropped, reconnecting..."));
    WiFi.disconnect();
    WiFi.begin(_ssid.c_str(), _password.c_str());
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
      delay(250);
    }
  }
}

void DxClusterClient::tryConnectCluster() {
  uint32_t now = millis();
  if (_client.connected() || now - _lastConnectAttemptMs < _reconnectDelayMs) {
    return;
  }
  _lastConnectAttemptMs = now;
  _loginSent = false;
  _passwordSent = false;
  _postLoginSent = false;
  _line = "";

  Serial.printf("Connecting telnet %s:%u\n", _host.c_str(), (unsigned)_port);
  if (_client.connect(_host.c_str(), _port)) {
    _connectedAtMs = now;
    _lastReceivedMs = now;
    Serial.println(F("DX cluster connected"));
  } else {
    Serial.println(F("DX cluster connect failed"));
  }
}

void DxClusterClient::sendLogin() {
  if (!_client.connected() || _loginSent) {
    return;
  }
  _client.print(_callsign);
  _client.print("\r\n");
  _loginSent = true;
  Serial.printf("Sent login: %s\n", _callsign.c_str());
}

void DxClusterClient::sendPassword() {
  if (!_client.connected() || _passwordSent || _callsignPassword.isEmpty()) {
    return;
  }
  _client.print(_callsignPassword);
  _client.print("\r\n");
  _passwordSent = true;
  Serial.println(F("Sent callsign password"));
}

void DxClusterClient::sendPostLogin() {
  if (!_client.connected() || _postLoginSent || _postLogin.isEmpty()) {
    return;
  }
  _client.print(_postLogin);
  _client.print("\r\n");
  _postLoginSent = true;
}

void DxClusterClient::pushSpot(const DxSpot &spot) {
  size_t moveCount = _spotCount < (_maxSpots - 1) ? _spotCount : (_maxSpots - 1);
  for (int i = (int)moveCount; i > 0; --i) {
    _spots[i] = _spots[i - 1];
  }
  _spots[0] = spot;
  if (_spotCount < _maxSpots) {
    _spotCount++;
  }
}

void DxClusterClient::pushResponse(const String &line) {
  // Skip empty lines and echo of our own commands.
  if (line.isEmpty()) return;
  // Don't store login/password prompt lines as response.
  String lower = line;
  lower.toLowerCase();
  if (lower.indexOf(F("login:")) >= 0 || lower.indexOf(F("password:")) >= 0 ||
      lower.indexOf(F("passcode:")) >= 0) {
    return;
  }
  if (_responseCount < MAX_RESPONSE_LINES) {
    _responseLines[_responseCount++] = line;
  } else {
    // Ring buffer: shift everything down and replace the last entry.
    for (size_t i = 0; i < MAX_RESPONSE_LINES - 1; ++i) {
      _responseLines[i] = _responseLines[i + 1];
    }
    _responseLines[MAX_RESPONSE_LINES - 1] = line;
  }
}

void DxClusterClient::handleLine(String line) {
  line.trim();
  if (line.isEmpty()) {
    return;
  }
  Serial.println(line);

  String lower = line;
  lower.toLowerCase();

  // Login prompt: send callsign
  if (!_loginSent) {
    if (lower.indexOf(F("login")) >= 0 || lower.indexOf(F("call")) >= 0 ||
        lower.indexOf(F("callsign")) >= 0 || lower.indexOf(F("please enter")) >= 0) {
      sendLogin();
      return;
    }
  }

  // Password prompt: send callsign password (if configured)
  if (_loginSent && !_passwordSent && !_callsignPassword.isEmpty()) {
    if (lower.indexOf(F("password")) >= 0 || lower.indexOf(F("passcode")) >= 0) {
      sendPassword();
      return;
    }
  }

  // After login (and optional password), send post-login command
  if (_loginSent && (_callsignPassword.isEmpty() || _passwordSent)) {
    sendPostLogin();
  }

  DxSpot spot;
  if (parseDxSpot(line, spot)) {
    pushSpot(spot);
  } else {
    // Not a spot — store in the response text buffer.
    pushResponse(line);
  }
}

void DxClusterClient::readCluster() {
  if (!_client.connected()) {
    return;
  }
  while (_client.available()) {
    char c = (char)_client.read();
    _lastReceivedMs = millis();
    if (c == '\n') {
      handleLine(_line);
      _line = "";
    } else if (c != '\r' && _line.length() < 240) {
      _line += c;
    }
  }
  // Fallback: if the cluster never sent a recognizable login prompt,
  // send the callsign after a short delay.
  if (!_loginSent && millis() - _connectedAtMs > 2500) {
    sendLogin();
  }
}

void DxClusterClient::loop() {
  ensureWifi();
  tryConnectCluster();
  readCluster();
}

bool DxClusterClient::isConnected() {
  return _client.connected();
}

void DxClusterClient::sendCommand(const String &command) {
  if (!_client.connected()) {
    Serial.println(F("Cannot send command: not connected"));
    return;
  }
  clearResponse();
  _client.print(command);
  _client.print("\r\n");
  Serial.printf("Sent command: %s\n", command.c_str());
}

size_t DxClusterClient::spotCount() const {
  return _spotCount;
}

const DxSpot *DxClusterClient::spotAt(size_t index) const {
  if (index >= _spotCount) {
    return nullptr;
  }
  return &_spots[index];
}

size_t DxClusterClient::responseCount() const {
  return _responseCount;
}

const String &DxClusterClient::responseLineAt(size_t index) const {
  static const String empty;
  if (index >= _responseCount) {
    return empty;
  }
  return _responseLines[index];
}

void DxClusterClient::clearResponse() {
  _responseCount = 0;
}

void DxClusterClient::updateClockText(char *out, size_t outLen) const {
  tm timeInfo;
  if (getLocalTime(&timeInfo, 10)) {
    snprintf(out, outLen, "%02d:%02dZ", timeInfo.tm_hour, timeInfo.tm_min);
  } else {
    snprintf(out, outLen, "--:--Z");
  }
}
