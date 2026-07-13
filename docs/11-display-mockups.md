# Display Mockups

This document shows how the firmware renders on each category of display.
The ESP32 DX Cluster Client supports 22 boards across 6 display layouts.
All mockups are approximate — actual rendering uses anti-aliased fonts,
colours, and animated elements described in the notes.

---

## Colour key (used in all mockups)

```
  CYAN  (#07FF)  = accent / connected / frequency band highlight
  WHITE (#FFFF)  = primary text (callsigns, clock)
  GREY  (#BDF7)  = dim text (spotter, comments, hints)
  AMBER (#FD20)  = alert / disconnected / setup mode
  BLUE  (#0841)  = panel background (fresh spot highlight)
  GREEN (#07E0)  = 20m band (14 MHz)
  YELLOW(#FFE0)  = 15m band (21 MHz)
  RED   (#F800)  = 40m band (7 MHz)
```

---

## Display categories

| Category | Boards | Effective size | Layout | Spots shown |
| :--- | :--- | :--- | :--- | :--- |
| Landscape wide | T-Display-S3 | 320 x 170 | Scrolling list | 4 |
| Landscape wide | T-HMI | 320 x 240 | Scrolling list | 5 |
| Landscape wide | T-Display-S3 AMOLED | 536 x 240 | Scrolling list | 5 |
| Portrait tall | M5StickC Plus | 135 x 240 | Scrolling list | 5 |
| Portrait tall | M5Stack Core / Core2, Sunton CYD | 240 x 320 | Scrolling list | 7 |
| Small landscape | T-Display | 240 x 135 | Single spot | 1 |
| Round / square | T-QT, Waveshare S3, T-Watch 2020/S3 | 128-240 sq | Compact | 1 |
| OLED small | Heltec WiFi Kit 32 (all), WiFi LoRa 32 (all) | 128 x 64 | Compact | 1 |
| OLED tiny | Heltec Wireless Stick / Stick Lite | 64 x 32 | Compact | 1 |
| TFT small | Heltec Wireless Tracker, T-Deck | 160x80 / 320x240 | Compact / List | 1-5 |

---

## 1. Landscape wide — 320 x 170 (LilyGO T-Display-S3)

### Normal mode — showing spots

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster        ●  12:34Z       │  ← header (22px)
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← spot 1 (fresh, highlighted)
│  de 9M2XYZ    FT8, strong in EU           │
│▌21.300  VK2DEF                            │  ← spot 2
│  de 9M2ABC    big signal                  │
│▌ 7.025  DL1AAA                            │  ← spot 3
│  de N0CALL    Europe opening              │
│▌28.500  ZL2XYZ                            │  ← spot 4
│  de JA1ABC    long path, strong           │
└──────────────────────────────────────────┘
```

**Details:**
- Header: "9M2PJU DX Cluster" (truncated from full title to fit), connection
  dot (cyan=connected, amber=connecting), UTC clock right-aligned.
- Animated cyan sweep line under the header (moves left to right).
- Each spot: 3px coloured bar on left (band colour), frequency + DX callsign
  on top line, "de SPOTTER  comment" on bottom line.
- Fresh spots (received < 5 seconds ago) have a blue panel background.
- 4 spots visible, scrolling, newest first.

### Setup mode

```
┌──────────────────────────────────────────┐
│        9M2PJU DX Cluster Client          │  ← cyan, centred
│            SETUP MODE                    │  ← amber, centred
│                                          │
│       9M2PJU-DXCluster-A1B2              │  ← white, the AP name
│         Open: 192.168.4.1                │  ← grey, the portal URL
│                                          │
│     Connect to the AP above              │  ← grey hint
│                                          │
│         ────────────────────             │  ← pulsing cyan line
└──────────────────────────────────────────┘
```

### Command menu (BOOT button pressed)

```
┌──────────────────────────────────────────┐
│              COMMANDS                    │  ← cyan header
├──────────────────────────────────────────┤
│▌sh/dx        Recent spots                │  ← highlighted (cyan bar + blue bg)
│ sh/dx 20     Last 20 spots               │
│ sh/dx/ft8    FT8 spots                   │
│ sh/dx/cw     CW spots                    │
│ sh/dx/ssb    SSB spots                   │
│ sh/wwv       Solar / geo                 │
│ sh/muf       MUF info                    │
│                                          │
│        tap:next  hold:send               │  ← grey footer hint
└──────────────────────────────────────────┘
```

### Command response (after sending `sh/wwv`)

```
┌──────────────────────────────────────────┐
│ > sh/wwv                                 │  ← cyan header (command sent)
├──────────────────────────────────────────┤
│ WWV Solar Flux: 142                      │  ← cluster response text
│ A Index:    5                             │
│ K Index:    2                             │
│ Sunspots:   78                            │
│ X-Ray:      B1.4                          │
│ Geomag:     Quiet                         │
│                                          │
│             tap:close                    │  ← grey footer hint
└──────────────────────────────────────────┘
```

### Connecting / no spots

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster        ●  12:34Z       │  ← amber dot (connecting)
├──────────────────────────────────────────┤
│                                          │
│       Connecting to DX cluster...        │  ← grey, centred
│                                          │
└──────────────────────────────────────────┘
```

---

## 2. Landscape wide — 320 x 240 (LilyGO T-HMI)

### Normal mode — showing spots

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client   ●  12:34Z     │  ← header (24px), full title fits
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← spot 1
│  de 9M2XYZ    FT8, strong in EU           │
│▌21.300  VK2DEF                            │  ← spot 2
│  de 9M2ABC    big signal                  │
│▌ 7.025  DL1AAA                            │  ← spot 3
│  de N0CALL    Europe opening              │
│▌28.500  ZL2XYZ                            │  ← spot 4
│  de JA1ABC    long path, strong           │
│▌18.100  W1AW                              │  ← spot 5
│  de OK2ABC    strong on 17m               │
└──────────────────────────────────────────┘
```

**Details:** Same as 320x170 but 5 spots visible and the full title
"9M2PJU DX Cluster Client" fits in the header.

---

## 3. Landscape wide — 536 x 240 (LilyGO T-Display-S3 AMOLED)

### Normal mode — showing spots

```
┌──────────────────────────────────────────────────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client                                              ●  12:34Z        │
├──────────────────────────────────────────────────────────────────────────────────────┤
│▌14.074  JA1ABC                            de 9M2XYZ    FT8, strong in EU               │
│▌21.300  VK2DEF                            de 9M2ABC    big signal on 15m              │
│▌ 7.025  DL1AAA                            de N0CALL    Europe opening                 │
│▌28.500  ZL2XYZ                            de JA1ABC    long path, strong              │
│▌18.100  W1AW                              de OK2ABC    strong on 17m                  │
└──────────────────────────────────────────────────────────────────────────────────────┘
```

**Details:** The AMOLED is very wide (536px). The full title fits, and each
spot row has plenty of room for the comment text on the same line as the
spotter. 5 spots visible. This is the most spacious layout.

---

## 4. Portrait tall — 135 x 240 (M5StickC Plus / Plus2)

### Normal mode — showing spots

```
┌──────────────┐
│ 9M2PJU  ●    │  ← header (24px), truncated title
│      12:34Z  │
├──────────────┤
│▌14.074       │  ← spot 1
│  JA1ABC      │
│  de 9M2XYZ   │
│▌21.300       │  ← spot 2
│  VK2DEF      │
│  de 9M2ABC   │
│▌ 7.025       │  ← spot 3
│  DL1AAA      │
│  de N0CALL   │
│▌28.500       │  ← spot 4
│  ZL2XYZ      │
│  de JA1ABC   │
│▌18.100       │  ← spot 5
│  W1AW        │
│  de OK2ABC   │
└──────────────┘
```

**Details:** Portrait orientation (135 wide, 240 tall). The title is
truncated to "9M2PJU" (only ~22 chars fit per line). 5 spots visible,
each showing frequency, callsign, and spotter. Comments are truncated
or omitted due to the narrow width.

### Setup mode

```
┌──────────────┐
│ 9M2PJU DX    │  ← cyan, truncated
│   SETUP      │  ← amber
│              │
│ DXCluster-   │  ← AP name (wrapped)
│ A1B2         │
│              │
│ 192.168.4.1  │  ← grey
│              │
│ Connect to   │  ← grey hint
│ the AP above │
│              │
│  ──────────  │  ← pulsing line
└──────────────┘
```

### Command menu

```
┌──────────────┐
│   COMMANDS   │
├──────────────┤
│▌sh/dx        │  ← highlighted
│ sh/dx 20     │
│ sh/dx/ft8    │
│ sh/dx/cw     │
│ sh/dx/ssb    │
│ sh/wwv       │
│ sh/muf       │
│ sh/qtc       │
│ sh/ann       │
│              │
│ tap:next     │  ← footer (truncated)
│ hold:send    │
└──────────────┘
```

**Details:** On the narrow 135px display, descriptions are hidden (only
shown when width >= 200px). The footer hint wraps to two lines. About
9 items fit without scrolling.

---

## 5. Portrait tall — 240 x 320 (M5Stack Core, Core2, Sunton CYD)

### Normal mode — showing spots

```
┌──────────────────────────┐
│ 9M2PJU DX Cluster        │  ← header (24px)
│              ●  12:34Z   │
├──────────────────────────┤
│▌14.074  JA1ABC            │  ← spot 1
│  de 9M2XYZ  FT8 strong    │
│▌21.300  VK2DEF            │  ← spot 2
│  de 9M2ABC  big signal    │
│▌ 7.025  DL1AAA            │  ← spot 3
│  de N0CALL  Europe open   │
│▌28.500  ZL2XYZ            │  ← spot 4
│  de JA1ABC  long path     │
│▌18.100  W1AW              │  ← spot 5
│  de OK2ABC  17m strong    │
│▌14.195  PY2ABC            │  ← spot 6
│  de LU1DEF  SSB, QSB      │
│▌ 3.700  K1AAA             │  ← spot 7
│  de W1BBB  80m CW         │
└──────────────────────────┘
```

**Details:** Portrait orientation (240 wide, 320 tall). 7 spots visible.
The title shows "9M2PJU DX Cluster" (truncated, full title doesn't fit
alongside the clock and connection dot). Comments are truncated to fit
the 240px width (~40 chars).

### Setup mode

```
┌──────────────────────────┐
│   9M2PJU DX Cluster      │  ← cyan
│      SETUP MODE          │  ← amber
│                          │
│   9M2PJU-DXCluster-A1B2  │  ← white
│     Open: 192.168.4.1    │  ← grey
│                          │
│   Connect to the AP      │  ← grey hint
│         above            │
│                          │
│                          │
│                          │
│                          │
│                          │
│      ────────────        │  ← pulsing cyan line
└──────────────────────────┘
```

### Command menu

```
┌──────────────────────────┐
│        COMMANDS          │
├──────────────────────────┤
│▌sh/dx       Recent spots │  ← highlighted
│ sh/dx 20    Last 20 spots │
│ sh/dx/ft8   FT8 spots     │
│ sh/dx/cw    CW spots      │
│ sh/dx/ssb   SSB spots     │
│ sh/wwv      Solar / geo   │
│ sh/muf      MUF info      │
│ sh/qtc      QTC bulletins │
│ sh/ann      Announcements │
│ sh/u        Users online  │
│ sh/c        Cluster links │
│ sh/h        Help          │
│                          │
│     tap:next  hold:send   │
└──────────────────────────┘
```

**Details:** At 240px wide, descriptions are shown (width >= 200). All
12 commands fit on screen without scrolling. The selected item has a
cyan bar on the left and a blue background.

### Command response

```
┌──────────────────────────┐
│ > sh/u                   │  ← cyan header
├──────────────────────────┤
│ 9M2PJU   G  9M2PJU-2     │
│ N0CALL   G  9M2XYZ       │
│ JA1ABC   G  VK2DEF       │
│ DL1AAA   G  OK2ABC       │
│ W1AW     G  PY2ABC       │
│ K1AAA    G  LU1DEF       │
│ ZL2XYZ   G  W1BBB        │
│                          │
│                          │
│                          │
│                          │
│                          │
│         tap:close        │
└──────────────────────────┘
```

---

## 6. Small landscape — 240 x 135 (LilyGO T-Display)

### Normal mode — single spot

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX       ●  12:34Z                │  ← header (18px)
├──────────────────────────────────────────┤
│▌14.074  JA1ABC                            │  ← frequency + callsign
│                                          │
│                                          │  ← (large vertical space)
│                                          │
│  de 9M2XYZ    FT8, strong in EU           │  ← spotter + comment
└──────────────────────────────────────────┘
```

**Details:** Only 1 spot visible (135px height with 18px header leaves
117px for one spot). The frequency and callsign are at the top of the
row, with the spotter and comment at the bottom. There is significant
vertical space between them. The title is truncated to "9M2PJU DX".

### Setup mode

```
┌──────────────────────────────────────────┐
│       9M2PJU DX Cluster Client           │  ← cyan (fits on 240px)
│            SETUP MODE                    │  ← amber
│       9M2PJU-DXCluster-A1B2              │  ← white
│         Open: 192.168.4.1                │  ← grey
│      Connect to the AP above             │  ← grey hint
└──────────────────────────────────────────┘
```

### Command menu

```
┌──────────────────────────────────────────┐
│              COMMANDS                    │
├──────────────────────────────────────────┤
│▌sh/dx        Recent spots                │  ← highlighted
│ sh/dx 20     Last 20 spots               │
│ sh/dx/ft8    FT8 spots                   │
│ sh/dx/cw     CW spots                    │
│ sh/dx/ssb    SSB spots                   │
│ sh/wwv       Solar / geo                 │
│        tap:next  hold:send               │
└──────────────────────────────────────────┘
```

**Details:** At 240px wide, descriptions are shown. About 5-6 items fit
in the available space. The menu scrolls if needed.

---

## 7. Round / square — 128 x 128 (LilyGO T-QT)

### Normal mode — compact

```
        .::::::::::::::::.
      ::::::::::::::::::::::
    ::::::::::::::::::::::::::
   ::::    9M2PJU DX     ::::   ← cyan title (truncated)
   ::::     Cluster      ::::
   ::::     12:34Z       ::::   ← white clock, centred
   ::::                  ::::
   ::::    ( ● )         ::::   ← connection ring (cyan=connected)
   ::::                  ::::
   ::::    14.074        ::::   ← frequency, large (2x size), band colour
   ::::                  ::::
   ::::   JA1ABC         ::::   ← DX callsign, very large (3x size)
   ::::                  ::::
   ::::  de 9M2XYZ       ::::   ← spotter, small, centred
    ::::::::::::::::::::::::::
      ::::::::::::::::::::::
        '::::::::::::::::'
```

**Details:** Round displays use a special compact renderer:
- "9M2PJU DX Cluster" at the top (centred, cyan)
- UTC clock below the title (centred, white)
- A connection ring drawn as a circle around the edge (cyan when
  connected, pulsing; amber when connecting)
- Frequency in large text (2x scale) in band colour
- DX callsign in very large text (3x scale), white, centred
- "de SPOTTER" in small text at the bottom

### Setup mode (round)

```
        .::::::::::::::::.
      ::::::::::::::::::::::
    ::::::::::::::::::::::::::
   ::::   9M2PJU DX     ::::   ← cyan title
   ::::     Cluster     ::::
   ::::                  ::::
   ::::     SETUP        ::::   ← amber, centred
   ::::                  ::::
   ::::  DXCluster-A1B2  ::::   ← white AP name (truncated to fit)
   ::::                  ::::
   ::::  192.168.4.1     ::::   ← grey IP address
   ::::                  ::::
    ::::::::::::::::::::::::::
      ::::::::::::::::::::::
        '::::::::::::::::'
```

### Command menu (round 128x128)

```
        .::::::::::::::::.
      ::::::::::::::::::::::
    ::::::::::::::::::::::::::
   ::::    COMMANDS     ::::   ← cyan header
   ::::::::::::::::::::::::::::
   ::::▌sh/dx           ::::   ← highlighted
   :::: sh/dx 20        ::::
   :::: sh/dx/ft8       ::::
   :::: sh/dx/cw        ::::
   :::: sh/dx/ssb       ::::
   ::::                  ::::
   ::::   tap:next       ::::   ← footer (hold:send truncated)
    ::::::::::::::::::::::::::
      ::::::::::::::::::::::
        '::::::::::::::::'
```

**Details:** On the tiny 128x128 round display, only the command labels
are shown (no descriptions, width < 200). About 5 items fit. The footer
hint may be truncated. The round bezel clips the corners of the text
area, so content is kept within the inscribed rectangle.

---

## 8. Round / square — 240 x 240 (Waveshare S3, T-Watch 2020, T-Watch S3)

### Normal mode — compact

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   ::::::                        ::::::
   ::::::    9M2PJU DX Cluster  ::::::   ← cyan title (centred)
   ::::::       12:34Z          ::::::   ← white clock
   ::::::                        ::::::
   ::::::       ( ● )           ::::::   ← connection ring (pulsing)
   ::::::                        ::::::
   ::::::      14.074            ::::::   ← frequency (2x, band colour)
   ::::::                        ::::::
   ::::::     JA1ABC             ::::::   ← DX callsign (3x, white)
   ::::::                        ::::::
   ::::::   de 9M2XYZ            ::::::   ← spotter (small, grey)
   ::::::                        ::::::
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

**Details:** Same compact layout as 128x128 but with more breathing room.
The frequency, callsign, and spotter are all larger and more readable.
The connection ring is drawn at the display edge.

### Setup mode (round 240x240)

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   ::::::                        ::::::
   ::::::   9M2PJU DX Cluster   ::::::   ← cyan title
   ::::::       SETUP           ::::::   ← amber
   ::::::                        ::::::
   ::::::  9M2PJU-DXCluster-    ::::::   ← white AP name
   ::::::       A1B2            ::::::
   ::::::                        ::::::
   ::::::    192.168.4.1        ::::::   ← grey IP
   ::::::                        ::::::
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

### Command menu (round 240x240)

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   ::::::      COMMANDS         ::::::   ← cyan header
   ::::::::::::::::::::::::::::::::
   ::::::▌sh/dx       Recent    ::::::   ← highlighted (cyan bar + blue bg)
   :::::: sh/dx 20    Last 20   ::::::
   :::::: sh/dx/ft8   FT8       ::::::
   :::::: sh/dx/cw    CW        ::::::
   :::::: sh/dx/ssb   SSB       ::::::
   :::::: sh/wwv      Solar     ::::::
   :::::: sh/muf      MUF       ::::::
   :::::: sh/qtc      QTC       ::::::
   ::::::                        ::::::
   ::::::    tap:next hold:send ::::::   ← grey footer
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

**Details:** At 240px, descriptions are shown (width >= 200) but
truncated to fit within the round bezel. About 8-9 items fit. The
round edges clip the corners, so text is kept within the inscribed
square.

### Command response (round 240x240)

```
        .::::::::::::::::::::::.
      ::::::::::::::::::::::::::::
    ::::::::::::::::::::::::::::::::
   :::::: > sh/wwv               ::::::   ← cyan header
   ::::::::::::::::::::::::::::::::
   :::::: Solar Flux: 142        ::::::
   :::::: A Index: 5             ::::::
   :::::: K Index: 2             ::::::
   :::::: Sunspots: 78           ::::::
   :::::: X-Ray: B1.4            ::::::
   :::::: Geomag: Quiet          ::::::
   ::::::                        ::::::
   ::::::                        ::::::
   ::::::       tap:close        ::::::   ← grey footer
    ::::::::::::::::::::::::::::::::
      ::::::::::::::::::::::::::::
        '::::::::::::::::::::::'
```

---

## Boot splash screen (all boards)

Shown briefly during boot, before setup mode or normal mode:

```
┌──────────────────────────────────────────┐
│ 9M2PJU DX Cluster Client                 │  ← white
│ LilyGO T-Display-S3                      │  ← board name
│ Booting...                               │  ← white
└──────────────────────────────────────────┘
```

On round displays, the text is centred and may be truncated to fit within
the round bezel.

---

## Band colour reference

The coloured bar on the left of each spot indicates the band:

```
  160m  (1.8 MHz)    = DARK BLUE   (#013F)
   80m  (3.5 MHz)    = RED         (#F800)
   60m  (5.3 MHz)    = MAGENTA     (#F81F)
   40m  (7.0 MHz)    = ORANGE      (#FD20)
   30m  (10.1 MHz)   = YELLOW      (#FFE0)
   20m  (14.0 MHz)   = GREEN       (#07E0)
   17m  (18.1 MHz)   = CYAN        (#07FF)
   15m  (21.0 MHz)   = LIGHT BLUE  (#456F)
   12m  (24.9 MHz)   = BLUE        (#017F)
   10m  (28.0 MHz)   = VIOLET      (#7B1F)
    6m  (50.0 MHz)   = PINK        (#FBA0)
    2m  (144 MHz)    = WHITE       (#FFFF)
  Other             = GREY         (#8410)
```

Example spot row with band colour:

```
 20m band →  ▌14.074  JA1ABC
              de 9M2XYZ  FT8, strong in EU

 40m band →  ▌ 7.025  DL1AAA
              de N0CALL  Europe opening

 15m band →  ▌21.300  VK2DEF
              de 9M2ABC  big signal
```

---

## Connection status indicator

The dot/ring colour indicates connection state:

```
  ●  CYAN (pulsing 2-4px)    = Connected to cluster, receiving spots
  ●  AMBER (steady 2px)      = Connecting to cluster or Wi-Fi
  ○  AMBER (empty ring)      = Disconnected, reconnecting
```

On rectangular displays, this is a dot in the header bar next to the clock.
On round displays, it's a ring drawn around the edge of the screen.

---

## Animation details

| Element | Animation | Purpose |
| :--- | :--- | :--- |
| Header sweep | Cyan line sweeps left-to-right under the header bar, ~18ms per pixel | Visual heartbeat — shows the firmware is running |
| Connection dot | Pulses between 2-4px radius every 300ms (connected) or steady 2px (connecting) | Connection health at a glance |
| Connection ring (round) | Pulses 1-3px inward every 400ms | Same, but for round displays |
| Fresh spot highlight | Blue panel background for 5 seconds after a new spot arrives | Draws attention to new spots |
| Setup mode pulse | Cyan line at bottom expands/contracts every 300ms | Indicates setup mode is active |
| Frame rate | ~30 FPS (33ms frame interval) | Smooth updates without burning CPU |
