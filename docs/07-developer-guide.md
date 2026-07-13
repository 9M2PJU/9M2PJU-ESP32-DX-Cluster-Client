# Developer Guide: Building from Source

This guide covers building the 9M2PJU ESP32 DX Cluster Client firmware from source for any of the 12 supported boards. It is intended for contributors, advanced users, and anyone who wants to build locally instead of using the browser-based web flasher.

---

## Prerequisites

| Requirement | Notes |
|---|---|
| **PlatformIO** | PlatformIO Core (CLI) or the PlatformIO IDE extension for VS Code. |
| **Python 3** | Required by PlatformIO. Version 3.8 or newer recommended. |
| **Git** | For cloning the repository. |
| **USB serial driver** | Some boards need a driver (CP210x, CH340, or native USB CDC for ESP32-S3). |
| **USB data cable** | A cable that carries data, not charge-only. Connect directly to the host, not through a hub if possible. |

### Installing PlatformIO Core (CLI)

```bash
python -m pip install --upgrade pip
pip install platformio esptool
```

Verify the install:

```bash
pio --version
```

### Installing PlatformIO IDE (VS Code)

1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. Open the Extensions panel (`Ctrl+Shift+X` / `Cmd+Shift+X`).
3. Search for **PlatformIO IDE** and install it.
4. Restart VS Code. The PlatformIO toolbar (checkmark, arrow, plug icon) appears in the bottom status bar.

PlatformIO IDE bundles its own Python and PlatformIO Core, so a separate `pip install` is not required.

---

## Cloning the Repository

```bash
git clone https://github.com/9M2PJU/9M2PJU-ESP32-DX-Cluster-Client.git
cd 9M2PJU-ESP32-DX-Cluster-Client
```

---

## Creating `config.h`

The firmware reads compile-time defaults from `include/config.h`. This file is **git-ignored** so your credentials are never committed. A template is provided:

```bash
cp include/config.example.h include/config.h
```

Then edit `include/config.h` and set sensible defaults for your environment:

```c
#define WIFI_SSID              "YOUR_WIFI_SSID"
#define WIFI_PASSWORD          "YOUR_WIFI_PASSWORD"
#define DX_CLUSTER_HOST        "9m2pju.hamradio.my"
#define DX_CLUSTER_PORT        7300
#define DX_CLUSTER_LOGIN_CALLSIGN "N0CALL"
#define DX_CLUSTER_CALLSIGN_PASSWORD ""
#define DX_CLUSTER_POST_LOGIN_COMMAND ""
#define NTP_SERVER_PRIMARY     "pool.ntp.org"
#define NTP_SERVER_SECONDARY   "time.google.com"
#define CLOCK_TIMEZONE_OFFSET_SECONDS 0
#define CLOCK_DAYLIGHT_OFFSET_SECONDS 0
#define MAX_STORED_DX_SPOTS    12
#define TELNET_RECONNECT_DELAY_MS 8000
#define DISPLAY_BACKLIGHT_BRIGHTNESS 220
```

> The values in `config.h` are **defaults only**. After the first boot, all settings can be changed at runtime through the built-in web admin captive portal and are stored in NVS. The NVS values override the compile-time defaults on every subsequent boot.

If `include/config.h` is missing, the build fails immediately with:

```
#error "Missing include/config.h. Copy include/config.example.h to include/config.h."
```

---

## The `platformio.ini` Structure

The project defines one PlatformIO environment per supported board. All environments share the same `src/` core and `include/config.h`; only the board-specific build flags differ.

### The `[common]` section

Shared settings applied to every environment:

```ini
[common]
lib_deps =
  lovyan03/LovyanGFX@^1.2.0
build_flags =
  -Wall -Wextra
monitor_speed = 115200
upload_speed = 921600
```

- **`lib_deps`** declares [LovyanGFX](https://github.com/lovyan03/LovyanGFX), the display abstraction library used for all 12 boards. PlatformIO downloads it automatically on first build.
- **`build_flags`** enables extra compiler warnings.
- **`monitor_speed`** and **`upload_speed`** are shared serial settings.

### Per-board `[env:...]` sections

Each board has its own environment that inherits from `[common]` and adds a `-D BOARD_*` flag plus any board-specific options. Example for the LilyGO T-Display-S3:

```ini
[env:lilygo-tdisplay-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
lib_deps = ${common.lib_deps}
build_flags =
  ${common.build_flags}
  -D BOARD_LILYGO_TDISPLAY_S3
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DARDUINO_USB_MODE=1
  -DBOARD_HAS_PSRAM
monitor_speed = ${common.monitor_speed}
upload_speed = ${common.upload_speed}
```

The `-D BOARD_LILYGO_TDISPLAY_S3` flag is what selects the board: `src/BoardConfig.h` uses it (via the preprocessor) to pull in the matching LovyanGFX panel configuration from `src/boards/lilygo_tdisplay_s3.h`.

### Available environments

| Environment (`-e` name) | Board flag | Board |
|---|---|---|
| `lilygo-tdisplay-s3` | `BOARD_LILYGO_TDISPLAY_S3` | LilyGO T-Display-S3 |
| `lilygo-tdisplay-s3-amoled` | `BOARD_LILYGO_TDISPLAY_S3_AMOLED` | LilyGO T-Display-S3 AMOLED |
| `lilygo-tdisplay` | `BOARD_LILYGO_TDISPLAY` | LilyGO T-Display |
| `lilygo-t-qt` | `BOARD_LILYGO_TQT` | LilyGO T-QT |
| `lilygo-t-hmi` | `BOARD_LILYGO_T_HMI` | LilyGO T-HMI |
| `m5stickc-plus` | `BOARD_M5STICKC_PLUS` | M5StickC Plus / Plus2 |
| `m5stack-core` | `BOARD_M5STACK_CORE` | M5Stack Core / Basic / Fire |
| `m5stack-core2` | `BOARD_M5STACK_CORE2` | M5Stack Core2 / CoreS3 |
| `sunton-2432s028` | `BOARD_SUNTON_2432S028` | Sunton ESP32-2432S028 (CYD) |
| `waveshare-s3-round` | `BOARD_WAVESHARE_S3_ROUND` | Waveshare S3 Round 1.28" |
| `lilygo-t-watch-2020` | `BOARD_LILYGO_TWATCH_2020` | LilyGO T-Watch 2020 |
| `lilygo-t-watch-s3` | `BOARD_LILYGO_TWATCH_S3` | LilyGO T-Watch S3 |

The default environment is `lilygo-tdisplay-s3` (set under `[platformio]`).

---

## Building a Specific Board

To build firmware for one board, pass the environment name with `-e`:

```bash
pio run -e lilygo-tdisplay-s3
```

The compiled firmware appears under `.pio/build/<env>/` (e.g. `.pio/build/lilygo-tdisplay-s3/firmware.bin`).

---

## Uploading to a Board

Connect the board via USB and run:

```bash
pio run -e lilygo-tdisplay-s3 -t upload
```

This builds (if needed) and flashes the firmware to the connected board. PlatformIO auto-detects the serial port on most systems.

> **ESP32-S3 port not detected?** Hold **BOOT**, press **RESET**, then release **BOOT**. This puts the USB serial chip into a mode the uploader can see. See the Troubleshooting guide for details.

### Specifying the serial port manually

If auto-detection fails, set the port explicitly:

```bash
pio run -e lilygo-tdisplay-s3 -t upload --upload-port /dev/ttyUSB0
```

On Windows, the port looks like `COM3`. On macOS, `/dev/cu.SLAB_USBtoUART` or `/dev/cu.usbmodem*`.

---

## Serial Monitor

Open a serial monitor at 115200 baud to see boot logs, Wi-Fi connection status, cluster login, and parsed spot lines:

```bash
pio device monitor -e lilygo-tdisplay-s3
```

Or combined with upload:

```bash
pio run -e lilygo-tdisplay-s3 -t upload -t monitor
```

Exit the monitor with `Ctrl+]` (or `Ctrl+C` on some setups).

---

## Building All Environments

To build firmware for every supported board (useful before submitting a pull request):

```bash
pio run
```

This compiles all 12 environments. The CI workflow (`.github/workflows/build-flash.yml`) does exactly this on every push to `main`.

To clean all build artifacts:

```bash
pio run -t clean
```

> Building all 12 environments the first time downloads the ESP32 Arduino framework and LovyanGFX once, then compiles each board. Expect the first full build to take several minutes.

---

## VS Code + PlatformIO IDE Workflow

1. Open the project folder in VS Code (`File > Open Folder`).
2. The PlatformIO IDE extension detects `platformio.ini` and lists all environments in the bottom toolbar.
3. Use the **Project Tasks** panel (the PlatformIO sidebar) to:
   - Build a specific environment: expand the env, then **General > Build**.
   - Upload: **General > Upload**.
   - Monitor: **General > Monitor**.
4. The default environment (`lilygo-tdisplay-s3`) is used for the toolbar's quick buttons. Switch environments from the env dropdown in the bottom status bar.

---

## Common Build Errors and Solutions

### `Missing include/config.h`

```
#error "Missing include/config.h. Copy include/config.example.h to include/config.h."
```

**Cause:** `include/config.h` does not exist.

**Fix:**

```bash
cp include/config.example.h include/config.h
```

### `platformio: command not found` / `pio: command not found`

**Cause:** PlatformIO Core is not installed or not on your `PATH`.

**Fix:** Install it with `pip install platformio`, or use the VS Code extension which bundles it. If you installed via `pip` and the command is missing, ensure Python's `Scripts`/`bin` directory is on your `PATH`.

### `intelhex` / `esptool` errors during upload

```
ModuleNotFoundError: No module named 'intelhex'
```

**Cause:** A supporting Python package is missing from PlatformIO's internal Python.

**Fix:**

```bash
pip install intelhex esptool
```

Or, if using PlatformIO's isolated environment:

```bash
pio pkg install --tool esptool
```

### LovyanGFX not found / library download fails

**Cause:** First build did not finish downloading dependencies, or there is a network issue.

**Fix:** Run `pio pkg update` or delete `.pio/libdeps` and rebuild. Check your network/proxy settings if behind a corporate firewall.

### Out of memory / region `iram0_1` full

**Cause:** A board with limited IRAM (e.g. original ESP32 without PSRAM) is being built with too many features, or an incompatible library version was pulled.

**Fix:** Ensure you are building the correct environment for your hardware. Clean and rebuild: `pio run -e <env> -t clean && pio run -e <env>`.

### USB port not detected (ESP32-S3)

**Cause:** ESP32-S3 boards use native USB CDC and may not enumerate until forced into bootloader mode.

**Fix:** Hold **BOOT**, press **RESET**, release **BOOT**. The port should appear. On some boards you may also need `ARDUINO_USB_CDC_ON_BOOT=1` (already set in the relevant environments).

### Wrong board environment flashed

**Cause:** You built and uploaded the wrong `-e` environment for your hardware, so the display pins do not match.

**Fix:** Rebuild and upload with the correct environment name. The board name is shown on the splash screen at boot, so you can confirm which firmware is running.

---

## Next Steps

- **Adding a new board:** See [08-adding-new-board.md](08-adding-new-board.md).
- **Troubleshooting runtime issues:** See [09-troubleshooting.md](09-troubleshooting.md).
- **Architecture and data flow:** See [10-firmware-architecture.md](10-firmware-architecture.md).
