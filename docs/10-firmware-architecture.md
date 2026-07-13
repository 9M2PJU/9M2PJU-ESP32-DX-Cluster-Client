# Firmware Architecture

This document describes the internal architecture of the 9M2PJU ESP32 DX Cluster Client: the module breakdown, the boot flow, the main loops, the display rendering pipeline, the command menu state machine, the configuration layering, and the board abstraction. It is intended for contributors who want to understand or modify the firmware.

---

## Module Overview

The firmware is a single PlatformIO project under `src/`. Each module has a clear responsibility:

| Module | Files | Responsibility |
|---|---|---|
| **Entry point** | `src/main.cpp` | Boot flow, mode selection, main loop dispatch. |
| **AppConfig** | `src/AppConfig.h/.cpp` | Runtime configuration stored in NVS (Preferences). |
| **WebAdmin** | `src/WebAdmin.h/.cpp` | Captive-portal setup UI (Wi-Fi AP + DNS + HTTP server). |
| **DxClusterClient** | `src/DxClusterClient.h/.cpp` | Wi-Fi, telnet client, login handshake, spot ring buffer, command send. |
| **DxSpot** | `src/DxSpot.h/.cpp` | Parses `DX de ...` spot lines; band color helper. |
| **DxDisplay** | `src/DxDisplay.h/.cpp` | Board-agnostic UI renderer (LovyanGFX), adaptive layout. |
| **CommandMenu** | `src/CommandMenu.h` | One-button menu state machine for cluster commands. |
| **BoardConfig** | `src/BoardConfig.h` | Preprocessor board selection; defines display class + geometry. |
| **Boards** | `src/boards/*.h` | Per-board LovyanGFX panel/bus/pin configurations (12 boards). |
| **Compile-time config** | `include/config.h` (from `config.example.h`) | Default values compiled into the firmware. |

### Dependency graph

```
                       include/config.h  (compile-time defaults)
                              |
                              v
main.cpp ---> AppConfig -----> NVS (Preferences)
        |         ^
        |         |
        +-> WebAdmin (writes config to NVS on save)
        |
        +-> DxClusterClient ---> DxSpot (parser)
        |         ^
        |         |
        +-> DxDisplay ---------> BoardConfig.h ---> src/boards/<board>.h
        |                                    (LovyanGFX subclass)
        +-> CommandMenu (reads BOOT button, emits events)
```

`main.cpp` owns the top-level objects (`AppConfig`, `DxClusterClient`, `DxDisplay`, `WebAdmin`, `CommandMenu`) and wires them together. The modules below are otherwise independent: `DxDisplay` does not know about `WebAdmin`, `CommandMenu` does not know about `DxClusterClient`, and so on.

---

## Boot Flow

`setup()` in `src/main.cpp` runs once at power-on or reset:

```
setup()
  |
  v
Serial.begin(115200)
  |
  v
appConfig.load()           <-- read NVS; fall back to config.h defaults
  |
  v
[BOARD_NEEDS_PMU_INIT?] --yes--> initTWatch2020PMU()   <-- enable display power
  |
  v
display.begin()            <-- init LovyanGFX, compute layout
  |
  v
Draw splash (project name + BOARD_NAME + "Booting...")
  |
  v
buttonHeld = isBootButtonHeld()    (GPIO0 held LOW at reset)
hasWifi   = appConfig.hasWifiConfig()
  |
  +-- buttonHeld OR !hasWifi --> startConfigMode()
  |
  +-- otherwise --------------> startNormalMode()
```

### Mode selection rules

The device enters **setup mode** if any of these is true:

1. The BOOT button (GPIO0) is held low during reset.
2. No Wi-Fi SSID is stored in NVS (first boot, or NVS was cleared).
3. (At runtime) Wi-Fi connection fails within 30 seconds — `startNormalMode()` falls back to setup mode.

Otherwise it enters **normal mode**.

### `startNormalMode()`

- Creates a `DxClusterClient` with a spot ring buffer of `MAX_STORED_DX_SPOTS` entries (default 12).
- Configures Wi-Fi credentials, cluster host/port/callsign, NTP, and reconnect delay from `AppConfig`.
- Calls `connectWifi(30000)`. On failure, switches to setup mode.
- On success, calls `syncClock()` (NTP) and invalidates the display for a full redraw.
- Configures GPIO0 as `INPUT_PULLUP` for button-menu use.

### `startConfigMode()`

- Builds an AP name of the form `9M2PJU-DXCluster-XXXX` (last 4 hex digits of the MAC).
- Starts the `WebAdmin` captive portal on that AP.
- Renders the config-mode screen (AP name + `192.168.4.1`).

---

## Main Loop: Normal Mode

When `configMode` is false, `loop()` runs the normal-mode cycle every iteration (~2 ms throttle):

```
loop()  [normal mode]
  |
  v
cluster->loop()
   |-- ensureWifi()         reconnect Wi-Fi if dropped
   |-- tryConnectCluster()  (re)connect telnet if not connected, respecting
   |                        reconnect delay; reset login flags on connect
   |-- readCluster()        read available bytes; assemble lines on '\n'
        |-- handleLine(line)
              |-- detect login/password prompt -> send callsign / password
              |-- sendPostLogin() once after login
              |-- parseDxSpot(line, spot)
                    |-- yes -> pushSpot(spot)   [ring buffer, newest at 0]
                    |-- no  -> pushResponse(line) [non-spot text buffer]
  |
  v
cmdMenu.loop()             read BOOT button, return Event
  |
  +-- Event::SEND   --> if connected: sendCommand(selectedItem().command)
  |                    else: close menu
  +-- Event::CLOSED --> display.invalidate()  (force full redraw)
  |
  v
switch (cmdMenu.state())
  |-- CLOSED  -> display.render(*cluster, millis())
  |-- BROWSING-> display.renderMenu(cmdMenu)
  |-- SHOWING -> display.renderResponse(*cluster, lastSentCommand)
  |
  v
delay(2)
```

### Spot ring buffer

`DxClusterClient` keeps a fixed-size array of `DxSpot` (size = `MAX_STORED_DX_SPOTS`, default 12). `pushSpot()` shifts existing entries down and inserts the new spot at index 0, so **index 0 is always the newest spot**. The display reads spots via `spotCount()` and `spotAt(index)`.

### Response (non-spot) buffer

Lines that are not DX spots (e.g. `sh/wwv` output, announcements) are stored in a separate ring buffer of up to 24 lines (`MAX_RESPONSE_LINES`). `sendCommand()` clears this buffer before sending so the response screen shows only the new command's output.

---

## Main Loop: Setup Mode

When `configMode` is true, `loop()` runs the setup-mode cycle:

```
loop()  [setup mode]
  |
  v
webAdmin.loop()
   |-- _dns.processNextRequest()   captive-portal DNS hijack
   |-- _server.handleClient()       HTTP request dispatch
        |-- GET /            -> handleRoot()   (serve PAGE_HTML)
        |-- POST /save       -> handleSave()
              |-- read form fields into AppConfig
              |-- apply defaults for empty host/port
              |-- config->save()   [writes NVS]
              |-- _reboot = true
        |-- GET /scan        -> handleScan()   (Wi-Fi scan, JSON)
        |-- * (catch-all)    -> 302 redirect to http://192.168.4.1/
  |
  v
Every 100 ms: display.renderConfigMode(apName, "192.168.4.1")
              (refresh for the pulsing animation)
  |
  v
if webAdmin.shouldReboot() --> delay(500) -> ESP.restart()
  |
  v
delay(2)
```

After reboot, `appConfig.load()` reads the freshly saved NVS values, and (assuming valid Wi-Fi credentials) the device boots into normal mode.

---

## Display Rendering Pipeline

`DxDisplay` is board-agnostic. It wraps the `BOARD_DISPLAY_CLASS` instance selected by `BoardConfig.h` and adapts its layout to the panel geometry computed in `begin()`.

### Layout adaptation (`begin()`)

Based on `BOARD_DISPLAY_WIDTH` / `BOARD_DISPLAY_HEIGHT` after rotation:

| Condition | Layout |
|---|---|
| Width == height (round/square) | `_round = true`; compact single-spot view via `drawRoundCompact()`. |
| Height <= 64 | Tiny: 14 px header, 1 row, no comment. |
| Height <= 135 | Small: 18 px header, 1 row, comment shown. |
| Height <= 170 | Medium: 22 px header, multiple rows, comment shown. |
| Otherwise | Large: 24 px header, 38 px rows, comment shown. |

`_maxVisible` is computed as `(height - headerHeight) / rowHeight`. The comment line is hidden on very small panels (`_showComment = height >= 90`).

### Frame rendering (`render()`)

Throttled to ~30 fps (`FRAME_MS = 33`). On each frame:

1. **Round panels:** call `drawRoundCompact()` — draws a connection ring, title, UTC clock, the newest spot's frequency (large), DX call (larger), and spotter (small). Returns.
2. **Rectangular panels:**
   - `fillScreen(background)`.
   - `drawHeader()` — title (truncated to fit), UTC clock (right-aligned), connection pulse dot (blue when connected, orange when not), animated accent sweep.
   - If no spots: `drawEmptyState()` ("Waiting for DX spots..." or "Connecting to DX cluster...").
   - Otherwise: `drawSpot()` for each of the visible spots (newest first). Each spot shows frequency, DX call (in band color), spotter, and comment. Spots less than 5 seconds old (`NEW_SPOT_MS`) are drawn with a highlighted panel background.

### Band coloring

`bandColor(float mhz)` in `DxSpot.cpp` maps a frequency to an RGB565 color by amateur band: 160m magenta, 80m cyan, 40m green, 20m yellow, 15m orange, 10m red-blue, 6m red, VHF grey, UHF teal, SHF white. The color is used as the accent for the frequency and DX call.

### Menu and response rendering

When the command menu is open, `renderMenu()` draws a scrollable list of the 12 cluster commands with the selected item highlighted. When a command has been sent, `renderResponse()` draws the command title as a header and the tail of the response buffer below it.

---

## Command Menu State Machine

`CommandMenu` (header-only, `src/CommandMenu.h`) implements a one-button menu driven by the BOOT button (GPIO0, active LOW). It produces events; `main.cpp` acts on them.

### States

| State | Meaning |
|---|---|
| `CLOSED` | Menu not shown; normal spot display is rendered. |
| `BROWSING` | Menu open; short press advances to the next item. |
| `SHOWING` | A command was sent; the response screen is rendered. |

### Events

| Event | Trigger |
|---|---|
| `OPENED` | Short press from `CLOSED`. |
| `NEXT` | Short press from `BROWSING`. |
| `SEND` | Long press (>= 800 ms) from `BROWSING`. |
| `CLOSED` | Short press from `SHOWING`, or timeout. |
| `NONE` | No event this iteration. |

### Transitions

```
            short press            long press (>=800ms)
CLOSED  ----------------> BROWSING  ----------------> SHOWING
  ^                          |                          |
  |   short press            | 10s timeout              | 8s timeout
  +--------------------------+                          |
  +-----------------------------------------------------+
            short press from SHOWING
```

A "short press" is a press released in under 800 ms; a "long press" fires at 800 ms while held (and again on release if it had not already fired). The menu auto-closes after 10 s of inactivity in `BROWSING` or 8 s in `SHOWING`.

### Command table

The menu offers 12 common DXSpider commands:

| Label | Command | Description |
|---|---|---|
| `sh/dx` | `sh/dx` | Recent spots |
| `sh/dx 20` | `sh/dx 20` | Last 20 spots |
| `sh/dx/ft8` | `sh/dx/ft8` | FT8 spots |
| `sh/dx/cw` | `sh/dx/cw` | CW spots |
| `sh/dx/ssb` | `sh/dx/ssb` | SSB spots |
| `sh/wwv` | `sh/wwv` | Solar / geo |
| `sh/muf` | `sh/muf` | MUF info |
| `sh/qtc` | `sh/qtc` | QTC bulletins |
| `sh/ann` | `sh/ann` | Announcements |
| `sh/u` | `sh/u` | Users online |
| `sh/c` | `sh/c` | Cluster links |
| `sh/h` | `sh/h` | Help |

When `Event::SEND` is produced, `main.cpp` checks `cluster->isConnected()`, calls `cluster->sendCommand(command)` (which clears the response buffer and sends the command + CRLF), records the command label as the response-screen title, and the next frames render `renderResponse()`.

---

## Configuration Layering

Settings come from two layers, with NVS taking precedence:

```
Layer 1 (compile-time):  include/config.h  (from config.example.h)
                              |
                              v   used as fallback defaults
Layer 2 (runtime):       NVS namespace "dxcluster"  (Preferences)
                              |
                              v   written by WebAdmin on form submit
                         AppConfig struct  (in RAM)
```

### Load (`AppConfig::load()`)

For each key, `load()` reads the value from NVS. If the NVS value is empty, it falls back to the corresponding `config.h` macro:

| NVS key | Fallback macro |
|---|---|
| `wifi_ssid` | (none — empty triggers setup mode) |
| `wifi_pass` | (none) |
| `cluster_host` | `DX_CLUSTER_HOST` |
| `cluster_port` | `DX_CLUSTER_PORT` |
| `callsign` | `DX_CLUSTER_LOGIN_CALLSIGN` |
| `call_pass` | `DX_CLUSTER_CALLSIGN_PASSWORD` |
| `post_login` | `DX_CLUSTER_POST_LOGIN_COMMAND` |
| `ntp1` | `NTP_SERVER_PRIMARY` |
| `ntp2` | `NTP_SERVER_SECONDARY` |
| `tz_offset` | `CLOCK_TIMEZONE_OFFSET_SECONDS` |
| `dst_offset` | `CLOCK_DAYLIGHT_OFFSET_SECONDS` |
| `reconnect_ms` | `TELNET_RECONNECT_DELAY_MS` |
| `brightness` | `DISPLAY_BACKLIGHT_BRIGHTNESS` |

### Save (`AppConfig::save()`)

`WebAdmin::handleSave()` reads the form fields into the `AppConfig` struct, applies defaults for empty host/port, then calls `save()` which writes every key to NVS. The device then reboots, and `load()` picks up the new values.

### Setup-mode triggers

`hasWifiConfig()` returns true only if a non-empty SSID is stored and it is not the placeholder `YOUR_WIFI_SSID`. This is what makes a freshly-flashed device (with only compile-time defaults) boot into setup mode on first run.

---

## Board Abstraction

The board is selected entirely at compile time via preprocessor macros. No runtime board detection is performed.

### How it works

1. `platformio.ini` sets a `-D BOARD_<NAME>` flag for the environment being built.
2. `src/BoardConfig.h` uses a chain of `#if defined(BOARD_<NAME>)` / `#elif` blocks to:
   - `#include` the matching `src/boards/<name>.h` header.
   - Define `BOARD_DISPLAY_CLASS` (the LovyanGFX subclass), `BOARD_DISPLAY_WIDTH`, `BOARD_DISPLAY_HEIGHT`, `BOARD_DISPLAY_ROTATION`, `BOARD_HAS_BACKLIGHT`, and optionally `BOARD_NEEDS_PMU_INIT`.
   - Define `BOARD_NAME` (a human-readable string for the splash screen).
3. If no `BOARD_*` flag is set, a fallback selects `BOARD_LILYGO_TDISPLAY_S3`.
4. `DxDisplay` holds a member `BOARD_DISPLAY_CLASS _lcd;` — the concrete subclass is baked in at compile time. All rendering code calls methods on `_lcd` that are common to all LovyanGFX devices.

### The board header pattern

Each `src/boards/<name>.h` defines a class deriving from `lgfx::LGFX_Device` that configures a panel, a bus (SPI / parallel / QSPI), and a backlight. The constructor wires them together:

```c
class LGFX_MYBOARD : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Light_PWM _light;
 public:
  LGFX_MYBOARD(void) {
    { /* configure _bus */  _bus.config(cfg);   _panel.setBus(&_bus); }
    { /* configure _panel */ _panel.config(cfg); }
    { /* configure _light */ _light.config(cfg); _panel.setLight(&_light); }
    setPanel(&_panel);
  }
};
```

Because the subclass is instantiated directly (not via a virtual factory), there is zero runtime overhead from board abstraction: the compiler emits only the code for the selected board.

### PMU handling

Boards whose display power is gated behind a PMU (currently the T-Watch 2020 with its AXP202) define `BOARD_NEEDS_PMU_INIT` and an `init<Board>PMU()` function in their board header. `main.cpp` calls the function in `setup()` before `display.begin()`:

```cpp
#if defined(BOARD_NEEDS_PMU_INIT)
  initTWatch2020PMU();
  delay(100);
#endif
```

This is also preprocessor-gated so non-PMU boards pay no cost.

---

## Data Flow Diagram

The diagram below shows the end-to-end flow of a DX spot from the cluster server to the screen, and the flow of a config save from the browser to NVS.

```
NORMAL MODE — a DX spot from cluster to screen:

  DXSpider server (telnet :7300)
        |
        |  "DX de 9M2PJU: 14.074 JA1ABC FT8, strong 1234Z\r\n"
        v
  DxClusterClient::readCluster()        [reads bytes, assembles line on '\n']
        |
        v
  DxClusterClient::handleLine(line)
        |-- (login/password handshake if prompt detected)
        |-- parseDxSpot(line, spot)     [DxSpot.cpp]
        |       |-- spotter  = "9M2PJU"
        |       |-- frequency = "14.074"
        |       |-- dxCall   = "JA1ABC"
        |       |-- comment  = "FT8, strong"  (trailing 1234Z stripped)
        |       |-- receivedAt = time(nullptr)
        |       +-- valid = true
        v
  pushSpot(spot)                        [ring buffer, index 0 = newest]
        |
        v
  main loop: display.render(*cluster, nowMs)
        v
  DxDisplay::drawSpot(spot, y, fresh, ...)
        |-- bandColor(14.074) -> 20m yellow
        |-- draw frequency, DX call (yellow), spotter, comment
        v
  LovyanGFX panel (BOARD_DISPLAY_CLASS) -> physical LCD


SETUP MODE — a config save from browser to NVS:

  Browser (http://192.168.4.1/)
        |
        |  POST /save  ssid=...&wifipass=...&callsign=...&host=...&port=...
        v
  WebAdmin::handleSave()
        |-- read form args into AppConfig
        |-- apply defaults for empty host/port
        v
  AppConfig::save()                     [writes NVS namespace "dxcluster"]
        |
        v
  _reboot = true  ->  main loop sees shouldReboot()  ->  ESP.restart()
        |
        v
  setup(): appConfig.load() reads new NVS values -> normal mode
```

---

## Summary

- **One entry point** (`main.cpp`) owns all top-level objects and dispatches between setup and normal modes.
- **Two loops**: setup mode serves a captive portal and writes NVS; normal mode reads telnet, parses spots, and renders.
- **Board selection is compile-time only**, via a single `-D BOARD_*` flag that flows through `BoardConfig.h` into a concrete LovyanGFX subclass. No runtime cost.
- **Configuration is layered**: compile-time defaults in `config.h` are overridden by NVS values written by the web admin.
- **The display adapts** to any panel size automatically, with a dedicated compact renderer for round displays.
- **The command menu** is a pure state machine that emits events; the main loop decides what to send and how to render the response.

For building instructions, see [07-developer-guide.md](07-developer-guide.md). For adding new hardware, see [08-adding-new-board.md](08-adding-new-board.md). For runtime issues, see [09-troubleshooting.md](09-troubleshooting.md).
