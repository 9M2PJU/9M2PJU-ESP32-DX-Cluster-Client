# Display After Connecting to DX Cluster

This document shows exactly what appears on screen when the device is
**connected to a DX cluster server and receiving live DX spots**.

The mockups are generated from the actual rendering code in `DxDisplay.cpp`
and `DxSpot.cpp` — every colour, position, and truncation matches the firmware.

---

## Colour reference (from source code)

```
  COLOR_BG       0x0008  near-black blue    → screen background
  COLOR_PANEL    0x0841  dark blue          → fresh spot highlight (< 5 sec)
  COLOR_TEXT     0xFFFF  white              → frequency, callsign, clock
  COLOR_TEXT_DIM 0xBDF7  light grey         → title, spotter, comments
  COLOR_ACCENT   0x07FF  cyan               → connected dot, sweep, DX call on round
  COLOR_ALERT    0xFD20  orange             → disconnected/connecting dot
  COLOR_GRID     0x2104  dark grey          → separator lines
```

### Band colours (from `bandColor()` in DxSpot.cpp)

```
  160m (1.8 MHz)   0xB81F  magenta
   80m (3.5 MHz)   0x07FF  cyan
   40m (7.0 MHz)   0x07E0  green
   20m (14 MHz)    0xFFE0  yellow
   15m (21 MHz)    0xFD20  orange
   10m (28 MHz)    0xF81F  red-blue (magenta)
    6m (50 MHz)    0xF800  red
  VHF (144 MHz)    0x7BEF  light grey
  UHF (430+ MHz)   0x8410  dark teal
```

---

## How the layout is chosen

The firmware measures the display at runtime (`_lcd.width()` / `_lcd.height()`
after `setRotation()`) and picks a layout:

| Condition | Layout | Spots | Header |
| :--- | :--- | :--- | :--- |
| width == height (square) | Round compact | 1 | none |
| height <= 64 | Tiny | 1 | 14px |
| height <= 135 | Small (1 spot, tall row) | 1 | 18px |
| height <= 170 | Medium | 4 | 22px |
| height > 170 | Large | 5-13 | 24px |

---

## Board-by-board actual dimensions

| Board | Width x Height | Rotation | Layout | Spots |
| :--- | :--- | :--- | :--- | :--- |
| LilyGO T-Display-S3 | 170 x 320 | 1 (portrait) | Large | 7 |
| LilyGO T-Display-S3 AMOLED | 240 x 536 | 0 (portrait) | Large | 13 |
| LilyGO T-Display | 240 x 135 | 0 (landscape) | Small | 1 |
| LilyGO T-QT | 128 x 128 | 0 (square) | Round | 1 |
| LilyGO T-HMI | 320 x 240 | 0 (landscape) | Large | 5 |
| M5StickC Plus | 135 x 240 | 1 (portrait) | Large | 5 |
| M5Stack Core | 240 x 320 | 1 (portrait) | Large | 7 |
| M5Stack Core2 | 240 x 320 | 1 (portrait) | Large | 7 |
| Sunton CYD 2432S028 | 320 x 240 | 1 (landscape) | Large | 5 |
| Waveshare S3 Round | 240 x 240 | 0 (square) | Round | 1 |
| LilyGO T-Watch 2020 | 240 x 240 | 0 (square) | Round | 1 |
| LilyGO T-Watch S3 | 240 x 240 | 0 (square) | Round | 1 |
| LilyGO T-Deck | 320 x 240 | 0 (landscape) | Large | 5 |

---

## 1. LilyGO T-Display-S3 — 170 x 320 (portrait, 7 spots)

The T-Display-S3 runs in **portrait mode** (rotation 1). The header shows
the full title, and 7 spots fit on screen.

```
┌─────────────────────────┐
│ 9M2PJU DX Cluster  ●12Z │  ← header (24px): grey title, white clock, cyan dot
├─────────────────────────┤  ← animated cyan sweep line
│▌14.074  JA1ABC           │  ← spot 1 (fresh = blue bg, yellow bar = 20m)
│ de 9M2XYZ   FT8 strong   │
│▌21.300  VK2DEF           │  ← spot 2 (orange bar = 15m)
│ de 9M2ABC   big signal    │
│▌ 7.025  DL1AAA           │  ← spot 3 (green bar = 40m)
│ de N0CALL   Europe open   │
│▌28.500  ZL2XYZ           │  ← spot 4 (magenta bar = 10m)
│ de JA1ABC   long path     │
│▌18.100  W1AW             │  ← spot 5 (yellow bar = 17m, within 15-22 range)
│ de OK2ABC   17m strong    │
│▌14.195  PY2ABC           │  ← spot 6 (yellow bar = 20m)
│ de LU1DEF   SSB QSB       │
│▌ 3.700  K1AAA            │  ← spot 7 (cyan bar = 80m)
│ de W1BBB    80m CW        │
└─────────────────────────┘
```

**Details:**
- Header: "9M2PJU DX Cluster Client" truncated to fit 170px width.
  At 170px, titleMaxChars ≈ (170-12-8)/6 ≈ 25, so full title fits.
- Connection dot: **cyan, pulsing** (radius 2→3→4 every 300ms) = connected.
- Each spot row is 38px tall. The 3px coloured bar on the left indicates the band.
- Frequency in white, DX callsign in the band colour.
- "de SPOTTER" + comment in grey on the second line of each row.
- Fresh spots (received < 5 seconds ago) have a **dark blue background**.

---

## 2. LilyGO T-Display-S3 AMOLED — 240 x 536 (portrait, 13 spots)

The AMOLED runs in portrait mode. 13 spots fit on the very tall screen.

```
┌──────────────────────────┐
│ 9M2PJU DX Cluster  ●12Z  │  ← header (24px)
├──────────────────────────┤
│▌14.074  JA1ABC            │  ← spot 1
│ de 9M2XYZ   FT8 strong    │
│▌21.300  VK2DEF            │  ← spot 2
│ de 9M2ABC   big signal    │
│▌ 7.025  DL1AAA            │  ← spot 3
│ de N0CALL   Europe open   │
│▌28.500  ZL2XYZ            │  ← spot 4
│ de JA1ABC   long path     │
│▌18.100  W1AW              │  ← spot 5
│ de OK2ABC   17m strong    │
│▌14.195  PY2ABC            │  ← spot 6
│ de LU1DEF   SSB QSB       │
│▌ 3.700  K1AAA             │  ← spot 7
│ de W1BBB    80m CW        │
│▌24.900  I2XYZ             │  ← spot 8
│ de G3ABC    12m opening   │
│▌50.150  F2ABC             │  ← spot 9 (red bar = 6m)
│ de EA4XYZ   6m Es         │
│▌14.070  N1ZZZ             │  ← spot 10
│ de K2AAA     FT8          │
│▌ 7.014  G4WWW             │  ← spot 11
│ de M0ABC     CW QRP       │
│▌21.200  ZS6XYZ            │  ← spot 12
│ de 5Z8ABC    long path    │
│▌14.063  JA2DEF            │  ← spot 13
│ de VK3ABC    FT8 JA        │
└──────────────────────────┘
```

---

## 3. LilyGO T-Display — 240 x 135 (landscape, 1 spot)

The T-Display shows a single spot in a tall row (117px). The frequency and
callsign are at the top, the spotter and comment at the bottom.

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client    ●  12:34Z   │  ← header (18px)
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← frequency (white) + callsign (yellow)
│                                          │
│                                          │
│                                          │
│                                          │
│  de 9M2XYZ    FT8, strong in EU           │  ← spotter + comment (grey)
└──────────────────────────────────────────┘
```

**Details:**
- Only 1 spot visible. Row height = 135 - 18 = 117px.
- At 240px wide, the full title "9M2PJU DX Cluster Client" fits.
- The 3px yellow bar on the left = 20m band (14 MHz).

---

## 4. LilyGO T-QT — 128 x 128 (round, compact)

The T-QT has a tiny round display. It shows one spot in a compact layout
with a connection ring around the edge.

```
        .::::::::::::::::.
      ::::::::::::::::::::::
    ::::::::::::::::::::::::::
   ::::                      ::::
   ::::   9M2PJU DX Cluster  ::::   ← cyan title (centred, top)
   ::::       12:34Z         ::::   ← white UTC clock
   ::::                      ::::
   ::::      14.074          ::::   ← frequency, 2x size, yellow (20m band)
   ::::                      ::::
   ::::     JA1ABC           ::::   ← DX callsign, 3x size, white
   ::::                      ::::
   ::::   de 9M2XYZ          ::::   ← spotter, small, grey (bottom)
    ::::::::::::::::::::::::::
      ::::::::::::::::::::::
        '::::::::::::::::'
```

**Details:**
- The connection ring is drawn as a circle at the screen edge.
  **Cyan, pulsing inward** (1-3px) every 400ms = connected.
- Title "9M2PJU DX Cluster" in cyan, centred at y=4.
- UTC clock in white, centred at y=16.
- Frequency at 2x text size (12px per char) in the band colour.
- DX callsign at 3x text size (18px per char) in white, centred.
- "de SPOTTER" in grey, small, centred at the bottom.

---

## 5. LilyGO T-HMI — 320 x 240 (landscape, 5 spots)

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client    ●  12:34Z   │  ← header (24px)
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← spot 1 (fresh = blue bg)
│ de 9M2XYZ   FT8, strong in EU             │
│▌21.300  VK2DEF                            │  ← spot 2
│ de 9M2ABC   big signal on 15m             │
│▌ 7.025  DL1AAA                            │  ← spot 3
│ de N0CALL   Europe opening                │
│▌28.500  ZL2XYZ                            │  ← spot 4
│ de JA1ABC   long path, strong             │
│▌18.100  W1AW                              │  ← spot 5
│ de OK2ABC   strong on 17m                 │
└──────────────────────────────────────────┘
```

---

## 6. M5StickC Plus — 135 x 240 (portrait, 5 spots)

The M5StickC Plus runs in portrait mode (135 wide, 240 tall). The title is
truncated because only ~15 chars fit in 135px.

```
┌───────────────┐
│ 9M2PJU DX ●12Z│  ← header (24px): "9M2PJU DX Cluster" truncated to "9M2PJU DX"
├───────────────┤
│▌14.074 JA1ABC  │  ← spot 1
│ de 9M2XYZ FT8  │
│▌21.300 VK2DEF  │  ← spot 2
│ de 9M2ABC big  │
│▌ 7.025 DL1AAA  │  ← spot 3
│ de N0CALL EU   │
│▌28.500 ZL2XYZ  │  ← spot 4
│ de JA1ABC LP   │
│▌18.100 W1AW    │  ← spot 5
│ de OK2ABC 17m  │
└───────────────┘
```

**Details:**
- At 135px wide, titleMaxChars ≈ (135-12-8)/6 ≈ 19, so "9M2PJU DX Cluster" (18 chars) fits.
- Comments are heavily truncated due to the narrow 135px width (~22 chars per line).
- 5 spots visible, each 38px tall.

---

## 7. M5Stack Core / Core2 — 240 x 320 (portrait, 7 spots)

```
┌──────────────────────────┐
│ 9M2PJU DX Cluster  ●12Z  │  ← header (24px)
├──────────────────────────┤
│▌14.074  JA1ABC            │  ← spot 1
│ de 9M2XYZ   FT8 strong    │
│▌21.300  VK2DEF            │  ← spot 2
│ de 9M2ABC   big signal    │
│▌ 7.025  DL1AAA            │  ← spot 3
│ de N0CALL   Europe open   │
│▌28.500  ZL2XYZ            │  ← spot 4
│ de JA1ABC   long path     │
│▌18.100  W1AW              │  ← spot 5
│ de OK2ABC   17m strong    │
│▌14.195  PY2ABC            │  ← spot 6
│ de LU1DEF   SSB QSB       │
│▌ 3.700  K1AAA             │  ← spot 7
│ de W1BBB    80m CW        │
└──────────────────────────┘
```

---

## 8. Sunton CYD (ESP32-2432S028) — 320 x 240 (landscape, 5 spots)

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client    ●  12:34Z   │  ← header (24px)
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← spot 1
│ de 9M2XYZ   FT8, strong in EU             │
│▌21.300  VK2DEF                            │  ← spot 2
│ de 9M2ABC   big signal on 15m             │
│▌ 7.025  DL1AAA                            │  ← spot 3
│ de N0CALL   Europe opening                │
│▌28.500  ZL2XYZ                            │  ← spot 4
│ de JA1ABC   long path, strong             │
│▌18.100  W1AW                              │  ← spot 5
│ de OK2ABC   strong on 17m                 │
└──────────────────────────────────────────┘
```

---

## 9. Waveshare S3 Round / T-Watch 2020 / T-Watch S3 — 240 x 240 (round, compact)

All three round/square 240x240 displays use the same compact layout.

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   ::::::                      ::::::
   ::::::  9M2PJU DX Cluster  ::::::   ← cyan title (centred, top)
   ::::::      12:34Z         ::::::   ← white UTC clock
   ::::::                      ::::::
   ::::::     14.074           ::::::   ← frequency, 2x size, yellow (20m)
   ::::::                      ::::::
   ::::::    JA1ABC            ::::::   ← DX callsign, 3x size, white
   ::::::                      ::::::
   ::::::  de 9M2XYZ           ::::::   ← spotter, small, grey (bottom)
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

**Details:**
- Connection ring at the edge: **cyan, pulsing** = connected.
- Frequency "14.074" at 2x size (each char 12px wide) in yellow (20m band).
- Callsign "JA1ABC" at 3x size (each char 18px wide) in white.
- "de 9M2XYZ" in grey at the bottom.
- On the 240x240 display, the callsign can be up to 8 chars before truncation.

---

## 10. LilyGO T-Deck — 320 x 240 (landscape, 5 spots)

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client    ●  12:34Z   │  ← header (24px)
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← spot 1
│ de 9M2XYZ   FT8, strong in EU             │
│▌21.300  VK2DEF                            │  ← spot 2
│ de 9M2ABC   big signal on 15m             │
│▌ 7.025  DL1AAA                            │  ← spot 3
│ de N0CALL   Europe opening                │
│▌28.500  ZL2XYZ                            │  ← spot 4
│ de JA1ABC   long path, strong             │
│▌18.100  W1AW                              │  ← spot 5
│ de OK2ABC   strong on 17m                 │
└──────────────────────────────────────────┘
```

---

## Connecting state (before spots arrive)

When connected to the cluster but no spots have arrived yet:

### Rectangular displays

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client    ●  12:34Z   │  ← cyan dot (connected)
├──────────────────────────────────────────┤
│                                          │
│       Waiting for DX spots...            │  ← grey, centred
│                                          │
└──────────────────────────────────────────┘
```

### Round displays

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   ::::::                      ::::::
   ::::::  9M2PJU DX Cluster  ::::::   ← cyan title
   ::::::      12:34Z         ::::::   ← white clock
   ::::::                      ::::::
   ::::::                      ::::::
   ::::::     No spots         ::::::   ← grey, centred
   ::::::                      ::::::
   ::::::                      ::::::
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

---

## Disconnected state (Wi-Fi or cluster connection lost)

If the connection drops, the dot/ring turns **orange** (COLOR_ALERT = 0xFD20)
and stays steady (no pulsing):

### Rectangular displays

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client    ●  12:34Z   │  ← ORANGE dot (disconnected)
├──────────────────────────────────────────┤
│                                          │
│    Connecting to DX cluster...           │  ← grey, centred
│                                          │
└──────────────────────────────────────────┘
```

### Round displays

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   ::::::                      ::::::
   ::::::  9M2PJU DX Cluster  ::::::   ← cyan title
   ::::::      12:34Z         ::::::   ← white clock
   ::::::                      ::::::
   ::::::                      ::::::
   ::::::    Connecting        ::::::   ← grey, centred (ring = orange, steady)
   ::::::                      ::::::
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

---

## Animation details (connected state)

| Element | Animation | Code |
| :--- | :--- | :--- |
| Connection dot (rectangular) | Pulses radius 2→3→4 every 300ms, cyan | `2 + ((nowMs / 300) % 3)` |
| Connection ring (round) | Pulses 1-3px inward every 400ms, cyan | `1 + ((nowMs / 400) % 3)` |
| Header sweep line | Cyan line sweeps left-to-right, 18ms per pixel | `(nowMs / 18) % _width` |
| Fresh spot highlight | Dark blue background for 5 seconds after spot arrives | `NEW_SPOT_MS = 5000` |
| Frame rate | ~30 FPS (33ms frame interval) | `FRAME_MS = 33` |

---

## Spot row anatomy (rectangular displays)

Each spot row contains these elements, drawn exactly as shown:

```
  ▌14.074  JA1ABC
    de 9M2XYZ   FT8, strong in EU
    │         │     │            │
    │         │     │            └─ comment (grey, truncated to fit width)
    │         │     └─ spotter callsign (grey, max 10 chars + ".")
    │         └─ DX callsign (band colour, truncated to fit)
    └─ frequency (white, e.g. "14.074")
    
    ▌ = 3px coloured bar (band colour), 3px from left edge, 3px from top/bottom
```

The band colour of the bar and DX callsign is determined by frequency:

```
  1.8 MHz  → magenta  ████  160m
  3.5 MHz  → cyan     ████  80m
  7.0 MHz  → green    ████  40m
 14.0 MHz  → yellow   ████  20m
 21.0 MHz  → orange   ████  15m
 28.0 MHz  → magenta  ████  10m
 50.0 MHz  → red      ████  6m
144.0 MHz  → grey     ████  VHF
```
