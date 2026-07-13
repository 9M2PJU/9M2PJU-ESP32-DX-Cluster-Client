#include "DxDisplay.h"
#include "DxClusterClient.h"
#include "CommandMenu.h"
#include "SettingsMenu.h"
#include "Battery.h"
#include <time.h>

// Theme colors (RGB565) — matched to LoRa_APRS_Tracker theme.
static constexpr uint16_t COLOR_BG          = 0x0000;  // pure black
static constexpr uint16_t COLOR_HEADER_BG   = 0x000F;  // navy blue
static constexpr uint16_t COLOR_HEADER_TEXT = 0xFFE0;  // yellow
static constexpr uint16_t COLOR_PANEL       = 0x0841;  // dark blue (fresh spot highlight)
static constexpr uint16_t COLOR_TEXT        = 0x07FF;  // cyan (body text)
static constexpr uint16_t COLOR_TEXT_DIM    = 0x7BEF;  // grey (spotter/comment)
static constexpr uint16_t COLOR_ACCENT      = 0x07FF;  // cyan
static constexpr uint16_t COLOR_ALERT       = 0xFD20;  // orange
static constexpr uint16_t COLOR_YELLOW      = 0xFFE0;  // yellow (callsigns, selected)
static constexpr uint16_t COLOR_GREEN       = 0x1482;  // green (connected)
static constexpr uint16_t COLOR_RED         = 0xF800;  // red (disconnected, errors)
static constexpr uint16_t COLOR_GRID        = 0x39E7;  // dark grey
static constexpr uint16_t COLOR_BATT_GOOD   = 0x1482;  // green (>= 50%)
static constexpr uint16_t COLOR_BATT_MID    = 0xFFE0;  // yellow (20-49%)
static constexpr uint16_t COLOR_BATT_LOW    = 0xF800;  // red (< 20%)

// UI timing.
static constexpr uint32_t FRAME_MS          = 33;
static constexpr uint32_t NEW_SPOT_MS       = 5000;

DxDisplay::DxDisplay() {}

void DxDisplay::begin() {
  _lcd.init();
  _lcd.setRotation(BOARD_DISPLAY_ROTATION);
  _lcd.setColorDepth(16);
  _lcd.fillScreen(COLOR_BG);
  _lcd.setTextWrap(false);
  _lcd.setTextColor(COLOR_TEXT);
  _lcd.setTextSize(1);

  _width = _lcd.width();
  _height = _lcd.height();

  // Adapt layout to panel size.
  _round = (_width == _height);  // square == round display
  _showComment = (_height >= 90);

  if (_round) {
    // Round displays use a dedicated compact renderer.
    _headerHeight = 0;
    _maxVisible = 1;
    _rowHeight = _height;
  } else if (_height <= 64) {
    _headerHeight = 14;
    _rowHeight = 16;
    _maxVisible = 1;
    _showComment = false;
  } else if (_height <= 135) {
    _headerHeight = 18;
    _rowHeight = (_height - _headerHeight) / 1;
    _maxVisible = 1;
    _showComment = true;
  } else if (_height <= 170) {
    _headerHeight = 22;
    _rowHeight = 34;
    _maxVisible = (_height - _headerHeight) / _rowHeight;
    _showComment = true;
  } else {
    _headerHeight = 24;
    _rowHeight = 38;
    _maxVisible = (_height - _headerHeight) / _rowHeight;
    _showComment = true;
  }
  if (_maxVisible < 1) _maxVisible = 1;

  _fullRedraw = true;
}

void DxDisplay::invalidate() { _fullRedraw = true; }

BOARD_DISPLAY_CLASS &DxDisplay::lcd() { return _lcd; }

void DxDisplay::render(DxClusterClient &client, uint32_t nowMs) {
  if (!_fullRedraw && nowMs - _lastFrameMs < FRAME_MS) {
    return;
  }
  _lastFrameMs = nowMs;

  if (_round) {
    drawRoundCompact(client, nowMs);
    _fullRedraw = false;
    return;
  }

  _lcd.fillScreen(COLOR_BG);
  drawHeader(client, nowMs);

  size_t count = client.spotCount();
  if (count == 0) {
    drawEmptyState(client.isConnected());
    _fullRedraw = false;
    return;
  }

  size_t visible = count < (size_t)_maxVisible ? count : (size_t)_maxVisible;
  for (size_t i = 0; i < visible; ++i) {
    const DxSpot *s = client.spotAt(i);
    if (!s) break;
    bool fresh = (time(nullptr) - s->receivedAt) * 1000 < (long)NEW_SPOT_MS;
    drawSpot(*s, _headerHeight + (int)i * _rowHeight, fresh, _rowHeight,
             _showComment);
  }

  _fullRedraw = false;
}

void DxDisplay::drawHeader(DxClusterClient &client, uint32_t nowMs) {
  // Navy blue header bar (LoRa_APRS_Tracker theme)
  _lcd.fillRect(0, 0, _width, _headerHeight, COLOR_HEADER_BG);
  _lcd.drawFastHLine(0, _headerHeight - 1, _width, COLOR_GRID);

  // Animated sweep under the header (cyan accent).
  int sweep = (nowMs / 18) % _width;
  int sweepStart = sweep > _headerHeight ? sweep - _headerHeight : 0;
  int sweepWidth = sweep > _headerHeight ? _headerHeight : sweep;
  if (sweepWidth > 0) {
    _lcd.drawFastHLine(sweepStart, _headerHeight - 2, sweepWidth, COLOR_ACCENT);
  }

  // UTC clock (right-aligned, yellow on navy).
  client.updateClockText(_clock, sizeof(_clock));
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_HEADER_TEXT);
  int clockW = (int)strlen(_clock) * 6;
  _lcd.setCursor(_width - clockW - 4, 4);
  _lcd.print(_clock);

  // Connection pulse dot (left of the clock: green=connected, red=disconnected).
  bool conn = client.isConnected();
  uint16_t dotColor = conn ? COLOR_GREEN : COLOR_RED;
  int pulse = conn ? 2 + ((nowMs / 300) % 3) : 2;
  int dotX = _width - clockW - 12;
  int dotY = _headerHeight / 2 - 1;
  _lcd.fillCircle(dotX, dotY, pulse, dotColor);

  // Battery indicator (left side of header).
  // On displays >= 128px: show "B:XX%" (percentage).
  // On displays >= 240px: show "B:X.XXV XX%" (voltage + percentage).
  int titleStartX = 4;
  if (Battery::hasBattery() && Battery::connected() && _width >= 128) {
    int pct = Battery::percent();
    float v = Battery::voltage();
    uint16_t battColor;
    if (pct < 20)       battColor = COLOR_BATT_LOW;
    else if (pct < 50)  battColor = COLOR_BATT_MID;
    else                battColor = COLOR_BATT_GOOD;

    _lcd.setTextSize(1);
    _lcd.setTextColor(battColor);

    if (_width >= 240) {
      // Show voltage + percentage: "B:4.15V 85%"
      char buf[20];
      snprintf(buf, sizeof(buf), "B:%.2fV %d%%", v, pct);
      _lcd.setCursor(4, 4);
      _lcd.print(buf);
      titleStartX = 4 + (int)strlen(buf) * 6 + 6;
    } else {
      // Show percentage only: "B:85%"
      char buf[12];
      snprintf(buf, sizeof(buf), "B:%d%%", pct);
      _lcd.setCursor(4, 4);
      _lcd.print(buf);
      titleStartX = 4 + (int)strlen(buf) * 6 + 6;
    }
  }

  // Title text — yellow on navy blue, truncated to fit between
  // the battery indicator and the connection dot.
  _lcd.setTextColor(COLOR_HEADER_TEXT);
  _lcd.setCursor(titleStartX, 4);
  int titleMaxW = dotX - titleStartX - 4;
  int titleMaxChars = titleMaxW / 6;
  if (titleMaxChars >= 25) {
    _lcd.print("9M2PJU DX Cluster Client");
  } else if (titleMaxChars >= 15) {
    _lcd.print("9M2PJU DX Cluster");
  } else if (titleMaxChars >= 9) {
    _lcd.print("9M2PJU DX");
  } else if (titleMaxChars >= 6) {
    _lcd.print("9M2PJU");
  }
}

void DxDisplay::drawSpot(const DxSpot &spot, int y, bool fresh,
                         int rowHeight, bool showComment) {
  uint16_t accent = bandColor(spot.frequency.toFloat());
  _lcd.fillRect(0, y, _width, rowHeight, fresh ? COLOR_PANEL : COLOR_BG);
  _lcd.fillRect(2, y + 3, 3, rowHeight - 6, accent);

  _lcd.setTextSize(1);

  // Frequency (left, band-colored to match accent bar).
  _lcd.setTextColor(accent);
  _lcd.setCursor(10, y + 3);
  _lcd.print(spot.frequency);

  // DX call (yellow, after frequency).
  int callX = 10 + (spot.frequency.length() + 1) * 6;
  if (callX > _width - 60) callX = _width - 60;
  _lcd.setTextColor(COLOR_YELLOW);
  _lcd.setCursor(callX, y + 3);
  _lcd.print(truncateForDisplay(spot.dxCall, (_width - callX - 4) / 6));

  if (showComment) {
    // Spotter in grey.
    _lcd.setTextColor(COLOR_TEXT_DIM);
    _lcd.setCursor(10, y + rowHeight - 12);
    _lcd.print("de ");
    _lcd.print(truncateForDisplay(spot.spotter, 10));

    // Comment on the right if there's room (grey).
    int commentX = 10 + (3 + spot.spotter.length() + 1) * 6;
    int commentW = _width - commentX - 4;
    if (commentW > 24) {
      _lcd.setCursor(commentX, y + rowHeight - 12);
      _lcd.print(truncateForDisplay(spot.comment, commentW / 6));
    }
  }
}

void DxDisplay::drawEmptyState(bool connected) {
  _lcd.setTextSize(1);
  _lcd.setTextColor(connected ? COLOR_TEXT : COLOR_ALERT);
  const char *msg =
      connected ? "Waiting for DX spots..." : "Connecting to DX cluster...";
  int textW = strlen(msg) * 6;
  _lcd.setCursor((_width - textW) / 2, _headerHeight + 20);
  _lcd.print(msg);
}

void DxDisplay::renderConfigMode(const String &apName,
                                 const String &ipAddress) {
  _lcd.fillScreen(COLOR_BG);

  if (_round) {
    // Round display: compact config mode
    _lcd.setTextSize(1);
    _lcd.setTextColor(COLOR_ACCENT);
    String title = "9M2PJU DX Cluster";
    int tw = title.length() * 6;
    _lcd.setCursor((_width - tw) / 2, 10);
    _lcd.print(title);

    _lcd.setTextColor(COLOR_ALERT);
    const char *setup = "SETUP";
    int sw = strlen(setup) * 6;
    _lcd.setCursor((_width - sw) / 2, 22);
    _lcd.print(setup);

    _lcd.setTextColor(COLOR_TEXT);
    _lcd.setTextSize(1);
    String ap = truncateForDisplay(apName, _width / 6 - 2);
    int apw = ap.length() * 6;
    _lcd.setCursor((_width - apw) / 2, _height / 2 - 4);
    _lcd.print(ap);

    _lcd.setTextColor(COLOR_TEXT_DIM);
    String ip = truncateForDisplay(ipAddress, _width / 6 - 2);
    int ipw = ip.length() * 6;
    _lcd.setCursor((_width - ipw) / 2, _height / 2 + 10);
    _lcd.print(ip);
    return;
  }

  // Rectangular display
  int y = _height / 2 - 36;

  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_HEADER_TEXT);  // yellow
  String title = "9M2PJU DX Cluster Client";
  int tw = title.length() * 6;
  if (tw > _width - 8) {
    title = "9M2PJU DX Cluster";
    tw = title.length() * 6;
  }
  _lcd.setCursor((_width - tw) / 2, y);
  _lcd.print(title);

  y += 14;
  _lcd.setTextColor(COLOR_ALERT);  // orange
  const char *setupLabel = "SETUP MODE";
  int slw = strlen(setupLabel) * 6;
  _lcd.setCursor((_width - slw) / 2, y);
  _lcd.print(setupLabel);

  y += 20;
  _lcd.setTextColor(COLOR_TEXT);  // cyan
  _lcd.setTextSize(1);
  String ap = truncateForDisplay(apName, _width / 6 - 2);
  int apw = ap.length() * 6;
  _lcd.setCursor((_width - apw) / 2, y);
  _lcd.print(ap);

  y += 14;
  _lcd.setTextColor(COLOR_TEXT_DIM);  // grey
  String ip = "Open: " + ipAddress;
  int ipw = ip.length() * 6;
  _lcd.setCursor((_width - ipw) / 2, y);
  _lcd.print(ip);

  y += 20;
  _lcd.setTextColor(COLOR_TEXT_DIM);  // grey
  const char *hint = "Connect to the AP above";
  int hw = strlen(hint) * 6;
  _lcd.setCursor((_width - hw) / 2, y);
  _lcd.print(hint);

  // Pulsing accent line at bottom
  uint32_t now = millis();
  int pulse = (now / 300) % 3;
  _lcd.drawFastHLine(_width / 2 - 20 - pulse * 4, _height - 6, 40 + pulse * 8,
                     COLOR_ACCENT);
}

void DxDisplay::drawRoundCompact(DxClusterClient &client,
                                 uint32_t nowMs) {
  _lcd.fillScreen(COLOR_BG);

  // Connection ring (green=connected, red=disconnected).
  bool conn = client.isConnected();
  uint16_t ringColor = conn ? COLOR_GREEN : COLOR_RED;
  _lcd.drawCircle(_width / 2, _height / 2, _height / 2 - 2, COLOR_GRID);
  if (conn) {
    int pulse = 1 + ((nowMs / 400) % 3);
    _lcd.drawCircle(_width / 2, _height / 2, _height / 2 - 2 - pulse, ringColor);
  }

  // Title at top (yellow).
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_YELLOW);
  const char *title = "9M2PJU DX Cluster";
  int titleW = strlen(title) * 6;
  _lcd.setCursor((_width - titleW) / 2, 4);
  _lcd.print(title);

  // UTC clock below title (cyan).
  client.updateClockText(_clock, sizeof(_clock));
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_TEXT);
  int clockW = strlen(_clock) * 6;
  _lcd.setCursor((_width - clockW) / 2, 16);
  _lcd.print(_clock);

  // Battery indicator (below clock, if available).
  if (Battery::hasBattery() && Battery::connected()) {
    int pct = Battery::percent();
    uint16_t battColor;
    if (pct < 20)       battColor = COLOR_BATT_LOW;
    else if (pct < 50)  battColor = COLOR_BATT_MID;
    else                battColor = COLOR_BATT_GOOD;
    _lcd.setTextColor(battColor);
    char buf[12];
    snprintf(buf, sizeof(buf), "B:%d%%", pct);
    int bw = strlen(buf) * 6;
    _lcd.setCursor((_width - bw) / 2, 28);
    _lcd.print(buf);
  }

  size_t count = client.spotCount();
  if (count == 0) {
    _lcd.setTextSize(1);
    _lcd.setTextColor(conn ? COLOR_TEXT : COLOR_ALERT);
    const char *msg = conn ? "No spots" : "Connecting";
    int w = strlen(msg) * 6;
    _lcd.setCursor((_width - w) / 2, _height / 2 + 4);
    _lcd.print(msg);
    return;
  }

  const DxSpot *s = client.spotAt(0);
  if (!s) return;
  uint16_t accent = bandColor(s->frequency.toFloat());

  // Frequency, large (band color).
  _lcd.setTextSize(2);
  _lcd.setTextColor(accent);
  int freqW = s->frequency.length() * 12;
  _lcd.setCursor((_width - freqW) / 2, _height / 2 - 18);
  _lcd.print(s->frequency);

  // DX call, large (yellow).
  _lcd.setTextSize(3);
  _lcd.setTextColor(COLOR_YELLOW);
  String call = truncateForDisplay(s->dxCall, 8);
  int callW = call.length() * 18;
  _lcd.setCursor((_width - callW) / 2, _height / 2 + 4);
  _lcd.print(call);

  // Spotter, small, bottom (grey).
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_TEXT_DIM);
  String de = "de " + truncateForDisplay(s->spotter, 12);
  int deW = de.length() * 6;
  _lcd.setCursor((_width - deW) / 2, _height - 14);
  _lcd.print(de);
}

// =============================================================================
// Command menu + response rendering
// =============================================================================

void DxDisplay::renderMenu(const CommandMenu &menu) {
  _lcd.fillScreen(COLOR_BG);

  // Header (navy blue bar with yellow text)
  _lcd.fillRect(0, 0, _width, 14, COLOR_HEADER_BG);
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_HEADER_TEXT);
  const char *title = "COMMANDS";
  int tw = strlen(title) * 6;
  _lcd.setCursor((_width - tw) / 2, 4);
  _lcd.print(title);
  _lcd.drawFastHLine(0, 14, _width, COLOR_GRID);

  // Menu items
  size_t selected = menu.selectedIndex();
  size_t count = CommandMenu::itemCount();

  // Calculate how many items fit on screen.
  int itemHeight = 18;  // 12px text + 6px gap
  int availableH = _height - 20;  // minus header
  int maxVisible = availableH / itemHeight;
  if (maxVisible < 1) maxVisible = 1;

  // Scroll window: keep the selected item visible.
  size_t startIdx = 0;
  if (count > (size_t)maxVisible) {
    if (selected >= count - maxVisible / 2) {
      startIdx = count - maxVisible;
    } else if (selected >= (size_t)maxVisible / 2) {
      startIdx = selected - maxVisible / 2;
    }
  }

  int y = 18;
  size_t endIdx = startIdx + maxVisible;
  if (endIdx > count) endIdx = count;

  for (size_t i = startIdx; i < endIdx; ++i) {
    const MenuItem &item = CommandMenu::items()[i];
    bool isSel = (i == selected);

    if (isSel) {
      _lcd.fillRect(0, y - 1, _width, itemHeight - 2, COLOR_PANEL);
      // Yellow accent bar on the left for selected
      _lcd.fillRect(0, y - 1, 3, itemHeight - 2, COLOR_YELLOW);
    }

    _lcd.setTextSize(1);
    if (isSel) {
      _lcd.setTextColor(COLOR_YELLOW);  // selected = yellow
    } else {
      _lcd.setTextColor(COLOR_TEXT);    // unselected = cyan
    }
    _lcd.setCursor(8, y);
    // Truncate label to fit
    int maxChars = (_width - 16) / 6;
    String label = truncateForDisplay(String(item.label), maxChars);
    _lcd.print(label);

    // Description on the same line if there's room, or on small displays
    // just show the label.
    if (_width >= 200) {
      int labelW = label.length() * 6;
      int descX = 8 + labelW + 8;
      int descMaxChars = (_width - descX - 4) / 6;
      if (descMaxChars > 4) {
        _lcd.setTextColor(isSel ? COLOR_TEXT_DIM : COLOR_GRID);
        _lcd.setCursor(descX, y);
        String desc = truncateForDisplay(String(item.desc), descMaxChars);
        _lcd.print(desc);
      }
    }

    y += itemHeight;
  }

  // Footer hint (orange)
  _lcd.setTextColor(COLOR_ALERT);
  _lcd.setTextSize(1);
  const char *hint = "tap:next  hold:send";
  int hw = strlen(hint) * 6;
  if (hw <= _width) {
    _lcd.setCursor((_width - hw) / 2, _height - 10);
    _lcd.print(hint);
  }
}

void DxDisplay::renderResponse(const DxClusterClient &client,
                               const String &cmdTitle) {
  _lcd.fillScreen(COLOR_BG);

  // Header (navy blue bar with yellow text)
  _lcd.fillRect(0, 0, _width, 14, COLOR_HEADER_BG);
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_HEADER_TEXT);
  String header = "> " + cmdTitle;
  int maxHeaderChars = (_width - 8) / 6;
  String headerTrunc = truncateForDisplay(header, maxHeaderChars);
  _lcd.setCursor(4, 4);
  _lcd.print(headerTrunc);
  _lcd.drawFastHLine(0, 14, _width, COLOR_GRID);

  // Response lines (cyan)
  size_t count = client.responseCount();
  int y = 18;
  int lineH = 10;  // 8px text + 2px gap
  int availableH = _height - 24;  // minus header + footer
  int maxLines = availableH / lineH;
  if (maxLines < 1) maxLines = 1;

  // Show the most recent lines (tail of the buffer).
  size_t startIdx = 0;
  if (count > (size_t)maxLines) {
    startIdx = count - maxLines;
  }

  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_TEXT);
  int maxChars = (_width - 8) / 6;

  for (size_t i = startIdx; i < count; ++i) {
    const String &line = client.responseLineAt(i);
    String trunc = truncateForDisplay(line, maxChars);
    _lcd.setCursor(4, y);
    _lcd.print(trunc);
    y += lineH;
    if (y > _height - 12) break;
  }

  // Footer hint (orange)
  _lcd.setTextColor(COLOR_ALERT);
  const char *hint = "tap:close";
  int hw = strlen(hint) * 6;
  _lcd.setCursor((_width - hw) / 2, _height - 10);
  _lcd.print(hint);
}

// =============================================================================
// Settings menu rendering
// =============================================================================

void DxDisplay::renderSettings(const SettingsMenu &menu) {
  _lcd.fillScreen(COLOR_BG);

  // Header (navy blue bar with yellow text)
  _lcd.fillRect(0, 0, _width, 14, COLOR_HEADER_BG);
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_HEADER_TEXT);
  const char *title = "SETTINGS";
  int tw = strlen(title) * 6;
  _lcd.setCursor((_width - tw) / 2, 4);
  _lcd.print(title);
  _lcd.drawFastHLine(0, 14, _width, COLOR_GRID);

  // Menu items
  size_t selected = menu.selectedIndex();
  size_t count = SettingsMenu::ITEM_COUNT;

  int itemHeight = 18;
  int availableH = _height - 20;
  int maxVisible = availableH / itemHeight;
  if (maxVisible < 1) maxVisible = 1;

  int y = 18;
  size_t endIdx = (count < (size_t)maxVisible) ? count : (size_t)maxVisible;

  for (size_t i = 0; i < endIdx; ++i) {
    const SettingsMenu::Item &item = SettingsMenu::items()[i];
    bool isSel = (i == selected);

    if (isSel) {
      _lcd.fillRect(0, y - 1, _width, itemHeight - 2, COLOR_PANEL);
      _lcd.fillRect(0, y - 1, 3, itemHeight - 2, COLOR_YELLOW);
    }

    _lcd.setTextSize(1);
    _lcd.setTextColor(isSel ? COLOR_YELLOW : COLOR_TEXT);
    _lcd.setCursor(8, y);
    int maxChars = (_width - 16) / 6;
    String label = truncateForDisplay(String(item.label), maxChars);
    _lcd.print(label);

    // Description on wider displays
    if (_width >= 160) {
      int labelW = label.length() * 6;
      int descX = 8 + labelW + 8;
      int descMaxChars = (_width - descX - 4) / 6;
      if (descMaxChars > 4) {
        _lcd.setTextColor(isSel ? COLOR_TEXT_DIM : COLOR_GRID);
        _lcd.setCursor(descX, y);
        String desc = truncateForDisplay(String(item.desc), descMaxChars);
        _lcd.print(desc);
      }
    }

    y += itemHeight;
  }

  // Footer hint (orange)
  _lcd.setTextColor(COLOR_ALERT);
  _lcd.setTextSize(1);
  const char *hint = "tap:next  hold:select";
  int hw = strlen(hint) * 6;
  if (hw <= _width) {
    _lcd.setCursor((_width - hw) / 2, _height - 10);
    _lcd.print(hint);
  }
}

void DxDisplay::renderSettingsConfirm(const SettingsMenu &menu) {
  _lcd.fillScreen(COLOR_BG);

  // Header (navy blue bar with orange text for confirm)
  _lcd.fillRect(0, 0, _width, 14, COLOR_HEADER_BG);
  _lcd.setTextSize(1);
  _lcd.setTextColor(COLOR_ALERT);
  const char *title = "CONFIRM";
  int tw = strlen(title) * 6;
  _lcd.setCursor((_width - tw) / 2, 4);
  _lcd.print(title);
  _lcd.drawFastHLine(0, 14, _width, COLOR_GRID);

  // Action name (yellow)
  size_t idx = menu.selectedIndex();
  const SettingsMenu::Item &item = SettingsMenu::items()[idx];

  _lcd.setTextColor(COLOR_YELLOW);
  _lcd.setTextSize(1);
  String label = truncateForDisplay(String(item.label), _width / 6 - 2);
  int lw = label.length() * 6;
  _lcd.setCursor((_width - lw) / 2, _height / 2 - 16);
  _lcd.print(label);

  // Yes / No choices
  bool yes = menu.confirmYes();

  int choiceY = _height / 2 + 4;
  int halfW = _width / 2;

  // No (left) — cyan when selected, grey when not
  if (!yes) {
    _lcd.fillRect(2, choiceY - 2, halfW - 4, 22, COLOR_PANEL);
    _lcd.fillRect(2, choiceY - 2, 3, 22, COLOR_TEXT);
    _lcd.setTextColor(COLOR_TEXT);
  } else {
    _lcd.setTextColor(COLOR_TEXT_DIM);
  }
  const char *noLabel = "No";
  int noW = strlen(noLabel) * 6;
  _lcd.setCursor((halfW - noW) / 2, choiceY + 2);
  _lcd.print(noLabel);

  // Yes (right) — red when selected, grey when not
  if (yes) {
    _lcd.fillRect(halfW + 2, choiceY - 2, halfW - 4, 22, COLOR_PANEL);
    _lcd.fillRect(halfW + 2, choiceY - 2, 3, 22, COLOR_RED);
    _lcd.setTextColor(COLOR_RED);
  } else {
    _lcd.setTextColor(COLOR_TEXT_DIM);
  }
  const char *yesLabel = "Yes";
  int yesW = strlen(yesLabel) * 6;
  _lcd.setCursor(halfW + (halfW - yesW) / 2, choiceY + 2);
  _lcd.print(yesLabel);

  // Footer hint (orange)
  _lcd.setTextColor(COLOR_ALERT);
  _lcd.setTextSize(1);
  const char *hint = "tap:toggle  hold:ok";
  int hw = strlen(hint) * 6;
  if (hw <= _width) {
    _lcd.setCursor((_width - hw) / 2, _height - 10);
    _lcd.print(hint);
  }
}

void DxDisplay::setBrightness(uint8_t level) {
#if BOARD_HAS_BACKLIGHT
  _lcd.setBrightness(level);
#endif
}
