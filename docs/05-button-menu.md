# Button Command Menu

The 9M2PJU ESP32 DX Cluster Client has a one-button command menu that lets
you send common DXSpider commands to the cluster without any extra hardware.
It uses only the **BOOT** button that is already present on most ESP32
boards (GPIO 0, active LOW), so it works on every supported board with no
wiring changes.

This guide describes how the button is read, the menu states, the available
commands, how responses are displayed, and the timing rules that govern the
menu.

---

## The BOOT Button in Normal Mode

Outside of setup mode, GPIO 0 is configured as an input with the internal
pull-up resistor enabled:

```cpp
pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
```

The button reads **LOW when pressed** and HIGH when released. The
`CommandMenu` class (in `src/CommandMenu.h`) is polled every `loop()`
iteration; it debounces by edge-detecting transitions and measures how long
the button was held to distinguish short and long presses.

> During setup mode the BOOT button is reserved for forcing setup mode at
> boot. The command menu only runs in normal mode.

### Press definitions

| Press type | Condition | Fires |
| --- | --- | --- |
| Short press | Button held **< 800 ms** then released | On release |
| Long press | Button held **>= 800 ms** | Immediately while still held (in BROWSING state) |

The 800 ms threshold is defined as `CommandMenu::SHORT_PRESS_MS`.

---

## Menu States

The menu is a small finite-state machine with three states, defined in
`CommandMenu::State`:

| State | Meaning | Display |
| --- | --- | --- |
| `CLOSED` | Menu inactive. Normal spot display is shown. | Live DX spots. |
| `BROWSING` | Menu is open; the user is cycling through commands. | Command list with the current selection highlighted. |
| `SHOWING` | A command was sent; the cluster response is on screen. | Response text from the cluster. |

### State diagram

```
                      short press
        +-----------------------------------------+
        |                                         |
        v                                         |
   +---------+    short press    +-----------+    |
   | CLOSED  | ----------------> | BROWSING  |    |
   +---------+                   +-----------+    |
        ^                          |     |  |     |
        |                          |     |  |     |
        |   10s timeout            |     |  |     |
        +--------------------------+   long press |
        |                          |     |  |     |
        |                          |     v  v     |
        |                          |  +--------+  |
        |                          |  |SHOWING |  |
        |   short press / 8s       |  +--------+  |
        +--------------------------+     |        |
                                   <-----+--------+
```

Transitions in detail:

| From | To | Trigger |
| --- | --- | --- |
| `CLOSED` | `BROWSING` | Short press (opens menu, resets selection to item 0) |
| `BROWSING` | `BROWSING` | Short press (advance to next item, wraps around) |
| `BROWSING` | `SHOWING` | Long press (sends the selected command) |
| `BROWSING` | `CLOSED` | 10-second inactivity timeout |
| `SHOWING` | `CLOSED` | Short press (dismiss) |
| `SHOWING` | `CLOSED` | 8-second inactivity timeout |

The menu can also be force-closed by `CommandMenu::close()`, which the main
loop calls if a command is requested while the cluster connection is down.

---

## Timeouts

| State | Timeout | Constant |
| --- | --- | --- |
| `BROWSING` | 10 seconds with no button activity | `BROWSING_TIMEOUT_MS` |
| `SHOWING` | 8 seconds after entering the state | `SHOWING_TIMEOUT_MS` |

When a timeout fires the menu returns to `CLOSED` and the main loop forces a
full redraw of the spot list (`display.invalidate()`). Timeouts are checked
every `loop()` call against the millis timestamp recorded when the state was
entered.

---

## Available Commands

The menu exposes 12 predefined DXSpider commands. They are stored in a
static table in `CommandMenu::items()`:

| # | Label | Command sent | Description | Output type |
| --- | --- | --- | --- | --- |
| 1 | `sh/dx` | `sh/dx` | Recent spots | Spot list |
| 2 | `sh/dx 20` | `sh/dx 20` | Last 20 spots | Spot list |
| 3 | `sh/dx/ft8` | `sh/dx/ft8` | FT8 spots | Spot list |
| 4 | `sh/dx/cw` | `sh/dx/cw` | CW spots | Spot list |
| 5 | `sh/dx/ssb` | `sh/dx/ssb` | SSB spots | Spot list |
| 6 | `sh/wwv` | `sh/wwv` | Solar / geomagnetic info | Text |
| 7 | `sh/muf` | `sh/muf` | MUF info | Text |
| 8 | `sh/qtc` | `sh/qtc` | QTC bulletins | Text |
| 9 | `sh/ann` | `sh/ann` | Announcements | Text |
| 10 | `sh/u` | `sh/u` | Users online | Text |
| 11 | `sh/c` | `sh/c` | Cluster links | Text |
| 12 | `sh/h` | `sh/h` | Help | Text |

Short presses cycle through this list in order, wrapping from item 12 back
to item 1.

---

## Spot Commands vs Text Commands

Although every menu entry is sent to the cluster the same way
(`DxClusterClient::sendCommand()`), the firmware treats the **response**
differently depending on whether each incoming line parses as a DX spot.

For every line received from the cluster, `DxClusterClient::handleLine()`
attempts `parseDxSpot()`:

- **If the line parses as a DX spot** it is pushed onto the spot ring buffer
  (`pushSpot()`), which is the same buffer the live spot display reads from.
  The spot appears in the main spot list and is shown the next time the
  display renders the spot view.
- **If the line does not parse as a spot** it is appended to the response
  text buffer (`pushResponse()`), which is what the `SHOWING` screen
  displays.

Practical effect:

- The `sh/dx*` commands (items 1-5) refresh the **spot list**. Their output
  is not shown as raw text on the response screen; instead the spots update
  in the background, and when the menu times out or is dismissed you see the
  new spots on the main display.
- The remaining commands (`sh/wwv`, `sh/muf`, `sh/qtc`, `sh/ann`, `sh/u`,
  `sh/c`, `sh/h`) produce **text** that is shown verbatim on the response
  screen while the menu is in the `SHOWING` state.

Login/password prompt lines and empty lines are filtered out and never
stored as response text.

---

## Sending a Command

When a long press triggers a `SEND` event, the main loop (`src/main.cpp`)
does the following:

```cpp
case CommandMenu::Event::SEND:
  if (cluster->isConnected()) {
    lastSentCommand = String(cmdMenu.selectedItem().label);
    cluster->sendCommand(cmdMenu.selectedItem().command);
  } else {
    cmdMenu.close();   // not connected: abort back to CLOSED
  }
  break;
```

`sendCommand()` clears the response buffer, sends the command string
followed by `\r\n`, and the menu transitions to `SHOWING`. Incoming lines
are collected into the response buffer as they arrive, so the response
screen fills in progressively.

---

## Display Layout

### BROWSING screen (command list)

Rendered by `DxDisplay::renderMenu()`:

```
+----------------------------------+
|            COMMANDS              |
|----------------------------------|
|  sh/dx        Recent spots       |  <- highlighted (accent bar)
|  sh/dx 20     Last 20 spots      |
|  sh/dx/ft8    FT8 spots          |
|  sh/dx/cw     CW spots           |
|  sh/dx/ssb    SSB spots          |
|  sh/wwv       Solar / geo        |
|              ...                 |
|       tap:next  hold:send        |
+----------------------------------+
```

- A centered **COMMANDS** header sits above a grid line.
- The currently selected item is shown with a highlighted panel and an
  accent bar on the left edge.
- Each row shows the command label and, on displays at least 200 px wide,
  a dimmed one-line description to the right.
- The list scrolls to keep the selection visible when there are more items
  than fit on screen.
- A footer hint reads **`tap:next  hold:send`**.

### SHOWING screen (response)

Rendered by `DxDisplay::renderResponse()`:

```
+----------------------------------+
| > sh/wwv                         |
|----------------------------------|
|  Solar flux 142, A-index 4       |
|  K-index 1, no storms            |
|  MUF 21MHz at this location      |
|              ...                 |
|           tap:close              |
+----------------------------------+
```

- The header shows the command that was sent, prefixed with `>`.
- The body shows the most recent response lines (the tail of the ring
  buffer), truncated to the screen width.
- A footer hint reads **`tap:close`**.
- After 8 seconds, or on a short press, the menu returns to `CLOSED` and
  the live spot display is redrawn.

### CLOSED screen (normal spot view)

When the menu is closed the display returns to the normal spot view
(`DxDisplay::render()`), showing the UTC clock, connection indicator, and
the most recent DX spots. A full redraw is forced on every return from the
menu so stale menu pixels are cleared.

---

## Quick Reference

| Action | Result |
| --- | --- |
| Short press (from spots) | Open the command menu |
| Short press (in menu) | Move selection to the next command |
| Long press (in menu) | Send the selected command and show its response |
| Short press (on response) | Close the menu and return to spots |
| Do nothing for 10 s (in menu) | Menu auto-closes |
| Do nothing for 8 s (on response) | Response auto-closes |
