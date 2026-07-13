# Supported Boards

> The 9M2PJU ESP32 DX Cluster Client runs on 22 popular ESP32 boards with a
> built-in screen. One PlatformIO environment is defined per board in
> `platformio.ini`, and a matching LovyanGFX panel configuration lives in
> `src/boards/`. Pick the environment that matches your hardware.

This document is the complete reference for every supported board: chip,
display controller, resolution, bus type, special hardware notes, and
recommendations for which board to choose for different use cases.

---

## Table of contents

- [Comparison table](#comparison-table)
- [Board reference](#board-reference)
  - [LilyGO T-Display-S3](#lilygo-t-display-s3)
  - [LilyGO T-Display-S3 AMOLED](#lilygo-t-display-s3-amoled)
  - [LilyGO T-Display](#lilygo-t-display)
  - [LilyGO T-QT](#lilygo-t-qt)
  - [LilyGO T-HMI](#lilygo-t-hmi)
  - [M5StickC Plus / Plus2](#m5stickc-plus--plus2)
  - [M5Stack Core / Basic / Fire](#m5stack-core--basic--fire)
  - [M5Stack Core2 / CoreS3](#m5stack-core2--cores3)
  - [Sunton ESP32-2432S028R (CYD)](#sunton-esp32-2432s028r-cyd)
  - [Waveshare ESP32-S3-Touch-LCD-1.28](#waveshare-esp32-s3-touch-lcd-128)
  - [LilyGO T-Watch 2020](#lilygo-t-watch-2020)
  - [LilyGO T-Watch S3](#lilygo-t-watch-s3)
  - [LilyGO T-Deck](#lilygo-t-deck)
  - [Heltec WiFi Kit 32 (V1/V2)](#heltec-wifi-kit-32-v1v2)
  - [Heltec WiFi Kit 32 V3](#heltec-wifi-kit-32-v3)
  - [Heltec WiFi LoRa 32 (V1/V2)](#heltec-wifi-lora-32-v1v2)
  - [Heltec WiFi LoRa 32 V3](#heltec-wifi-lora-32-v3)
  - [Heltec Wireless Stick / Stick Lite](#heltec-wireless-stick--stick-lite)
  - [Heltec Wireless Tracker](#heltec-wireless-tracker)
- [Choosing a board by use case](#choosing-a-board-by-use-case)
- [How board support works](#how-board-support-works)

---

## Comparison table

| Environment (`-e`) | Board | Chip | Display controller | Resolution | Bus | Flash | PSRAM | Special notes |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `lilygo-tdisplay-s3` | LilyGO T-Display-S3 | ESP32-S3 | ST7789 | 320x170 | 8-bit parallel | 16 MB | OPI PSRAM | USB CDC on boot; fastest refresh of the SPI boards |
| `lilygo-tdisplay-s3-amoled` | LilyGO T-Display-S3 AMOLED | ESP32-S3 | RM67162 | 536x240 | QSPI | 16 MB | OPI PSRAM | AMOLED panel; QSPI bus; high contrast |
| `lilygo-tdisplay` | LilyGO T-Display | ESP32 | ST7789 | 240x135 | SPI | 4 MB | none | Classic, low-cost, widely available |
| `lilygo-t-qt` | LilyGO T-QT | ESP32-S3 | GC9A01 | 128x128 | SPI | 4 MB | none | Tiny key-fob form factor; compact UI |
| `lilygo-t-hmi` | LilyGO T-HMI | ESP32-S3 | ILI9341 | 320x240 | SPI | 16 MB | OPI PSRAM | Touch panel; large readable display |
| `m5stickc-plus` | M5StickC Plus / Plus2 | ESP32-PICO | ST7789 | 240x135 | SPI | 4 MB | none | Built-in battery; wearable form factor; uses M5StickC lib |
| `m5stack-core` | M5Stack Basic / Core / Fire | ESP32 | ILI9341 | 320x240 | SPI | 4 MB | none (Fire has PSRAM) | Integrated buttons and speaker |
| `m5stack-core2` | M5Stack Core2 / CoreS3 | ESP32 | ILI9341 | 320x240 | SPI | 16 MB | none (Core2 has PSRAM) | Touch panel; AXP202 PMU on Core2 |
| `sunton-2432s028` | Sunton ESP32-2432S028R (CYD) | ESP32 | ILI9341 | 320x240 | SPI | 4 MB | none | The "Cheap Yellow Display"; lowest-cost 320x240 option |
| `waveshare-s3-round` | Waveshare ESP32-S3-Touch-LCD-1.28 | ESP32-S3 | GC9A01 | 240x240 (round) | SPI | 8 MB | none | Round display; compact single-spot UI with connection ring |
| `lilygo-t-watch-2020` | LilyGO T-Watch 2020 | ESP32 | ST7789 | 240x240 | SPI | 16 MB | none | AXP202 PMU init required for display power; wearable watch |
| `lilygo-t-watch-s3` | LilyGO T-Watch S3 | ESP32-S3 | ST7789V3 | 240x240 | SPI | 16 MB | OPI PSRAM | AXP2101 PMU on board; backlight driven directly from GPIO 45 so no PMU init needed for display |
| `lilygo-tdeck` | LilyGO T-Deck | ESP32-S3 | ST7789 | 320x240 | SPI | 16 MB | OPI PSRAM | Keyboard + trackball; peripheral power-enable on GPIO 10 |
| `heltec-wifi-kit-32` | Heltec WiFi Kit 32 | ESP32 | SSD1306 | 128x64 | I2C | 4 MB | none | 0.96" OLED; SDA=4, SCL=15, RST=16 |
| `heltec-wifi-kit-32-v2` | Heltec WiFi Kit 32 V2 | ESP32 | SSD1306 | 128x64 | I2C | 8 MB | none | 0.96" OLED; same pins as V1; 8 MB flash |
| `heltec-wifi-kit-32-v3` | Heltec WiFi Kit 32 V3 | ESP32-S3 | SSD1306 | 128x64 | I2C | 8 MB | none | 0.96" OLED; SDA=17, SCL=18, RST=21; USB CDC |
| `heltec-wifi-lora-32` | Heltec WiFi LoRa 32 | ESP32 | SSD1306 | 128x64 | I2C | 4 MB | none | 0.96" OLED + SX1276 LoRa (unused); same OLED pins as WiFi Kit 32 |
| `heltec-wifi-lora-32-v2` | Heltec WiFi LoRa 32 V2 | ESP32 | SSD1306 | 128x64 | I2C | 8 MB | none | 0.96" OLED + SX1276 LoRa (unused); 8 MB flash |
| `heltec-wifi-lora-32-v3` | Heltec WiFi LoRa 32 V3 | ESP32-S3 | SSD1306 | 128x64 | I2C | 8 MB | none | 0.96" OLED + SX1262 LoRa (unused); SDA=17, SCL=18, RST=21; USB CDC |
| `heltec-wireless-stick` | Heltec Wireless Stick | ESP32 | SSD1306 | 64x32 | I2C | 8 MB | none | 0.49" OLED + SX1276 LoRa (unused); compact UI |
| `heltec-wireless-stick-lite` | Heltec Wireless Stick Lite | ESP32 | SSD1306 | 64x32 | I2C | 4 MB | none | 0.49" OLED + SX1276 LoRa (unused); smallest screen |
| `heltec-wireless-tracker` | Heltec Wireless Tracker | ESP32-S3 | ST7735 | 160x80 | SPI | 8 MB | none | 0.96" TFT + SX1262 LoRa + UC6580 GNSS (both unused); USB CDC |

> **Don't see your board?** Adding a new board takes about 30 lines of code.
> See [Adding a New Board](08-adding-new-board.md).

---

## Board reference

Each subsection below covers one board in detail. The environment name is
what you pass to PlatformIO with `-e`, and it is also the name used in the
web flasher board list.

### LilyGO T-Display-S3

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-tdisplay-s3` |
| Chip | ESP32-S3 |
| Display controller | ST7789 |
| Resolution | 320 x 170 |
| Bus type | 8-bit parallel |
| Flash | 16 MB |
| PSRAM | OPI PSRAM (`BOARD_HAS_PSRAM`) |
| Build flag | `BOARD_LILYGO_TDISPLAY_S3` |
| Board config | `src/boards/lilygo_tdisplay_s3.h` |

**Notes:** The T-Display-S3 uses an 8-bit parallel bus, which gives it the
fastest display refresh of the SPI-based boards. It is the default
environment in `platformio.ini`. USB CDC is enabled on boot
(`ARDUINO_USB_CDC_ON_BOOT=1`), so the USB port appears as a virtual serial
port without needing a USB-to-UART bridge. OPI PSRAM is enabled and
available for the framebuffer.

**Best for:** A desk display. The 320x170 panel is large enough for the
scrolling spot list, and the parallel bus keeps redraws smooth.

---

### LilyGO T-Display-S3 AMOLED

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-tdisplay-s3-amoled` |
| Chip | ESP32-S3 |
| Display controller | RM67162 |
| Resolution | 536 x 240 |
| Bus type | QSPI |
| Flash | 16 MB |
| PSRAM | OPI PSRAM (`BOARD_HAS_PSRAM`) |
| Build flag | `BOARD_LILYGO_TDISPLAY_S3_AMOLED` |
| Board config | `src/boards/lilygo_tdisplay_s3_amoled.h` |

**Notes:** This is the only AMOLED panel in the supported set, and the only
board that uses a **QSPI** bus. The RM67162 controller drives a 536x240
AMOLED with deep blacks and high contrast. QSPI pins are
`SCK=39, CS=40, D0=46, D1=38, D2=45, D3=48`. USB CDC and OPI PSRAM are
enabled. Because the panel is wider than it is tall (536x240 in native
orientation), the UI renders in landscape.

**Best for:** A premium desk display where contrast and brightness matter.
The AMOLED makes the band colour-coding on spot frequencies really pop.

---

### LilyGO T-Display

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-tdisplay` |
| Chip | ESP32 (classic) |
| Display controller | ST7789 |
| Resolution | 240 x 135 |
| Bus type | SPI |
| Flash | 4 MB |
| PSRAM | none |
| Build flag | `BOARD_LILYGO_TDISPLAY` |
| Board config | `src/boards/lilygo_tdisplay.h` |

**Notes:** The original LilyGO T-Display is one of the most popular and
affordable ESP32-with-screen boards. It uses a classic ESP32 (no PSRAM, no
USB CDC) with a 240x135 ST7789 over SPI. The small panel uses the compact
single-spot UI layout.

**Best for:** A low-cost entry point. If you already have a T-Display in
your parts bin, this is the fastest way to try the project.

---

### LilyGO T-QT

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-t-qt` |
| Chip | ESP32-S3 |
| Display controller | GC9A01 |
| Resolution | 128 x 128 |
| Bus type | SPI |
| Flash | 4 MB |
| PSRAM | none |
| Build flag | `BOARD_LILYGO_TQT` |
| Board config | `src/boards/lilygo_tqt.h` |

**Notes:** The T-QT is a tiny key-fob-sized board with a 0.8" 128x128
GC9A01 panel. USB CDC is enabled. The screen is the smallest in the
supported set, so the firmware uses the compact single-spot view with a
connection ring. No PSRAM is available, so the framebuffer lives in
internal RAM.

**Best for:** A minimal, pocketable display, or for testing the firmware
on the cheapest S3 board available.

---

### LilyGO T-HMI

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-t-hmi` |
| Chip | ESP32-S3 |
| Display controller | ILI9341 |
| Resolution | 320 x 240 |
| Bus type | SPI |
| Flash | 16 MB |
| PSRAM | OPI PSRAM (`BOARD_HAS_PSRAM`) |
| Build flag | `BOARD_LILYGO_T_HMI` |
| Board config | `src/boards/lilygo_t_hmi.h` |

**Notes:** The T-HMI is a 2.4" 320x240 ILI9341 board with a resistive touch
panel (touch is not used by this firmware, but the panel is present). USB
CDC and OPI PSRAM are enabled. The 320x240 resolution is enough for the
full scrolling spot list with frequency, callsign, spotter, and comment.

**Best for:** A desk display with the largest readable spot list at a
reasonable price. The touch panel leaves room for future interactive
features.

---

### M5StickC Plus / Plus2

| Property | Value |
| :--- | :--- |
| Environment | `m5stickc-plus` |
| Chip | ESP32-PICO |
| Display controller | ST7789 |
| Resolution | 240 x 135 |
| Bus type | SPI |
| Flash | 4 MB |
| PSRAM | none |
| Build flag | `BOARD_M5STICKC_PLUS` |
| Board config | `src/boards/m5stickc_plus.h` |
| Extra library | `m5stack/M5StickC@^0.0.7` |

**Notes:** The M5StickC Plus (and Plus2) is a wearable stick form factor
with a built-in battery and an IPX Wi-Fi antenna. Because it uses the
ESP32-PICO module with an AXP192 PMU, the build pulls in the `M5StickC`
library to handle power initialization. The 240x135 ST7789 panel uses the
compact single-spot UI.

> **Important:** This environment depends on the `M5StickC` library (added
> in `lib_deps`). The other environments do not. If you build only this
> environment, PlatformIO installs the extra dependency automatically.

**Best for:** A wearable or portable display. The built-in battery means
you can clip it to a lanyard and walk around the shack while watching
spots.

---

### M5Stack Core / Basic / Fire

| Property | Value |
| :--- | :--- |
| Environment | `m5stack-core` |
| Chip | ESP32 (classic) |
| Display controller | ILI9341 |
| Resolution | 320 x 240 |
| Bus type | SPI |
| Flash | 4 MB |
| PSRAM | none (Fire variant has PSRAM) |
| Build flag | `BOARD_M5STACK_CORE` |
| Board config | `src/boards/m5stack_core.h` |

**Notes:** The M5Stack Core (also sold as Basic and Fire) is the classic
grey box with a 2" 320x240 ILI9341, three front buttons, and a speaker.
The Fire variant includes PSRAM, but the firmware does not require it.
The 320x240 panel uses the full scrolling spot list layout.

**Best for:** A desk display, especially if you already own M5Stack
hardware. The three front buttons are not used by this firmware (the BOOT
button on the side is used for the command menu), but the form factor is
robust and self-contained.

---

### M5Stack Core2 / CoreS3

| Property | Value |
| :--- | :--- |
| Environment | `m5stack-core2` |
| Chip | ESP32 (classic) |
| Display controller | ILI9341 |
| Resolution | 320 x 240 |
| Bus type | SPI |
| Flash | 16 MB |
| PSRAM | none (Core2 has PSRAM on board) |
| Build flag | `BOARD_M5STACK_CORE2` |
| Board config | `src/boards/m5stack_core2.h` |

**Notes:** The Core2 and CoreS3 are the successors to the M5Stack Core,
with a capacitive touch panel, an AXP202 PMU (Core2) / AXP2101 PMU
(CoreS3), and a built-in battery. The display is the same 2" 320x240
ILI9341 as the original Core, so the UI layout is identical. The larger
flash (16 MB) leaves plenty of room for future features.

**Best for:** A desk display with touch and battery. If you have a Core2
or CoreS3 already, use this environment instead of `m5stack-core`.

---

### Sunton ESP32-2432S028R (CYD)

| Property | Value |
| :--- | :--- |
| Environment | `sunton-2432s028` |
| Chip | ESP32 (classic) |
| Display controller | ILI9341 |
| Resolution | 320 x 240 |
| Bus type | SPI |
| Flash | 4 MB |
| PSRAM | none |
| Build flag | `BOARD_SUNTON_2432S028` |
| Board config | `src/boards/sunton_2432s028.h` |

**Notes:** The Sunton ESP32-2432S028R is widely known as the "Cheap Yellow
Display" (CYD). It is a 2.8" 320x240 ILI9341 board with a resistive touch
panel, an RGB LED, and a classic ESP32, usually costing under $10. It is
the lowest-cost way to get a 320x240 panel running this firmware. The
board uses the standard `esp32dev` PlatformIO board definition.

**Best for:** The lowest-cost desk display. If you want to try the project
on a 320x240 panel and do not want to spend much, get a CYD.

---

### Waveshare ESP32-S3-Touch-LCD-1.28

| Property | Value |
| :--- | :--- |
| Environment | `waveshare-s3-round` |
| Chip | ESP32-S3 |
| Display controller | GC9A01 |
| Resolution | 240 x 240 (round) |
| Bus type | SPI |
| Flash | 8 MB |
| PSRAM | none |
| Build flag | `BOARD_WAVESHARE_S3_ROUND` |
| Board config | `src/boards/waveshare_s3_round.h` |

**Notes:** This is a round 1.28" GC9A01 panel on an ESP32-S3 carrier board.
The circular active area is 240x240 with the corners masked. The firmware
renders the compact single-spot view with a connection ring drawn around
the edge of the circle. USB CDC is enabled. Flash is 8 MB.

**Best for:** A novel desk gadget or a custom enclosure project. The round
form factor makes a striking spot display, especially with the connection
ring indicating cluster link status.

---

### LilyGO T-Watch 2020

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-t-watch-2020` |
| Chip | ESP32 (classic) |
| Display controller | ST7789 |
| Resolution | 240 x 240 |
| Bus type | SPI |
| Flash | 16 MB |
| PSRAM | none |
| Build flag | `BOARD_LILYGO_TWATCH_2020` |
| Board config | `src/boards/lilygo_twatch_2020.h` |

**Notes:** The T-Watch 2020 is a smartwatch form factor with a 1.54"
240x240 ST7789. Critically, it uses an **AXP202 PMU** for power
management, and the display backlight is powered by **AXP202 LDO2**, which
must be enabled via I2C before the display will show anything. The
firmware handles this automatically: `initTWatch2020PMU()` is called once
in `setup()` before `display.begin()`, and it enables LDO2 and sets it to
3.3 V over I2C (SDA=21, SCL=22, address 0x35). No external PMU library is
needed.

> **V3 variant:** The T-Watch 2020 V3 uses backlight pin GPIO 15 instead
> of GPIO 12. If you have a V3, add `-D TWATCH_BL_PIN=15` to the
> environment's `build_flags` in `platformio.ini`.

The 240x240 round-ish square panel uses the compact single-spot view with
a connection ring.

**Best for:** A wearable. The watch form factor with a built-in battery
makes this the most portable of the supported boards.

---

### LilyGO T-Watch S3

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-t-watch-s3` |
| Chip | ESP32-S3 |
| Display controller | ST7789V3 |
| Resolution | 240 x 240 |
| Bus type | SPI |
| Flash | 16 MB |
| PSRAM | OPI PSRAM (`BOARD_HAS_PSRAM`) |
| Build flag | `BOARD_LILYGO_TWATCH_S3` |
| Board config | `src/boards/lilygo_twatch_s3.h` |

**Notes:** The T-Watch S3 is the S3 successor to the T-Watch 2020, with a
1.54" 240x240 **ST7789V3** panel. Unlike the 2020, the S3 variant drives
the display backlight **directly from GPIO 45**, so no PMU initialization
is needed for the display. The board does include an **AXP2101 PMU** for
battery management (I2C SDA=10, SCL=11), but the firmware does not need
to talk to it to light the screen. OPI PSRAM is enabled, and USB CDC is
on. The panel uses `invert = true` in its LovyanGFX config (ST7789V3
quirk).

**Best for:** A wearable with more headroom. The S3's PSRAM and larger
flash leave room for future features, and the direct backlight drive
makes the boot path simpler than the 2020.

---

### LilyGO T-Deck

| Property | Value |
| :--- | :--- |
| Environment | `lilygo-tdeck` |
| Chip | ESP32-S3 |
| Display controller | ST7789 |
| Resolution | 320 x 240 |
| Bus type | SPI |
| Flash | 16 MB |
| PSRAM | OPI PSRAM (`BOARD_HAS_PSRAM`) |
| Build flag | `BOARD_LILYGO_TDECK` |
| Board config | `src/boards/lilygo_tdeck.h` |

**Notes:** The T-Deck is a pocket-sized companion with a 2.8" 320x240
ST7789 display, a physical keyboard, and a trackball. It has a
peripheral power-enable pin (GPIO 10) that must be set HIGH before the
display receives power — the firmware handles this automatically via
`initTDeckPower()` at boot. USB CDC and OPI PSRAM are enabled. The
keyboard and trackball are not used by this firmware (the BOOT button
is used for the command menu), but the form factor makes a nice
self-contained desk gadget.

**Best for:** A desk display with a premium form factor. The keyboard
leaves room for future interactive features.

---

### Heltec WiFi Kit 32 (V1/V2)

| Property | Value |
| :--- | :--- |
| Environment | `heltec-wifi-kit-32`, `heltec-wifi-kit-32-v2` |
| Chip | ESP32 (classic) |
| Display controller | SSD1306 |
| Resolution | 128 x 64 |
| Bus type | I2C (SDA=GPIO4, SCL=GPIO15, RST=GPIO16) |
| Flash | 4 MB (V1) / 8 MB (V2) |
| PSRAM | none |
| Build flag | `BOARD_HELTEC_WIFI_KIT_32` / `BOARD_HELTEC_WIFI_KIT_32_V2` |
| Board config | `src/boards/heltec_oled_128x64.h` |

**Notes:** The WiFi Kit 32 is Heltec's classic Wi-Fi-only board with a
0.96" monochrome OLED. V1 has 4 MB flash; V2 has 8 MB. Both use the same
I2C pinout for the OLED (SDA=4, SCL=15, RST=16, address 0x3C). The
128x64 OLED uses the compact single-spot UI layout. No backlight pin —
the OLED is always on.

**Best for:** A minimal, low-cost OLED display. If you have a Heltec
WiFi Kit 32 in your parts bin, this is the fastest way to get the
firmware running on an OLED.

---

### Heltec WiFi Kit 32 V3

| Property | Value |
| :--- | :--- |
| Environment | `heltec-wifi-kit-32-v3` |
| Chip | ESP32-S3 |
| Display controller | SSD1306 |
| Resolution | 128 x 64 |
| Bus type | I2C (SDA=GPIO17, SCL=GPIO18, RST=GPIO21) |
| Flash | 8 MB |
| PSRAM | none |
| Build flag | `BOARD_HELTEC_WIFI_KIT_32_V3` |
| Board config | `src/boards/heltec_oled_128x64_v3.h` |

**Notes:** The V3 switches from the classic ESP32 to the ESP32-S3, and
the OLED I2C pins moved: SDA=17, SCL=18, RST=21 (different from V1/V2).
USB CDC is enabled. The display is the same 0.96" 128x64 SSD1306 OLED.

**Best for:** A modern Heltec OLED board with USB CDC and the S3's
extra headroom.

---

### Heltec WiFi LoRa 32 (V1/V2)

| Property | Value |
| :--- | :--- |
| Environment | `heltec-wifi-lora-32`, `heltec-wifi-lora-32-v2` |
| Chip | ESP32 (classic) |
| Display controller | SSD1306 |
| Resolution | 128 x 64 |
| Bus type | I2C (SDA=GPIO4, SCL=GPIO15, RST=GPIO16) |
| Flash | 4 MB (V1) / 8 MB (V2) |
| PSRAM | none |
| Build flag | `BOARD_HELTEC_WIFI_LORA_32` / `BOARD_HELTEC_WIFI_LORA_32_V2` |
| Board config | `src/boards/heltec_oled_128x64.h` |

**Notes:** The WiFi LoRa 32 is Heltec's popular LoRa board with the same
0.96" 128x64 SSD1306 OLED as the WiFi Kit 32. It includes a SX1276 LoRa
radio, but this firmware only uses Wi-Fi — the LoRa radio is unused. The
OLED pinout is identical to the WiFi Kit 32 (SDA=4, SCL=15, RST=16).

**Best for:** If you already have a WiFi LoRa 32 and want to use it as
a DX cluster display. The LoRa radio is simply ignored.

---

### Heltec WiFi LoRa 32 V3

| Property | Value |
| :--- | :--- |
| Environment | `heltec-wifi-lora-32-v3` |
| Chip | ESP32-S3 |
| Display controller | SSD1306 |
| Resolution | 128 x 64 |
| Bus type | I2C (SDA=GPIO17, SCL=GPIO18, RST=GPIO21) |
| Flash | 8 MB |
| PSRAM | none |
| Build flag | `BOARD_HELTEC_WIFI_LORA_32_V3` |
| Board config | `src/boards/heltec_oled_128x64_v3.h` |

**Notes:** The V3 upgrades to the ESP32-S3 and SX1262 LoRa radio. The
OLED pins moved to SDA=17, SCL=18, RST=21 (same as WiFi Kit 32 V3). USB
CDC is enabled. The LoRa radio is unused by this firmware.

**Best for:** A modern Heltec LoRa board repurposed as a DX cluster
display.

---

### Heltec Wireless Stick / Stick Lite

| Property | Value |
| :--- | :--- |
| Environment | `heltec-wireless-stick`, `heltec-wireless-stick-lite` |
| Chip | ESP32 (classic) |
| Display controller | SSD1306 |
| Resolution | 64 x 32 |
| Bus type | I2C (SDA=GPIO4, SCL=GPIO15, RST=GPIO16) |
| Flash | 8 MB (Stick) / 4 MB (Stick Lite) |
| PSRAM | none |
| Build flag | `BOARD_HELTEC_WIRELESS_STICK` / `BOARD_HELTEC_WIRELESS_STICK_LITE` |
| Board config | `src/boards/heltec_oled_64x32.h` |

**Notes:** The Wireless Stick and Stick Lite are compact boards with a
tiny 0.49" 64x32 OLED — the smallest screen in the supported set. They
also include a SX1276 LoRa radio (unused). The 64x32 panel uses the
compact single-spot UI. The SSD1306 controller's memory is 128x64, so
the panel is configured with an offset (offset_x=32) to address the
visible 64x32 region.

**Best for:** A minimal, pocketable OLED display. The smallest form
factor in the supported set.

---

### Heltec Wireless Tracker

| Property | Value |
| :--- | :--- |
| Environment | `heltec-wireless-tracker` |
| Chip | ESP32-S3 |
| Display controller | ST7735 |
| Resolution | 160 x 80 |
| Bus type | SPI (MOSI=42, SCLK=41, DC=40, RST=39, CS=38) |
| Flash | 8 MB |
| PSRAM | none |
| Build flag | `BOARD_HELTEC_WIRELESS_TRACKER` |
| Board config | `src/boards/heltec_wireless_tracker.h` |

**Notes:** The Wireless Tracker is the only Heltec board in the
supported set with a color TFT display (ST7735, 160x80) instead of an
OLED. It also includes a SX1262 LoRa radio and a UC6580 dual-frequency
GNSS receiver — both unused by this firmware (Wi-Fi only). Uses
`esp32-s3-devkitc-1` as the PlatformIO board definition since no
official Heltec Wireless Tracker board ID exists. USB CDC is enabled.

**Best for:** A compact color display with GPS capability for future
features. The 160x80 panel is larger than the OLED boards but still
uses the compact UI layout.

---

## Choosing a board by use case

Not sure which board to get? Here are recommendations based on how you
plan to use the device.

### Desk display

A desk display sits next to your radio, plugged into USB power, showing
spots all day. You want a large, readable panel and a robust form factor.

| Recommendation | Board | Why |
| :--- | :--- | :--- |
| Top pick | **Sunton CYD** (`sunton-2432s028`) | Lowest cost, 320x240, full spot list |
| Premium pick | **LilyGO T-Display-S3 AMOLED** (`lilygo-tdisplay-s3-amoled`) | Best contrast and brightness, wide panel |
| Fast refresh | **LilyGO T-Display-S3** (`lilygo-tdisplay-s3`) | Parallel bus, smooth redraws |
| Touch / future-proof | **LilyGO T-HMI** (`lilygo-t-hmi`) | 320x240 with touch, 16 MB flash, PSRAM |
| M5Stack ecosystem | **M5Stack Core2** (`m5stack-core2`) | Self-contained, battery, touch |

### Wearable

A wearable is strapped to your wrist or clipped to a lanyard while you
operate. You want a small panel, a built-in battery, and a watch or stick
form factor.

| Recommendation | Board | Why |
| :--- | :--- | :--- |
| Top pick | **LilyGO T-Watch 2020** (`lilygo-t-watch-2020`) | True watch form factor, battery |
| Modern alternative | **LilyGO T-Watch S3** (`lilygo-t-watch-s3`) | S3, PSRAM, simpler backlight |
| Stick form factor | **M5StickC Plus** (`m5stickc-plus`) | Clip-on stick, battery, cheap |

### Portable / pocketable

A portable device is small enough to carry but not necessarily worn on the
wrist. You want low cost and a compact panel.

| Recommendation | Board | Why |
| :--- | :--- | :--- |
| Top pick | **LilyGO T-QT** (`lilygo-t-qt`) | Tiny, cheap, S3 |
| Novelty pick | **Waveshare S3 Round** (`waveshare-s3-round`) | Round display, striking UI |
| Lowest cost | **LilyGO T-Display** (`lilygo-tdisplay`) | Cheap, classic, easy to find |
| OLED pick | **Heltec WiFi Kit 32 V3** (`heltec-wifi-kit-32-v3`) | 0.96" OLED, ESP32-S3, USB CDC |
| Smallest | **Heltec Wireless Stick Lite** (`heltec-wireless-stick-lite`) | 0.49" OLED, tiny form factor |

---

## How board support works

Each board is defined in three places:

1. **`platformio.ini`** — one `[env:...]` section per board. This sets the
   PlatformIO board ID, flash size, PSRAM type, USB CDC flags, and the
   `-D BOARD_*` build flag that selects the board at compile time.
2. **`src/boards/<board>.h`** — a LovyanGFX device class that configures
   the display controller, bus (SPI / parallel / QSPI), pin assignments,
   panel geometry, and backlight. Some boards also include helper
   functions (e.g. `initTWatch2020PMU()` for the AXP202).
3. **`src/BoardConfig.h`** — maps each `BOARD_*` macro to the correct
   board header and exposes a common `lgfx::LGFX_Device` reference so the
   rest of the firmware is board-agnostic.

Because the UI code in `src/DxDisplay.cpp` talks to the LovyanGFX API and
not to any specific panel, the same source compiles for all 22 boards.
The display library, [LovyanGFX](https://github.com/lovyan03/LovyanGFX),
is what makes this practical: a single library supports every controller
we target (ST7789, ILI9341, GC9A01, RM67162, ST7789V3, SSD1306, ST7735)
across SPI, 8-bit parallel, QSPI, and I2C buses with one unified API.

To add a board that is not yet supported, see
[Adding a New Board](08-adding-new-board.md).
