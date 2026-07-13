# Web Admin Setup (Captive Portal)

The 9M2PJU ESP32 DX Cluster Client includes a built-in web administration
interface that runs as a Wi-Fi access point with a captive portal. You use it
once, during initial setup, to give the device your Wi-Fi credentials and DX
cluster login details. After that, the device boots straight into normal mode
and connects on its own.

This guide explains when setup mode activates, how to reach the portal, every
field on the configuration form, and how to recover when something goes wrong.

---

## When Setup Mode Activates

The firmware enters setup mode (and starts the captive portal) in any of the
following situations:

| Trigger | What happens |
| --- | --- |
| **First boot** (no Wi-Fi SSID saved in NVS) | The device has never been configured, so it goes straight to setup mode. |
| **BOOT button held during power-on / reset** | Holding GPIO 0 LOW at boot forces setup mode regardless of saved settings. |
| **Wi-Fi unreachable after 30 seconds** | If the configured network cannot be joined within the 30-second timeout, the device falls back to setup mode so you can correct the credentials. |

The decision is made in `setup()` in `src/main.cpp`:

```cpp
if (buttonHeld || !hasWifi) {
  startConfigMode();
} else {
  startNormalMode();   // falls back to config mode on Wi-Fi failure
}
```

While in setup mode the device:

- Starts a soft Wi-Fi access point (AP mode).
- Runs a tiny DNS server that resolves every hostname to the AP address.
- Serves the configuration page over HTTP on port 80.
- Displays **SETUP MODE**, the AP name, and the portal URL on the screen.

---

## The Access Point

When setup mode is active the device advertises an open Wi-Fi network named:

```
9M2PJU-DXCluster-XXXX
```

The `XXXX` suffix is derived from the last two bytes of the ESP32's factory
MAC address (printed in uppercase hex), so every device gets a unique,
predictable AP name. For example, a board whose MAC ends in `3A:7F` will
broadcast:

```
9M2PJU-DXCluster-3A7F
```

The AP has no password. The device assigns itself the static IP address
`192.168.4.1` with a `/24` subnet, which is the conventional ESP32 soft-AP
address.

> The AP name is generated in `makeApName()` in `src/main.cpp` from
> `ESP.getEfuseMac()`.

---

## Connecting to the Portal

You can use any phone, tablet, or computer with Wi-Fi.

1. On your device, open the Wi-Fi settings.
2. Find and join the network named `9M2PJU-DXCluster-XXXX` (no password).
3. Wait a few seconds. Most modern devices detect the captive portal
   automatically and pop up a browser window pointing at the configuration
   page.
4. If the portal does not pop up, open a browser manually and go to:

   ```
   http://192.168.4.1/
   ```

The captive portal works by running a DNS server on UDP port 53 that returns
`192.168.4.1` for every query, and by redirecting the common captive-portal
detection URLs (`/generate_204`, `/gen_204`, `/hotspot-detect.html`,
`/connecttest.txt`, `/fwlink`) plus any other unknown path back to the root
page. This is implemented in `src/WebAdmin.cpp`.

---

## The Configuration Form

The portal serves a single mobile-friendly HTML page (stored in PROGMEM) with
a form that POSTs to `/save`. The fields are grouped into three sections plus
an advanced section.

### Wi-Fi

| Field | HTML name | Required | Description |
| --- | --- | --- | --- |
| Network (SSID) | `ssid` | Yes | The Wi-Fi network to join at boot. |
| Scan for networks | (button) | — | Triggers a live Wi-Fi scan; results appear in a drop-down you can pick from. Calls `GET /scan`. |
| Password | `wifipass` | No | The Wi-Fi password. Leave blank for open networks. |

### DX Cluster Login

| Field | HTML name | Required | Description |
| --- | --- | --- | --- |
| Callsign | `callsign` | Yes | Your amateur radio callsign, sent when the cluster presents a `login:` prompt. |
| Callsign Password | `callpass` | No | Sent only when the cluster asks for `Password:` / `Passcode:`. Leave blank if your cluster does not require one. |

### DX Cluster Server

| Field | HTML name | Required | Default | Description |
| --- | --- | --- | --- | --- |
| Host / Address | `host` | Yes | `9m2pju.hamradio.my` | Hostname or IP of the DXSpider telnet server. |
| Port | `port` | Yes | `7300` | TCP port (1-65535). DXSpider telnet conventionally uses 7300. |

### Advanced

| Field | HTML name | Required | Description |
| --- | --- | --- | --- |
| Post-login command | `postlogin` | No | A single command sent once after login completes, e.g. `set/dx` or a filter line. Leave blank if unsure. |

> The advanced section is collapsed by default. Tap **Advanced settings**
> to expand it.

---

## Saving and Rebooting

When you press **Save & Reboot** the browser POSTs the form to `/save`. The
firmware:

1. Reads each field from the POST body into the in-memory `AppConfig`.
2. Applies safe defaults for empty required fields (port `7300`, host
   `9m2pju.hamradio.my`).
3. Calls `AppConfig::save()`, which writes every field to NVS under the
   namespace `dxcluster`.
4. Sets an internal reboot flag.
5. On the next `loop()` iteration, waits 500 ms and calls `ESP.restart()`.

The browser shows a confirmation message ("Saved! Rebooting..."). After the
reboot the device leaves AP mode, joins your Wi-Fi, and connects to the DX
cluster. Reconnect your phone/computer to your normal Wi-Fi network.

---

## Forcing Setup Mode

If the device is already configured and running normally, you can force it
back into setup mode to change settings:

1. Keep the **BOOT** button (GPIO 0) pressed.
2. While holding BOOT, press the **RESET** button (or power-cycle the board).
3. Continue holding BOOT for about a second after reset, then release.

The firmware samples BOOT early in `setup()` (`isBootButtonHeld()` in
`src/main.cpp`) and routes to `startConfigMode()` when it reads LOW. The
existing NVS values are pre-filled as defaults, so you only need to change the
fields you want to update.

---

## How Config Is Stored in NVS

All runtime settings are persisted in the ESP32's Non-Volatile Storage (NVS)
under the namespace **`dxcluster`**. NVS survives reboots and power loss, and
is managed by the Arduino-ESP32 `Preferences` library.

The mapping between web-admin fields and NVS keys (see `src/AppConfig.cpp`)
is:

| NVS key | Type | Web-admin field |
| --- | --- | --- |
| `wifi_ssid` | string | Network (SSID) |
| `wifi_pass` | string | Password |
| `callsign` | string | Callsign |
| `call_pass` | string | Callsign Password |
| `cluster_host` | string | Host / Address |
| `cluster_port` | uint16 | Port |
| `post_login` | string | Post-login command |
| `ntp1` | string | (compile-time default only) |
| `ntp2` | string | (compile-time default only) |
| `tz_offset` | int32 | (compile-time default only) |
| `dst_offset` | int32 | (compile-time default only) |
| `reconnect_ms` | uint32 | (compile-time default only) |
| `brightness` | uint8 | (compile-time default only) |

On every boot `AppConfig::load()` reads each key from NVS. If a key is empty
or missing, the compile-time default from `include/config.h` is used instead.
This means the web admin only needs to set the user-facing fields; the rest
fall back to sensible defaults. See [06-configuration.md](06-configuration.md)
for the full reference.

`AppConfig::reset()` clears the entire `dxcluster` namespace, which returns
the device to its first-boot state.

---

## Troubleshooting

### The captive portal does not pop up automatically

Some devices (especially desktops, and certain Android versions) do not
trigger the captive-portal browser. Workarounds:

- Open any browser and navigate to `http://192.168.4.1/` manually.
- If the browser redirects you elsewhere, try an explicit URL such as
  `http://example.com/` — the on-device DNS will resolve it to the portal.
- Turn mobile data off before connecting, so the phone does not try to use
  cellular for the captive-portal check.
- Try a different browser or device.

### Cannot connect to the `9M2PJU-DXCluster-XXXX` AP

- Move closer to the device; the soft-AP range is short.
- Make sure no other device is already connected and saturating the AP
  (the soft-AP is intended for a single client at a time).
- Forget the network in your Wi-Fi settings and rejoin — occasionally the
  OS caches an old connection attempt.
- Confirm the device is actually in setup mode: the screen should show
  **SETUP MODE** and the AP name. If it shows spots instead, force setup
  mode (hold BOOT + RESET).

### Forgot the saved settings / device will not boot into setup

- Use the **hold BOOT + press RESET** method described above. This forces
  setup mode on the next boot regardless of what is stored in NVS.
- From the portal you can re-enter all fields and save again, which
  overwrites the old NVS values.
- As a last resort, flash the firmware with `pio run -t erase` (or
  `esptool.py erase_flash`) to wipe NVS entirely; the next boot will be a
  clean first-boot setup.

### Saved but the device still shows SETUP MODE

This usually means the Wi-Fi credentials you entered are wrong, or the
network is out of range. After saving, the device tries to join your Wi-Fi
for 30 seconds; if it fails, it falls back to setup mode so you can correct
the SSID/password. Reconnect to the AP and re-save with the correct details.

### Changed the cluster host but nothing happened

Settings only take effect after the reboot that follows a save. If you
closed the browser before the reboot completed, just wait — the device
reboots on its own a few hundred milliseconds after saving.
