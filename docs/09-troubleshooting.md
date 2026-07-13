# Troubleshooting

This guide collects the most common issues users encounter with the 9M2PJU ESP32 DX Cluster Client, along with concrete solutions. It is organised by symptom. If you are building from source and hitting compile errors, see the [Developer Guide](07-developer-guide.md) as well.

---

## Table of Contents

- [Flashing via the web flasher](#flashing-via-the-web-flasher)
- [Setup AP and captive portal](#setup-ap-and-captive-portal)
- [Wi-Fi connection](#wi-fi-connection)
- [DX cluster connection](#dx-cluster-connection)
- [Spots not appearing](#spots-not-appearing)
- [Display issues](#display-issues)
- [Button menu](#button-menu)
- [Build failures](#build-failures)
- [Serial monitor debugging](#serial-monitor-debugging)

---

## Flashing via the web flasher

### The "Connect" or "Install" button does nothing

**Problem:** I open the web flasher page, click the install button, and nothing happens (no port picker, no progress).

**Cause:** The browser does not support Web Serial, which the ESP Web Tools flasher relies on.

**Solution:** Use a Chromium-based browser: **Google Chrome, Microsoft Edge, Opera, or Brave**. Firefox and Safari do not support Web Serial. On Linux, Chrome must be installed (not Chromium Snap, which is sandboxed and cannot access serial ports). On macOS, Chrome and Edge both work.

### The serial port does not appear in the picker

**Problem:** The port picker is empty or my board's port is not listed.

**Solutions, in order:**

1. **Use a data USB cable.** Many cheap cables are charge-only. Try a cable you know carries data.
2. **Plug directly into the computer**, not through a USB hub or dock.
3. **Install the USB driver.** Original ESP32 boards with a CP2102 need Silicon Labs CP210x drivers; CH340-based boards need WCH CH340 drivers. ESP32-S3 boards with native USB usually need no driver.
4. **ESP32-S3 BOOT button trick.** Hold **BOOT**, press **RESET**, then release **BOOT**. This forces the USB CDC interface into bootloader mode so the port enumerates. You may need to do this each time before flashing an S3 board.
5. **Close other serial programs.** A serial monitor or other tool holding the port open will prevent the browser from connecting.

### Flashing fails partway with a connection error

**Problem:** Flashing starts but then aborts with a timeout or "failed to write" error.

**Solutions:**

- Use a shorter, higher-quality USB cable.
- Avoid USB hubs and front-panel ports on desktop towers; use a rear motherboard port.
- Retry. Transient USB errors are common on some S3 boards. Re-do the BOOT/RESET trick.
- If it keeps failing, flash from PlatformIO CLI instead (`pio run -e <env> -t upload`).

---

## Setup AP and captive portal

### I cannot see the setup Wi-Fi network

**Problem:** After entering setup mode, I cannot find the `9M2PJU-DXCluster-XXXX` Wi-Fi network.

**Cause / Solution:**

- The AP name is `9M2PJU-DXCluster-` followed by four hex digits from the board's MAC address. Look for a network starting with `9M2PJU-DXCluster-`.
- Move closer to the device. The AP range is short.
- Confirm the device is actually in setup mode: the screen should show "SETUP MODE", the AP name, and `192.168.4.1`. If it shows the normal spot screen, hold **BOOT** and press **RESET** to force setup mode.
- Some phones hide "low-quality" networks. Check your Wi-Fi settings to ensure such networks are not being filtered out.

### The captive portal does not pop up automatically

**Problem:** I connect to the setup AP but no login page appears.

**Cause:** Captive-portal detection is handled by the OS/browser and is unreliable, especially on desktops.

**Solution:** Open a browser and navigate manually to **`http://192.168.4.1/`**. The config form loads directly. The firmware intercepts any HTTP request and redirects to this address, so typing any non-HTTPS URL (e.g. `http://example.com`) usually works too.

### The config page loads but saving does nothing

**Problem:** I fill in the form and click "Save & Reboot" but nothing happens or I get "Save failed".

**Solutions:**

- Make sure you are still connected to the `9M2PJU-DXCluster-` AP. If your phone auto-switched back to mobile data or another Wi-Fi network, the save request cannot reach the device.
- Disable mobile data while configuring so the phone routes the request over Wi-Fi.
- Try a different browser. Some mobile browsers block POST requests to captive-portal IPs.
- Check the serial monitor: after a successful save you should see `Config saved to NVS` followed by `Rebooting after config save...`.

---

## Wi-Fi connection

### The device never connects to my Wi-Fi

**Problem:** After saving config and rebooting, the device falls back to setup mode or stays on "Connecting to DX cluster...".

**Solutions:**

- **Wrong password.** Re-enter setup mode (hold BOOT + RESET) and retype the Wi-Fi password carefully. Passwords are case-sensitive.
- **5 GHz only network.** The ESP32 only supports 2.4 GHz Wi-Fi. Ensure your router broadcasts a 2.4 GHz SSID. Many dual-band routers use the same SSID for both bands; if so, temporarily disable 5 GHz or move far enough that only 2.4 GHz is reachable.
- **WPA3-only network.** The ESP32's support for WPA3 is limited. Use WPA2 or a WPA2/WPA3-mixed mode on your router.
- **Hidden SSID.** If your network is hidden, the ESP32 can still connect if the SSID is entered exactly, but some routers behave poorly. Try unhiding the SSID during setup.
- **MAC filtering / captive portal on the Wi-Fi.** If your network requires a web login or blocks unknown MAC addresses, the ESP32 will not be able to connect.
- **Signal strength.** If the device is too far from the access point, the connection times out (the firmware gives up after 30 seconds and falls back to setup mode).

### The device connects initially but drops repeatedly

**Cause:** Weak signal or an access point that aggressively kicks idle clients.

**Solution:** Move the device closer to the router. The firmware automatically reconnects if Wi-Fi drops, but repeated drops prevent the cluster connection from staying up.

---

## DX cluster connection

### The cluster never connects

**Problem:** Wi-Fi is connected but the display stays on "Connecting to DX cluster..." and no spots appear.

**Solutions:**

- **Wrong host or port.** Re-enter setup mode and verify the cluster host and port. The default is `9m2pju.hamradio.my` on port `7300`. Many DXSpider nodes use port `7300` or `23`; check the cluster's published connection details.
- **Firewall.** Some networks block outgoing telnet (port 7300/23). Try a different network (e.g. a phone hotspot) to rule this out.
- **Cluster is down.** DX cluster nodes occasionally go offline. Try connecting to the same host:port from a computer (`telnet host 7300` or `nc host 7300`) to confirm the server is reachable and responding.
- **DNS resolution.** If you entered a hostname, make sure it resolves. Try entering the IP address directly in setup mode as a test.

### The cluster connects but immediately disconnects

**Cause / Solution:**

- **Callsign rejected.** Some clusters reject callsigns that are not in their user database or that look invalid (`N0CALL` is often rejected). Enter your real amateur radio callsign.
- **Password required but not set.** If the cluster asks for a password and you left the callsign password blank, the login stalls. Re-enter setup and provide the password, or use a cluster that does not require one.
- **Too many connections.** Some clusters limit the number of simultaneous connections from the same IP or callsign. Wait a few minutes and retry.

---

## Spots not appearing

### Connected to the cluster but the list stays empty

**Problem:** The header shows a connected (blue) pulse, but the screen says "Waiting for DX spots..." indefinitely.

**Solutions:**

- **The cluster has no spots right now.** DX spots depend on band activity and other users submitting them. On a quiet band or a small cluster, spots can be sparse. Wait a few minutes, especially during off-peak hours.
- **Filter / `set/dx` issue.** Some clusters require you to enable DX spots with a command like `set/dx` after login. Use the button menu (short press to open, navigate to `sh/dx`, long press to send) to request recent spots manually. You can also set a post-login command (e.g. `set/dx`) in the advanced section of the setup form.
- **Callsign not fully logged in.** If the cluster is waiting for a password or a confirmation, it will not stream spots. Check the serial monitor for the login exchange.
- **Wrong cluster type.** The parser expects DXSpider-format lines (`DX de SPOTTER: FREQ DXCALL COMMENT`). A non-DXSpider cluster may emit a different format that the parser ignores.

### Spots appear but the comment is cut off

**Cause:** The display truncates long comments to fit the panel width. This is expected behaviour on small screens.

**Solution:** This is by design. Larger boards (320x240) show more of the comment. There is no setting to change truncation.

---

## Display issues

### The display is completely blank

**Solutions, in order:**

1. **Wrong board environment flashed.** The most common cause. If you flashed firmware built for a different board, the display pins do not match and nothing appears. Re-flash with the correct environment for your hardware. Check the splash screen serial output for the `BOARD_NAME`.
2. **T-Watch 2020 PMU not initialised.** The LilyGO T-Watch 2020 powers its display through an AXP202 PMU. The firmware enables LDO2 at boot, but only when built with the `lilygo-t-watch-2020` environment (which sets `BOARD_NEEDS_PMU_INIT`). Flashing the wrong environment leaves the display unpowered.
3. **Backlight too dim.** The default brightness is set in `config.h` (`DISPLAY_BACKLIGHT_BRIGHTNESS`). If it was set very low, the screen may look off. Rebuild with a higher value (0-255).
4. **Bad USB cable / insufficient power.** Some panels draw significant current. A weak USB port can cause the display to fail to initialise. Use a powered USB hub or a different port.
5. **T-Watch V3 variant.** The T-Watch 2020 V3 uses backlight pin GPIO15 instead of GPIO12. If you have a V3, add `-D TWATCH_BL_PIN=15` to the `lilygo-t-watch-2020` environment's `build_flags` and rebuild.

### The display is mirrored or rotated incorrectly

**Cause:** The panel offsets, invert flag, or rotation are wrong for your specific panel variant.

**Solution:** Adjust the panel config in `src/boards/<name>.h`:

- For upside-down images, change `BOARD_DISPLAY_ROTATION` in `BoardConfig.h` (0, 1, 2, or 3).
- For mirrored images, toggle `cfg.invert` or adjust `cfg.offset_x` / `cfg.offset_y`.
- For swapped colors (red/blue), toggle `cfg.rgb_order`.

See [08-adding-new-board.md](08-adding-new-board.md) for details.

### Colors look wrong (e.g. red and blue swapped)

**Cause:** The panel's RGB/BGR order differs from the config.

**Solution:** Toggle `cfg.rgb_order` in the board's panel config (`src/boards/<name>.h`) and rebuild.

---

## Button menu

### The button menu does not respond

**Problem:** Pressing the BOOT button does not open the command menu.

**Solutions:**

- **GPIO0 wiring.** The menu uses the BOOT button on GPIO0 (active LOW, internal pull-up). On most dev boards this is the BOOT button. If your board does not wire a button to GPIO0, the menu cannot work. Check your board's pinout.
- **Button not accessible.** Some boards enclose the BOOT button or require a specific press. On the T-Watch, the button is on the side. On M5Stack devices, use the side button labelled BOOT or the main button depending on the model.
- **You are in setup mode.** The menu only runs in normal mode. If the device is showing the setup screen, the button is used to detect setup mode entry, not menu navigation. Configure Wi-Fi first, reboot, then use the menu.
- **Debounce / press timing.** A "short press" is a press shorter than 800 ms; a "long press" is 800 ms or longer. Press and release quickly to open/navigate; hold to send. Very quick taps may be missed.

### The menu opens but I cannot send a command

**Cause:** The cluster is not connected. The firmware refuses to send when not connected and closes the menu.

**Solution:** Wait for the header pulse to turn blue (connected) before opening the menu. If the cluster never connects, see [DX cluster connection](#dx-cluster-connection).

---

## Build failures

### `Missing include/config.h`

```
#error "Missing include/config.h. Copy include/config.example.h to include/config.h."
```

**Fix:**

```bash
cp include/config.example.h include/config.h
```

### `platformio: command not found` or `pio: command not found`

**Cause:** PlatformIO Core is not installed or not on `PATH`.

**Fix:** Install with `pip install platformio`, or use the VS Code PlatformIO IDE extension. If installed via `pip`, ensure Python's `Scripts` (Windows) or `bin` (macOS/Linux) directory is on your `PATH`.

### `ModuleNotFoundError: No module named 'intelhex'` (or `esptool`)

**Cause:** A supporting Python package needed for flashing is missing.

**Fix:**

```bash
pip install intelhex esptool
```

### LovyanGFX not found

**Cause:** The library failed to download on first build, or `.pio/libdeps` is corrupted.

**Fix:** Delete `.pio/libdeps` and rebuild. If behind a proxy, configure PlatformIO's proxy settings.

### Build runs out of memory (`region 'iram0_1' overflowed`)

**Cause:** The target board has limited IRAM, or an incompatible library version was pulled.

**Fix:** Ensure you are building the correct environment for your hardware. Clean and rebuild:

```bash
pio run -e <env> -t clean && pio run -e <env>
```

### Upload fails with a port error

**Cause:** The serial port is in use, the wrong port is selected, or the board is not in bootloader mode.

**Fix:** Close any serial monitor. For ESP32-S3, do the BOOT/RESET trick. Specify the port explicitly with `--upload-port`.

---

## Serial monitor debugging

The firmware prints detailed status to the serial console at **115200 baud**. This is the single most useful debugging tool.

### How to open the monitor

```bash
pio device monitor -e <env>
```

Or from VS Code: PlatformIO Project Tasks > env > General > Monitor.

### What to look for

| Log line | Meaning |
|---|---|
| `9M2PJU DX Cluster Client` + `BOARD_NAME` + `Booting...` | Splash. Confirms the correct board firmware is running. |
| `BOOT button: HELD` / `no` | Whether the BOOT button was held at reset. `HELD` forces setup mode. |
| `WiFi configured: yes` / `no` | Whether an SSID is stored in NVS. `no` forces setup mode. |
| `=== Setup mode ===` | The device entered the captive portal. |
| `=== Normal mode ===` | The device is trying to connect to Wi-Fi and the cluster. |
| `Connecting Wi-Fi....` | Wi-Fi connection in progress. Dots appear every 250 ms. |
| `Wi-Fi connected: 192.168.x.x` | Wi-Fi succeeded. |
| `Wi-Fi connect timed out` | Wi-Fi failed after 30 s; the device falls back to setup mode. |
| `UTC clock synced` | NTP succeeded; the header clock will show real UTC. |
| `Connecting telnet host:7300` | Attempting a cluster connection. |
| `DX cluster connected` | Telnet connected to the cluster. |
| `DX cluster connect failed` | Telnet connection refused or timed out. |
| `Sent login: <callsign>` | The callsign was sent in response to a login prompt. |
| `Sent callsign password` | A password was sent (only if configured). |
| Raw cluster lines | Every non-empty line received from the cluster is echoed. Spot lines start with `DX de`. |
| `Menu: sending '<command>'` | A command was sent via the button menu. |
| `Menu: not connected, cannot send` | The menu was used while disconnected. |
| `Config saved to NVS` / `Rebooting after config save...` | The setup form was submitted successfully. |

### Common patterns

- **Stuck at `Connecting Wi-Fi....` then `Wi-Fi connect timed out`:** Wi-Fi credentials or network issue. See [Wi-Fi connection](#wi-fi-connection).
- **`DX cluster connect failed` repeating:** Host/port/firewall issue. See [DX cluster connection](#dx-cluster-connection).
- **Connected but no `DX de` lines in the log:** The cluster is not sending spots. Try `sh/dx` from the menu, or check that the cluster accepts your callsign.
- **No output at all:** Wrong baud rate (must be 115200), wrong port, or the board is not sending serial (ESP32-S3 USB CDC requires `ARDUINO_USB_CDC_ON_BOOT=1`, which the environments set automatically).

> **Tip:** If you do not have a serial connection handy, the display itself reports the most important state: the header pulse color (blue = connected, orange = disconnected), the UTC clock (populated once NTP syncs), and the empty-state message ("Connecting to DX cluster..." vs "Waiting for DX spots...").
