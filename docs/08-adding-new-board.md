# Adding Support for a New Board

This guide explains how to add a new ESP32 board with a screen to the 9M2PJU ESP32 DX Cluster Client. The project uses [LovyanGFX](https://github.com/lovyan03/LovyanGFX) for display abstraction, and each board is described by a small LovyanGFX subclass plus a few preprocessor macros. Adding a board touches six places, all listed below.

---

## Overview

Every board is selected at compile time by a `-D BOARD_<NAME>` build flag. That flag causes `src/BoardConfig.h` to include a board-specific header from `src/boards/` and to define a set of geometry macros (`BOARD_DISPLAY_CLASS`, `BOARD_DISPLAY_WIDTH`, etc.) that the rest of the firmware uses. Because the selection is purely preprocessor-based, only one board is compiled into any given firmware image.

There are **six steps** to add a board:

1. Create `src/boards/<name>.h` with a LovyanGFX subclass.
2. Add a selection block in `src/BoardConfig.h`.
3. Add an `[env:<name>]` section in `platformio.ini`.
4. Add an entry to the `boards` array in `docs/index.html`.
5. Add an entry to the `boards` dict in `.github/workflows/build-flash.yml`.
6. Build and test on real hardware.

Each step is detailed below, followed by a complete worked example.

---

## Step 1: Create `src/boards/<name>.h`

Create a new header that defines a LovyanGFX device subclass. The subclass configures the panel, the bus (SPI, parallel, or QSPI), and optionally the backlight. The class name should be unique and descriptive, e.g. `LGFX_MYBOARD`.

The file must:

- `#include <LovyanGFX.hpp>`
- Declare a class deriving from `lgfx::LGFX_Device`.
- Hold member objects for the panel, bus, and light.
- In the constructor, configure each object and wire them together with `setBus()` / `setLight()` / `setPanel()`.

If the board requires a PMU (power management IC) to be initialised before the display will receive power (e.g. the T-Watch 2020's AXP202), also define an `init<Board>PMU()` function in this header. See `src/boards/lilygo_twatch_2020.h` for the pattern.

See the [complete example](#complete-example-board-file) at the end of this guide.

---

## Step 2: Add a block in `src/BoardConfig.h`

Open `src/BoardConfig.h` and add an `#elif defined(BOARD_<NAME>)` block in the board-selection chain. The block must define:

| Macro | Purpose |
|---|---|
| `BOARD_DISPLAY_CLASS` | The LovyanGFX subclass to instantiate (from step 1). |
| `BOARD_DISPLAY_WIDTH` | Panel width in pixels, in the default rotation. |
| `BOARD_DISPLAY_HEIGHT` | Panel height in pixels, in the default rotation. |
| `BOARD_DISPLAY_ROTATION` | LovyanGFX rotation value (0-3). |
| `BOARD_HAS_BACKLIGHT` | `1` if the board has a PWM backlight pin, `0` otherwise. |
| `BOARD_NEEDS_PMU_INIT` | (Optional) `1` if a PMU must be initialised before the display. Triggers a call to the PMU init function in `main.cpp`. |

Example block:

```c
#elif defined(BOARD_MYBOARD)
#include "boards/myboard.h"
#define BOARD_DISPLAY_CLASS   LGFX_MYBOARD
#define BOARD_DISPLAY_WIDTH   320
#define BOARD_DISPLAY_HEIGHT  240
#define BOARD_DISPLAY_ROTATION 1
#define BOARD_HAS_BACKLIGHT   1
```

Then add a matching `BOARD_NAME` string in the board-name section at the bottom of the file:

```c
#elif defined(BOARD_MYBOARD)
#define BOARD_NAME "My Custom Board"
```

> **Important:** If you add a new `BOARD_*` flag, also add it to the fallback guard at the top of `BoardConfig.h` (the long `#if !defined(...)` chain). Otherwise the default-board fallback may override your flag. The fallback selects `BOARD_LILYGO_TDISPLAY_S3` when no board flag is set.

---

## Step 3: Add an `[env:<name>]` section in `platformio.ini`

Add a new environment that sets the `-D BOARD_<NAME>` flag and the correct PlatformIO board definition. Inherit shared settings from `[common]`:

```ini
; --- My Custom Board (2.0" ILI9341 320x240, ESP32-S3, SPI) -----------------
[env:myboard]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps = ${common.lib_deps}
build_flags =
  ${common.build_flags}
  -D BOARD_MYBOARD
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DARDUINO_USB_MODE=1
monitor_speed = ${common.monitor_speed}
upload_speed = ${common.upload_speed}
```

Key fields:

- **`board`** must match the PlatformIO board identifier for your chip (e.g. `esp32dev` for original ESP32, `esp32-s3-devkitc-1` for ESP32-S3, `m5stick-c` for M5StickC).
- **`-D BOARD_<NAME>`** is the flag consumed by `BoardConfig.h`.
- For ESP32-S3 boards with native USB, include `-DARDUINO_USB_CDC_ON_BOOT=1` and `-DARDUINO_USB_MODE=1` so the serial console works over USB.
- For boards with PSRAM, add `-DBOARD_HAS_PSRAM` and the `board_build.psram_type` / `board_build.arduino.memory_type` options (copy from an existing S3 environment).
- Set `board_upload.flash_size` if the board has a non-default flash size.

---

## Step 4: Add to `docs/index.html`

The web flasher page lists each board as a card. Add an entry to the `boards` JavaScript array near the bottom of `docs/index.html`:

```js
const boards = [
  // ...existing entries...
  {id:"myboard", name:"My Custom Board", chip:"ESP32-S3",
   display:"2.0\" ILI9341 320x240", manifest:"manifests/myboard.json"},
];
```

The `id` must match the environment name from step 3. The `manifest` path is relative to the site root and is generated by CI.

---

## Step 5: Add to `.github/workflows/build-flash.yml`

The CI workflow builds every environment and packages firmware + a manifest for the web flasher. Add your board to the `boards` dict in the Python packaging step:

```python
boards = {
    # ...existing entries...
    "myboard": {"chip": "ESP32-S3", "bl_offset": 0x0},
}
```

- **`chip`** is `"ESP32-S3"` for S3 boards, `"ESP32"` for original ESP32 / PICO variants. This controls the `chipFamily` field in the generated manifest, which the web flasher uses to pick the correct flashing protocol.
- **`bl_offset`** is the bootloader partition offset: `0x0` for ESP32-S3, `0x1000` for original ESP32. Check the board's partition table if unsure.

The key must match the environment name from step 3 exactly.

---

## Step 6: Build and Test

```bash
cp include/config.example.h include/config.h   # if not already present
pio run -e myboard
pio run -e myboard -t upload
pio device monitor -e myboard
```

Verify on the serial monitor that:

1. The splash screen shows the correct `BOARD_NAME`.
2. The display initialises (not blank, no garbage).
3. The panel geometry and rotation look correct.
4. The config-mode screen is readable (enter setup by holding BOOT during reset).
5. After configuring Wi-Fi, spots render in the correct orientation and the header/clock are visible.

If the display is blank or mirrored, revisit the panel config (offsets, invert, rotation) in your board header.

---

## Complete Example: Board File

Below is a complete, annotated example of a `src/boards/` header for a hypothetical SPI-based ESP32-S3 board with an ILI9341 panel. This is the pattern used by every board in the project: configure the bus, configure the panel, configure the backlight, then wire them together.

```c
#pragma once
/*
 * My Custom Board (ESP32-S3, 2.0" ILI9341 320x240, SPI).
 * Pinout from the board's schematic / manufacturer docs.
 */
#include <LovyanGFX.hpp>

class LGFX_MYBOARD : public lgfx::LGFX_Device {
  // The panel object. Use the class matching your controller IC:
  //   Panel_ILI9341, Panel_ST7789, Panel_GC9A01, Panel_RM67162, etc.
  lgfx::Panel_ILI9341 _panel;

  // The bus object. Use Bus_SPI for SPI, Bus_Parallel8 for 8-bit
  // parallel, Bus_Parallel16 for 16-bit, etc.
  lgfx::Bus_SPI _bus;

  // The backlight object. Use Light_PWM for a PWM-controllable pin.
  lgfx::Light_PWM _light;

 public:
  LGFX_MYBOARD(void) {
    // --- 1. Configure the SPI bus ---
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;       // SPI2_HOST or VSPI_HOST
      cfg.spi_mode = 0;              // SPI mode 0 for most ILI/ST panels
      cfg.freq_write = 40000000;     // 40 MHz write
      cfg.freq_read = 16000000;      // 16 MHz read
      cfg.spi_3wire = false;         // true if DC is sent over SPI MOSI
      cfg.use_lock = true;
      cfg.dma_channel = 1;           // DMA channel (1 or 2); 0 to disable
      cfg.pin_sclk = 18;             // SCK
      cfg.pin_mosi = 23;             // MOSI (SDA)
      cfg.pin_miso = -1;             // MISO (-1 if not connected)
      cfg.pin_dc = 16;               // DC (data/command)
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    // --- 2. Configure the panel geometry ---
    {
      auto cfg = _panel.config();
      cfg.memory_width = 240;        // controller's internal frame width
      cfg.memory_height = 320;       // controller's internal frame height
      cfg.panel_width = 240;         // visible panel width
      cfg.panel_height = 320;        // visible panel height
      cfg.offset_x = 0;             // column offset (some panels need 40 etc.)
      cfg.offset_y = 0;             // row offset
      cfg.offset_rotation = 0;      // add to rotation to correct orientation
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;          // false if the panel cannot read back
      cfg.invert = false;           // true for some ST7789 / AMOLED panels
      cfg.rgb_order = false;        // true if colors are swapped (BGR)
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;        // true if SPI bus is shared with SD card etc.
      _panel.config(cfg);
    }

    // --- 3. Configure the backlight (PWM) ---
    {
      auto cfg = _light.config();
      cfg.pin_bl = 4;               // backlight GPIO
      cfg.invert = false;           // true if backlight is active-low
      cfg.freq = 44100;             // PWM frequency
      cfg.pwm_channel = 7;          // LEDC channel (avoid channels used elsewhere)
      _light.config(cfg);
      _panel.setLight(&_light);
    }

    // --- 4. Register the panel with the device ---
    setPanel(&_panel);
  }
};
```

### Boards requiring a PMU

If the board's display power is gated behind a PMU (e.g. AXP202, AXP2101), add an init function and set `BOARD_NEEDS_PMU_INIT` in `BoardConfig.h`. The firmware calls the function once in `setup()` before `display.begin()`. The T-Watch 2020 is the reference implementation:

```c
inline void initTWatch2020PMU() {
  Wire.begin(21, 22);                 // SDA, SCL
  // ...enable LDO2 (display power) via I2C writes to the AXP202...
}
```

```c
// In BoardConfig.h:
#define BOARD_NEEDS_PMU_INIT 1
```

```cpp
// In main.cpp setup():
#if defined(BOARD_NEEDS_PMU_INIT)
  initTWatch2020PMU();
  delay(100);
#endif
```

---

## Reference: Board Macros

| Macro | Required | Description |
|---|---|---|
| `BOARD_<NAME>` | Yes | The compile-time flag set in `platformio.ini`. Must be unique. |
| `BOARD_DISPLAY_CLASS` | Yes | The concrete LovyanGFX subclass to instantiate. |
| `BOARD_DISPLAY_WIDTH` | Yes | Panel width in pixels (default rotation). |
| `BOARD_DISPLAY_HEIGHT` | Yes | Panel height in pixels (default rotation). |
| `BOARD_DISPLAY_ROTATION` | Yes | LovyanGFX rotation (0-3). Used by `DxDisplay::begin()`. |
| `BOARD_HAS_BACKLIGHT` | Yes | `1` if a PWM backlight pin exists; `0` otherwise. |
| `BOARD_NEEDS_PMU_INIT` | No | `1` if a PMU must be initialised before the display. Triggers the PMU init call in `main.cpp`. |
| `BOARD_NAME` | Yes | Human-readable board name shown on the splash and about screens. |

The display renderer (`DxDisplay`) reads `BOARD_DISPLAY_WIDTH`, `BOARD_DISPLAY_HEIGHT`, and `BOARD_DISPLAY_ROTATION` to adapt its layout automatically. Round/square panels (width == height) get a compact single-spot view; very small panels hide the comment line; larger panels show more spots. You do not need to write any rendering code for a new board as long as the geometry macros are correct.

---

## Tips for Finding the Right Pins

Accurate pin assignments are the most common source of trouble when adding a board. Use these sources:

1. **Manufacturer documentation.** Look for the board's schematic, pinout diagram, or "factory" Arduino/TFT_eSPI example. LilyGO, M5Stack, Waveshare, and Sunton all publish these on GitHub or their product pages.
2. **Existing LovyanGFX or TFT_eSPI configs.** Search the [LovyanGFX examples](https://github.com/lovyan03/LovyanGFX/tree/master/examples) and the board vendor's repos for a known-good `setup()` for the same panel. The pin numbers transfer directly.
3. **The board's pinout card.** Many boards ship with a printed card labelling MOSI, SCK, DC, CS, RST, and BL pins.
4. **Controller datasheet.** If you know the panel controller IC (ST7789, ILI9341, GC9A01, RM67162, etc.), use the matching `lgfx::Panel_*` class. The controller name is usually printed on the flex cable or in the product listing.
5. **Test incrementally.** If the display lights up but shows garbage, the most likely culprits are `offset_x`/`offset_y`, `invert`, `rgb_order`, or `offset_rotation`. Adjust one at a time.

> **PSRAM and flash size:** For ESP32-S3 boards with octal PSRAM, set `board_build.arduino.memory_type = qio_opi`, `board_build.psram_type = opi`, and `board_upload.flash_size` to the real flash size. Copy these from an existing S3 environment (e.g. `lilygo-tdisplay-s3`) to avoid boot loops.

---

## Checklist

Before opening a pull request that adds a board, confirm:

- [ ] `src/boards/<name>.h` exists and compiles.
- [ ] `src/BoardConfig.h` has the `#elif` block, the fallback guard update, and a `BOARD_NAME`.
- [ ] `platformio.ini` has an `[env:<name>]` with the correct `-D BOARD_<NAME>` flag.
- [ ] `docs/index.html` has a matching entry in the `boards` array.
- [ ] `.github/workflows/build-flash.yml` has a matching entry in the `boards` dict with the correct `chip` and `bl_offset`.
- [ ] `pio run -e <name>` builds cleanly.
- [ ] Firmware was uploaded and verified on real hardware (display, setup portal, spot rendering).
