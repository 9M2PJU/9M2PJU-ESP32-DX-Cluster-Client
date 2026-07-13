#include "AppConfig.h"

#if __has_include("config.h")
#include "config.h"
#else
#error "Missing include/config.h. Copy include/config.example.h to include/config.h."
#endif

#include <Preferences.h>
#include <nvs_flash.h>

static constexpr const char *NVS_NAMESPACE = "dxcluster";

// Helper: read a string from NVS, fall back to a compile-time default.
static String getString(Preferences &prefs, const char *key,
                        const char *fallback) {
  String val = prefs.getString(key, "");
  if (val.isEmpty() && fallback && fallback[0]) {
    val = String(fallback);
  }
  return val;
}

void AppConfig::load() {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, true);

  wifiSsid = prefs.getString("wifi_ssid", "");
  wifiPassword = prefs.getString("wifi_pass", "");

  clusterHost = getString(prefs, "cluster_host", DX_CLUSTER_HOST);
  clusterPort = (uint16_t)prefs.getUShort("cluster_port", DX_CLUSTER_PORT);
  callsign = getString(prefs, "callsign", DX_CLUSTER_LOGIN_CALLSIGN);
  callsignPassword = getString(prefs, "call_pass", DX_CLUSTER_CALLSIGN_PASSWORD);
  postLoginCommand =
      getString(prefs, "post_login", DX_CLUSTER_POST_LOGIN_COMMAND);

  ntpPrimary = getString(prefs, "ntp1", NTP_SERVER_PRIMARY);
  ntpSecondary = getString(prefs, "ntp2", NTP_SERVER_SECONDARY);
  tzOffsetSeconds = prefs.getLong("tz_offset", CLOCK_TIMEZONE_OFFSET_SECONDS);
  dstOffsetSeconds = prefs.getLong("dst_offset", CLOCK_DAYLIGHT_OFFSET_SECONDS);
  reconnectDelayMs =
      prefs.getULong("reconnect_ms", TELNET_RECONNECT_DELAY_MS);
  backlightBrightness =
      (uint8_t)prefs.getUChar("brightness", DISPLAY_BACKLIGHT_BRIGHTNESS);

  prefs.end();
}

void AppConfig::save() {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, false);

  prefs.putString("wifi_ssid", wifiSsid);
  prefs.putString("wifi_pass", wifiPassword);
  prefs.putString("cluster_host", clusterHost);
  prefs.putUShort("cluster_port", clusterPort);
  prefs.putString("callsign", callsign);
  prefs.putString("call_pass", callsignPassword);
  prefs.putString("post_login", postLoginCommand);
  prefs.putString("ntp1", ntpPrimary);
  prefs.putString("ntp2", ntpSecondary);
  prefs.putLong("tz_offset", tzOffsetSeconds);
  prefs.putLong("dst_offset", dstOffsetSeconds);
  prefs.putULong("reconnect_ms", reconnectDelayMs);
  prefs.putUChar("brightness", backlightBrightness);

  prefs.end();
}

void AppConfig::reset() {
  Preferences prefs;
  prefs.begin(NVS_NAMESPACE, false);
  prefs.clear();
  prefs.end();
}

bool AppConfig::hasWifiConfig() const {
  return wifiSsid.length() > 0 && wifiSsid != F("YOUR_WIFI_SSID");
}

bool AppConfig::hasCallsign() const {
  return callsign.length() > 0 && callsign != F("YOUR_CALLSIGN");
}
