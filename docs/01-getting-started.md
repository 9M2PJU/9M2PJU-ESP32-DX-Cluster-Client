# Getting Started

> From zero to live DX spots on your desk in about five minutes. No software
> to install, no drivers, no command line required for the user path.

This guide walks you through flashing the **9M2PJU ESP32 DX Cluster Client**
firmware onto a supported ESP32 board, configuring it from your phone, and
seeing your first DX spots appear on the screen.

---

## Table of contents

- [What you need](#what-you-need)
- [Choose your path](#choose-your-path)
- [Path A: Web Flasher (recommended for users)](#path-a-web-flasher-recommended-for-users)
- [Path B: PlatformIO (for developers)](#path-b-platformio-for-developers)
- [First boot: setup mode](#first-boot-setup-mode)
- [Configuring via the web admin UI](#configuring-via-the-web-admin-ui)
- [Seeing your first spots](#seeing-your-first-spots)
- [Boot flow diagram](#boot-flow-diagram)
- [Next steps](#next-steps)

---

## What you need

Before you start, gather the following:

| Item | Details |
| :--- | :--- |
| **ESP32 board with a screen** | Any of the 12 supported boards. See [Supported Boards](02-supported-boards.md) for the full list. |
| **USB data cable** | A cable that carries data, not charge-only. Many cheap cables only supply power. If your computer does not "see" the board when you plug it in, suspect the cable first. |
| **Computer with Chrome or Edge** | Required only for the web flasher. Any Chromium-based browser with Web Serial support works (Chrome, Edge, Opera). Firefox and Safari are not supported by the web flasher. |
| **Wi-Fi network** | 2.4 GHz network (the ESP32 does not support 5 GHz-only networks). You need the SSID and password. |
| **Amateur radio callsign** | Sent to the DX cluster at login. Use your own callsign, or `N0CALL` as a placeholder to try things out. |
| **DX cluster server details** | Hostname or IP and port (default `7300`). The project's companion node is `9m2pju.hamradio.my:7300`, but any DXSpider / CC Cluster / AR-Cluster node works. |

> **Tip:** If this is your first time, the **web flasher** path is by far the
> easiest. You can be looking at live spots within five minutes of opening the
> page.

---

## Choose your path

There are two ways to get the firmware onto your board:

| Path | Best for | Requires |
| :--- | :--- | :--- |
| **A. Web Flasher** | End users, first-timers, anyone who just wants it working | Chrome/Edge + USB cable |
| **B. PlatformIO** | Developers, contributors, anyone modifying the source | PlatformIO, Python, USB cable |

Both paths produce an identical device. Pick one and continue below.

---

## Path A: Web Flasher (recommended for users)

The web flasher installs the firmware directly from your browser using Web
Serial. No software installation, no drivers, no toolchain.

1. Open the **[Web Flasher](https://9m2pju.github.io/9M2PJU-ESP32-DX-Cluster-Client/)**
   in Chrome or Edge.
2. Connect your ESP32 board to your computer with a USB data cable.
3. On the page, find your board in the list and click **Connect**.
4. In the browser prompt, choose the serial port that matches your board
   (often labelled `USB JTAG/serial debug unit` on ESP32-S3, or `CP210x` /
   `CH340` on classic ESP32 boards) and click **Connect**.
5. Click **Install**. Confirm the erase prompt (this wipes any existing
   firmware on the board).
6. Wait for the flash to complete. This takes about 30 seconds. A progress
   bar is shown in the browser.
7. When it finishes, click **Connect** again to verify, or just close the
   page and unplug the board.

> **ESP32-S3 port not detected?** Hold the **BOOT** button, press and release
> **RESET**, then release **BOOT**. This puts the board into bootloader mode
> so the port appears. See [Web Flasher](03-web-flasher.md) for full
> troubleshooting.

For the complete step-by-step with screenshots and troubleshooting, see
[Web Flasher](03-web-flasher.md).

Once flashing is done, skip ahead to [First boot: setup mode](#first-boot-setup-mode).

---

## Path B: PlatformIO (for developers)

Use this path if you want to build from source, modify the firmware, or
contribute to the project.

### Prerequisites

- [PlatformIO Core](https://platformio.org/install) (or the PlatformIO IDE
  extension for VS Code)
- Python 3.x (installed with PlatformIO)
- Git

### Steps

```sh
# 1. Clone the repository
git clone https://github.com/9M2PJU/9M2PJU-ESP32-DX-Cluster-Client.git
cd 9M2PJU-ESP32-DX-Cluster-Client

# 2. Create your private config file (compile-time defaults)
cp include/config.example.h include/config.h

# 3. Edit include/config.h with your default values
#    (these can be overridden later via the web admin UI)
```

The `include/config.h` file is git-ignored, so your credentials are never
committed accidentally. It holds compile-time defaults such as
`WIFI_SSID`, `DX_CLUSTER_HOST`, and `DX_CLUSTER_LOGIN_CALLSIGN`. After the
first configuration via the web admin UI, values saved in NVS take
precedence over these defaults.

```sh
# 4. Build the firmware for your board
pio run -e lilygo-tdisplay-s3

# 5. Connect the board via USB, then upload
pio run -e lilygo-tdisplay-s3 -t upload

# 6. Monitor serial output (optional, useful for debugging)
pio device monitor -e lilygo-tdisplay-s3
```

Replace `lilygo-tdisplay-s3` with the environment name that matches your
board. The full list of environments is in [Supported Boards](02-supported-boards.md).

> **VS Code users:** Install the PlatformIO IDE extension, open this folder,
> pick your board environment in the sidebar, and click **Build** / **Upload**
> / **Monitor**.

> **First build is slow.** PlatformIO downloads the ESP32 platform and the
> LovyanGFX library on the first run, which can take several minutes.
> Subsequent builds are much faster.

The first build also produces the same firmware binary that the web flasher
ships, so the device behaves identically regardless of which path you chose.

Continue to [First boot: setup mode](#first-boot-setup-mode).

---

## First boot: setup mode

After flashing, the device boots into **setup mode** automatically on the
first boot, because no Wi-Fi credentials are saved yet. You do not need to
do anything special to trigger it.

### What you see on the screen

The display shows something like this (layout varies by screen size):

```
+------------------------------+
|    9M2PJU DX Cluster Client  |
|        SETUP MODE            |
|   9M2PJU-DXCluster-A1B2      |
|    Open: 192.168.4.1         |
|  Connect to the AP above     |
+------------------------------+
```

The device has started its own Wi-Fi access point (AP). The AP name is
`9M2PJU-DXCluster-XXXX`, where `XXXX` is the last four characters of the
chip's MAC address, so each device has a unique AP name.

### What to do

1. On your phone (or laptop), open Wi-Fi settings.
2. Connect to the `9M2PJU-DXCluster-XXXX` network. There is no password.
3. A **captive portal** page should pop up automatically on most phones.
   If it does not, open a web browser and go to **http://192.168.4.1/**.

> **Captive portal not popping up?** Some phones (especially older Android
> versions) do not trigger the captive portal detection reliably. Just open
> any browser and navigate to `http://192.168.4.1/` manually. Make sure you
> are still connected to the `9M2PJU-DXCluster-XXXX` Wi-Fi.

You are now in the web admin UI. Continue to the next section.

---

## Configuring via the web admin UI

The web admin page presents a configuration form. Fill it in as follows:

| Field | What to enter |
| :--- | :--- |
| **Wi-Fi SSID** | Your Wi-Fi network name. Tap **Scan for networks** to populate a list of nearby networks and pick yours. |
| **Wi-Fi Password** | Your Wi-Fi password. |
| **Callsign** | Your amateur radio callsign (e.g. `9M2XYZ`). The default placeholder is `N0CALL`. |
| **Callsign Password** | Only if your cluster requires one. Leave blank if unsure. The firmware only sends this if the cluster explicitly asks for a password. |
| **DX Cluster Host** | The cluster server hostname or IP, e.g. `9m2pju.hamradio.my`. |
| **DX Cluster Port** | The cluster telnet port. The DXSpider default is `7300`. |
| **Post-login command** | (Advanced) An optional command sent after login, e.g. `set/here` or `accept/spot`. Leave blank if you don't know what this is. |

After filling in the form, tap **Save & Reboot**.

### What happens next

1. The device saves your settings to **NVS** (non-volatile storage in flash
   memory). They survive reboots and firmware updates.
2. The device reboots.
3. It connects to your Wi-Fi (this takes a few seconds).
4. It syncs the UTC clock via NTP.
5. It opens a telnet connection to the DX cluster and logs in with your
   callsign.
6. Spots start arriving and appear on the screen.

If the Wi-Fi connection fails within 30 seconds, the device falls back to
setup mode so you can try again. See the [Boot flow diagram](#boot-flow-diagram)
below for the full decision tree.

> **Need to reconfigure later?** Hold the **BOOT** button while pressing
> **RESET** (or while powering on). The device boots into setup mode
> regardless of saved configuration. This is useful if you change your
> Wi-Fi password or want to point the device at a different cluster server.

---

## Seeing your first spots

Once the device is connected to Wi-Fi and the cluster, the display switches
to the normal spot view. The exact layout depends on your screen size and
shape.

### Wide / large panels (T-Display-S3, M5Stack Core, Sunton CYD, T-HMI)

```
+------------------------------+
| 9M2PJU DX Cluster  o  12:34Z |
+------------------------------+
| |14.074  JA1ABC               |
|  de 9M2XYZ  FT8, strong in EU |
| |21.300  VK2DEF               |
|  de 9M2ABC  big signal        |
| | 7.025  DL1AAA               |
|  de N0CALL  Europe opening    |
+------------------------------+
```

The header shows the project name, a connection indicator (`o` = connected),
and the UTC clock. Below it is a scrolling list of spots, newest first. Spot
frequencies are colour-coded by band for quick visual scanning.

### Small / round panels (T-QT, Waveshare round, T-Watch)

```
        9M2PJU DX Cluster
         12:34Z
      (   o   )
       14.074
       JA1ABC
       de 9M2XYZ
```

Small and round screens show a compact single-spot view with a connection
ring around the edge.

### Using the button

In normal mode, the **BOOT button** (GPIO 0) opens a command menu for
sending DX cluster commands without a computer:

| Action | Result |
| :--- | :--- |
| Short press (from spot view) | Opens the command menu |
| Short press (in menu) | Cycles to the next command |
| Long press (~1s hold) | Sends the highlighted command |
| Short press (on response) | Closes the response, returns to spots |
| 10s no input (in menu) | Auto-closes the menu |
| 8s no input (on response) | Auto-closes the response |

Available commands include `sh/dx`, `sh/dx 20`, `sh/dx/ft8`, `sh/dx/cw`,
`sh/dx/ssb`, `sh/wwv`, `sh/muf`, `sh/qtc`, `sh/ann`, `sh/u`, `sh/c`, and
`sh/h`. See the main README for the full list.

---

## Boot flow diagram

The diagram below shows every decision the firmware makes from power-on to
showing spots. Use it to understand what the device is doing at each stage,
or to debug when something does not behave as expected.

```
                    ┌─────────────────┐
                    │     Power On     │
                    └────────┬────────┘
                             │
                    ┌────────▼────────┐
                    │  Load NVS config │
                    │  (with defaults) │
                    └────────┬────────┘
                             │
              ┌──────────────▼──────────────┐
              │  BOOT button held?           │
              │  OR no Wi-Fi SSID saved?     │
              └──────┬──────────────┬───────┘
                     │ Yes          │ No
              ┌──────▼──────┐ ┌────▼─────────────┐
              │  Setup Mode  │ │  Normal Mode     │
              │  (AP + web)  │ │  Connect Wi-Fi   │
              └──────┬──────┘ └────┬──────────────┘
                     │             │
                     │      ┌──────▼──────────────┐
                     │      │ Wi-Fi connected      │
                     │      │ within 30s?          │
                     │      └──┬────────────┬─────┘
                     │         │Yes         │No
                     │    ┌────▼─────┐  ┌───▼──────────┐
                     │    │ NTP sync │  │ Fallback to  │
                     │    │ Telnet   │  │ Setup Mode   │
                     │    │ Spots!   │  └──┬───────────┘
                     │    └──────────┘     │
                     │                     │
              ┌──────▼─────────────────────▼──────┐
              │         Setup Mode                 │
              │  AP: 9M2PJU-DXCluster-XXXX         │
              │  Web: http://192.168.4.1/         │
              │  Save → Reboot → Normal Mode       │
              └────────────────────────────────────┘
```

**Key takeaways from the flow:**

- **Setup mode** is entered on first boot (no saved SSID) or whenever you
  hold BOOT during reset.
- **Normal mode** attempts to join Wi-Fi. If it cannot connect within 30
  seconds, it falls back to setup mode so you can fix the credentials.
- Once Wi-Fi is up, the device syncs UTC via NTP, then opens the telnet
  connection to the cluster and begins displaying spots.
- Automatic reconnect handles Wi-Fi drops and cluster server restarts
  without intervention.

---

## Next steps

Now that your device is showing spots, you may want to:

- Read the **[Supported Boards](02-supported-boards.md)** reference to
  understand your hardware and discover other boards you can flash.
- Learn how the **[Web Flasher](03-web-flasher.md)** works under the hood,
  including troubleshooting and the CI pipeline that publishes it.
- Explore the full **[Configuration](../README.md#configuration)** options
  for compile-time defaults and NVS-stored settings.
- Read the **[Troubleshooting](09-troubleshooting.md)** guide if anything
  is not working as expected.

Welcome to the cluster. Good DX.
