#pragma once

/*
 * 9M2PJU ESP32 DX Cluster Client — default configuration.
 *
 * These values are compiled into the firmware as DEFAULTS. After the first
 * boot, all settings can be changed at runtime via the built-in web admin UI
 * (captive portal) and are stored in NVS (non-volatile storage).
 *
 * The web admin UI is activated when:
 *   - The BOOT button (GPIO0) is held during power-on/reset, OR
 *   - No Wi-Fi SSID has been saved yet (first boot), OR
 *   - The configured Wi-Fi network cannot be reached within 30 seconds
 *
 * To force the web admin UI at any time: hold the BOOT button while pressing
 * RESET (or while powering on).
 *
 * Copy this file to include/config.h to build:
 *
 *   cp include/config.example.h include/config.h
 *
 * include/config.h is ignored by git so your credentials are never committed.
 */

// ---------------------------------------------------------------------------
// Wi-Fi (defaults — overridden by web admin UI after first config)
// ---------------------------------------------------------------------------
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ---------------------------------------------------------------------------
// DX cluster (DXSpider telnet) — defaults, overridden by web admin UI
// ---------------------------------------------------------------------------
#define DX_CLUSTER_HOST "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT 7300
#define DX_CLUSTER_LOGIN_CALLSIGN "N0CALL"

// Callsign password — sent when the cluster asks "Password:".
// Leave empty ("") if your cluster does not require one.
// Can be set via the web admin UI.
#define DX_CLUSTER_CALLSIGN_PASSWORD ""

// Optional command sent once after login (e.g. "set/dx" or a filter line).
#define DX_CLUSTER_POST_LOGIN_COMMAND ""

// ---------------------------------------------------------------------------
// UTC clock / NTP
// ---------------------------------------------------------------------------
#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.google.com"
#define CLOCK_TIMEZONE_OFFSET_SECONDS 0
#define CLOCK_DAYLIGHT_OFFSET_SECONDS 0

// ---------------------------------------------------------------------------
// Behaviour
// ---------------------------------------------------------------------------
#define MAX_STORED_DX_SPOTS 12
#define TELNET_RECONNECT_DELAY_MS 8000

// Backlight brightness 0..255 (only used on boards with a PWM backlight pin).
#define DISPLAY_BACKLIGHT_BRIGHTNESS 220
