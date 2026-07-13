#pragma once

/*
 * DxDisplay — board-agnostic UI renderer for the DX Cluster Client.
 *
 * It wraps the board-specific LovyanGFX subclass selected by BoardConfig.h
 * and draws:
 *   - a header bar with the project name, connection pulse, and UTC clock
 *   - a scrolling list of recent DX spots (newest first)
 *   - an empty-state message while waiting for the first spot
 *
 * The layout adapts to the panel size: small round panels (T-QT, Waveshare
 * round) get a compact single-spot view; very small panels hide the comment
 * line; larger panels show more spots.
 */

#include <Arduino.h>
#include "BoardConfig.h"
#include "DxSpot.h"

class DxClusterClient;
class CommandMenu;
class SettingsMenu;

class DxDisplay {
 public:
  DxDisplay();

  // Initialise the panel. Call once in setup().
  void begin();

  // Mark the next draw as a full redraw (clears the whole screen).
  void invalidate();

  // Render one frame. Reads spots + connection state from `client`.
  void render(DxClusterClient &client, uint32_t nowMs);

  // Render the config-mode screen (AP name + IP + instructions).
  void renderConfigMode(const String &apName, const String &ipAddress);

  // Render the command menu overlay (shows list of commands, highlight
  // the selected one). Call every frame while the menu is open.
  void renderMenu(const CommandMenu &menu);

  // Render the settings menu overlay (WiFi AP, Sleep, Restart, Brightness).
  void renderSettings(const SettingsMenu &menu);

  // Render the confirm dialog for a settings action.
  void renderSettingsConfirm(const SettingsMenu &menu);

  // Render the command response text (cluster output after a command).
  // Shows as many lines as fit on the display.
  void renderResponse(const DxClusterClient &client, const String &cmdTitle);

  // Set backlight brightness (0-255).  No-op on boards without backlight.
  void setBrightness(uint8_t level);

  // Access the underlying LovyanGFX instance (for splash screens etc.).
  BOARD_DISPLAY_CLASS &lcd();

 private:
  void drawHeader(DxClusterClient &client, uint32_t nowMs);
  void drawSpot(const DxSpot &spot, int y, bool fresh, int rowHeight,
                bool showComment);
  void drawEmptyState(bool connected);
  void drawRoundCompact(DxClusterClient &client, uint32_t nowMs);

  BOARD_DISPLAY_CLASS _lcd;
  bool _fullRedraw = true;
  uint32_t _lastFrameMs = 0;
  char _clock[8] = "--:--Z";

  // Layout cache, computed in begin().
  int _width = 0;
  int _height = 0;
  int _headerHeight = 24;
  int _rowHeight = 34;
  int _maxVisible = 4;
  bool _showComment = true;
  bool _round = false;
};
