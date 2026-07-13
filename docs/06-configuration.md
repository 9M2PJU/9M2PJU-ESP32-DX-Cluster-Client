# Configuration Reference

The 9M2PJU ESP32 DX Cluster Client has two layers of configuration:

1. **Compile-time defaults** in `include/config.h` — baked into the firmware
   at build time. These provide safe fallback values.
2. **Runtime values** in NVS — written by the web admin captive portal and
   read back on every boot.

At runtime, NVS values take precedence: any value saved through the web
admin overrides the matching compile-time default. Values that were never
saved fall back to the defaults from `config.h`.

This document is the complete reference for both layers.

---

## How the Two Layers Interact

On boot, `AppConfig::load()` (`src/AppConfig.cpp`) opens the NVS namespace
`dxcluster` and reads each key. For every field it uses the pattern:

```cpp
wifiSsid = prefs.getString("wifi_ssid", "");
clusterHost = getString(prefs, "cluster_host", DX_CLUSTER_HOST);
clusterPort = (uint16_t)prefs.getUShort("cluster_port", DX_CLUSTER_PORT);
```

- If the NVS key exists and is non-empty, the NVS value is used.
- If the NVS key is missing or empty, the compile-time `#define` from
  `config.h` is used instead.

After the web admin saves, `AppConfig::save()` writes every field to NVS
and the device reboots, so the new values take effect immediately.

```
   include/config.h            NVS ("dxcluster")           AppConfig (in memory)
   (compile-time)              (runtime, written by        (loaded at boot)
                                the web admin)
        |                              |                              |
        |  defaults                    |  saved values                |  used by app
        v                              v                              v
   WIFI_SSID "..."  <--fallback--  wifi_ssid  <--override--        wifiSsid
   DX_CLUSTER_HOST  <--fallback--  cluster_host <--override--      clusterHost
        ...                              ...                              ...
```

---

## Creating `config.h`

The repository ships a template at `include/config.example.h`. The actual
`include/config.h` is **git-ignored** so your credentials are never
committed.

To build the firmware you must create it from the template:

```bash
cp include/config.example.h include/config.h
```

Then edit `include/config.h` and fill in your defaults. If you skip this
step the build fails with:

```
#error "Missing include/config.h. Copy include/config.example.h to include/config.h."
```

This check lives in `src/AppConfig.cpp` and `src/main.cpp` via
`#if __has_include("config.h")`.

---

## The `config.example.h` Template

Below is the full template. Every value here is a **default** and can be
overridden later at runtime through the web admin (for the user-facing
fields) or by editing this file and reflashing (for the behavioural
fields).

```c
#pragma once

// Wi-Fi (defaults — overridden by web admin UI after first config)
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// DX cluster (DXSpider telnet) — defaults, overridden by web admin UI
#define DX_CLUSTER_HOST "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT 7300
#define DX_CLUSTER_LOGIN_CALLSIGN "N0CALL"
#define DX_CLUSTER_CALLSIGN_PASSWORD ""
#define DX_CLUSTER_POST_LOGIN_COMMAND ""

// UTC clock / NTP
#define NTP_SERVER_PRIMARY "pool.ntp.org"
#define NTP_SERVER_SECONDARY "time.google.com"
#define CLOCK_TIMEZONE_OFFSET_SECONDS 0
#define CLOCK_DAYLIGHT_OFFSET_SECONDS 0

// Behaviour
#define MAX_STORED_DX_SPOTS 12
#define TELNET_RECONNECT_DELAY_MS 8000

// Backlight brightness 0..255 (only used on boards with a PWM backlight pin)
#define DISPLAY_BACKLIGHT_BRIGHTNESS 220
```

---

## Full Settings Reference

The table below lists every configuration field, its compile-time default
macro, its default value, the NVS key it is stored under at runtime, and
whether it can be changed from the web admin.

### Wi-Fi

| Setting | `config.h` macro | Default | NVS key | Web admin? |
| --- | --- | --- | --- | --- |
| Wi-Fi SSID | `WIFI_SSID` | `YOUR_WIFI_SSID` | `wifi_ssid` | Yes |
| Wi-Fi password | `WIFI_PASSWORD` | `YOUR_WIFI_PASSWORD` | `wifi_pass` | Yes |

### DX cluster

| Setting | `config.h` macro | Default | NVS key | Web admin? |
| --- | --- | --- | --- | --- |
| Cluster host | `DX_CLUSTER_HOST` | `9m2pju.hamradio.my` | `cluster_host` | Yes |
| Cluster port | `DX_CLUSTER_PORT` | `7300` | `cluster_port` | Yes |
| Login callsign | `DX_CLUSTER_LOGIN_CALLSIGN` | `N0CALL` | `callsign` | Yes |
| Callsign password | `DX_CLUSTER_CALLSIGN_PASSWORD` | `""` | `call_pass` | Yes |
| Post-login command | `DX_CLUSTER_POST_LOGIN_COMMAND` | `""` | `post_login` | Yes (Advanced) |

### UTC clock / NTP

| Setting | `config.h` macro | Default | NVS key | Web admin? |
| --- | --- | --- | --- | --- |
| Primary NTP server | `NTP_SERVER_PRIMARY` | `pool.ntp.org` | `ntp1` | No |
| Secondary NTP server | `NTP_SERVER_SECONDARY` | `time.google.com` | `ntp2` | No |
| Timezone offset (s) | `CLOCK_TIMEZONE_OFFSET_SECONDS` | `0` | `tz_offset` | No |
| Daylight offset (s) | `CLOCK_DAYLIGHT_OFFSET_SECONDS` | `0` | `dst_offset` | No |

### Behaviour

| Setting | `config.h` macro | Default | NVS key | Web admin? |
| --- | --- | --- | --- | --- |
| Max stored DX spots | `MAX_STORED_DX_SPOTS` | `12` | (compile-time only) | No |
| Telnet reconnect delay (ms) | `TELNET_RECONNECT_DELAY_MS` | `8000` | `reconnect_ms` | No |
| Backlight brightness (0-255) | `DISPLAY_BACKLIGHT_BRIGHTNESS` | `220` | `brightness` | No |

> `MAX_STORED_DX_SPOTS` sizes the in-memory spot ring buffer at compile
> time, so it cannot be changed without reflashing.

---

## Field Details

### Wi-Fi

- **`WIFI_SSID` / `WIFI_PASSWORD`** — The network the device joins at boot.
  These are only used as defaults; after the first successful web-admin
  save the NVS values are used. `AppConfig::hasWifiConfig()` returns true
  only when a non-empty SSID that is not the placeholder `YOUR_WIFI_SSID`
  is stored, which is what triggers normal mode on first boot logic.

### DX cluster

- **`DX_CLUSTER_HOST`** — Hostname or IP address of the DXSpider telnet
  server. The default points at the project's own cluster.
- **`DX_CLUSTER_PORT`** — TCP port. DXSpider telnet conventionally listens
  on `7300`.
- **`DX_CLUSTER_LOGIN_CALLSIGN`** — Sent when the cluster presents a
  `login:` / `call:` / `callsign:` / `please enter` prompt. A fallback
  also sends the callsign 2.5 s after connecting if no prompt was seen.
- **`DX_CLUSTER_CALLSIGN_PASSWORD`** — Sent only when the cluster asks for
  `password:` or `passcode:`. Leave empty (`""`) if your cluster does not
  require authentication.
- **`DX_CLUSTER_POST_LOGIN_COMMAND`** — A single command sent once after
  login (and password, if any) completes. Useful for `set/dx` or a custom
  filter line. Leave empty if not needed.

### UTC clock / NTP

- **`NTP_SERVER_PRIMARY` / `NTP_SERVER_SECONDARY`** — Two NTP servers used
  by `configTime()` to sync the ESP32's RTC. The clock is displayed as UTC
  in the header.
- **`CLOCK_TIMEZONE_OFFSET_SECONDS` / `CLOCK_DAYLIGHT_OFFSET_SECONDS`** —
  Passed to `configTime()` as the gmtOffset and daylightOffset parameters.
  Defaults of `0` keep the displayed clock in UTC, which is conventional
  for amateur radio logging.

### Behaviour

- **`MAX_STORED_DX_SPOTS`** — Number of DX spots kept in the in-memory
  ring buffer. Newer spots push older ones down; once the buffer is full
  the oldest is discarded. Default `12`.
- **`TELNET_RECONNECT_DELAY_MS`** — Minimum delay between telnet reconnect
  attempts when the cluster connection drops. Default `8000` ms.
- **`DISPLAY_BACKLIGHT_BRIGHTNESS`** — PWM duty cycle 0-255 for the
  display backlight, applied only on boards whose `BoardConfig.h` defines
  a PWM-capable backlight pin. Default `220`.

---

## How the Web Admin Saves to NVS

The save flow (in `src/WebAdmin.cpp`, `handleSave()`) is:

1. The browser POSTs the form to `/save` as URL-encoded parameters.
2. `handleSave()` reads each parameter with `_server.arg(...)` and writes
   it into the shared `AppConfig` instance.
3. Empty required fields get safe defaults applied (port `7300`, host
   `9m2pju.hamradio.my`).
4. `AppConfig::save()` is called, which opens the `dxcluster` namespace in
   read-write mode and writes every field with the appropriate
   `Preferences` putter (`putString`, `putUShort`, etc.).
5. The namespace is closed (flushing to flash), a 200 OK is sent to the
   browser, and the `_reboot` flag is set.
6. The main loop sees `shouldReboot()` return true, waits 500 ms, and
   calls `ESP.restart()`.

Only the user-facing fields (Wi-Fi, callsign, cluster host/port, passwords,
post-login command) are written by the web admin. The behavioural and NTP
fields retain whatever is already in NVS, or the compile-time default if
they have never been written.

### Clearing all settings

`AppConfig::reset()` clears the entire `dxcluster` namespace:

```cpp
Preferences prefs;
prefs.begin("dxcluster", false);
prefs.clear();
prefs.end();
```

After a reset the next boot behaves like a first boot: the device enters
setup mode because `hasWifiConfig()` returns false.

---

## See Also

- [04-web-admin-setup.md](04-web-admin-setup.md) — Step-by-step captive
  portal configuration guide.
- [05-button-menu.md](05-button-menu.md) — BOOT button command menu.
- `include/config.example.h` — The annotated template.
- `src/AppConfig.cpp` / `src/AppConfig.h` — NVS load/save implementation.
- `src/WebAdmin.cpp` — Captive portal and save handler.
