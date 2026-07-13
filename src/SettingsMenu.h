#pragma once

/*
 * SettingsMenu — one-button settings menu triggered by triple-click.
 *
 * Triple-click (3 short presses within 800ms) opens the settings menu
 * from the normal spot-viewing mode.  Navigation is the same as
 * CommandMenu:
 *
 *   CLOSED    →  triple-click      →  BROWSING
 *   BROWSING  →  short press       →  next item
 *   BROWSING  →  long press        →  SELECT (execute or enter confirm)
 *   BROWSING  →  10s timeout       →  CLOSED
 *   CONFIRM   →  short press       →  toggle Yes / No
 *   CONFIRM   →  long press        →  execute selected choice
 *   CONFIRM   →  10s timeout       →  BROWSING
 *
 * Items:
 *   0. Enable WiFi AP  — reboot into setup mode (captive portal)
 *   1. Sleep            — enter deep sleep, wake with BOOT button
 *   2. Restart          — soft reboot
 *   3. Brightness       — cycle backlight level (only on boards with backlight)
 *
 * Brightness is cycled immediately on select (no confirm screen).
 * WiFi AP, Sleep, and Restart show a Yes/No confirm screen.
 */

#include <Arduino.h>

class SettingsMenu {
 public:
  enum class State { CLOSED, BROWSING, CONFIRM };
  enum class Event { NONE, OPENED, NEXT, SELECT, EXECUTE, CLOSED };

  enum class Action {
    NONE,             // no action selected yet
    ENABLE_WIFI_AP,   // reboot into setup mode
    SLEEP,            // deep sleep
    RESTART,          // soft reboot
    BRIGHTNESS,       // cycle brightness (already applied)
  };

  static constexpr int BUTTON_PIN = 0;
  static constexpr uint16_t SHORT_PRESS_MS = 800;
  static constexpr uint16_t TRIPLE_CLICK_WINDOW_MS = 800;
  static constexpr uint16_t BROWSING_TIMEOUT_MS = 10000;
  static constexpr uint16_t CONFIRM_TIMEOUT_MS = 10000;

  // Brightness levels (0-255).  Index 0 = off is skipped.
  static constexpr size_t BRIGHTNESS_LEVEL_COUNT = 4;
  static const uint8_t *brightnessLevels() {
    static const uint8_t levels[] = {64, 128, 192, 255};
    return levels;
  }

  static constexpr size_t ITEM_COUNT = 4;

  struct Item {
    const char *label;
    const char *desc;
    bool needsConfirm;
  };

  static const Item *items() {
    static const Item table[] = {
      {"WiFi AP",    "Reboot to setup",   true},
      {"Sleep",      "Deep sleep",        true},
      {"Restart",    "Soft reboot",       true},
      {"Brightness", "Backlight level",   false},
    };
    return table;
  }

  SettingsMenu()
    : _state(State::CLOSED), _selectedIndex(0), _confirmYes(false),
      _btnWasUp(true), _btnDownMs(0), _longFired(false),
      _stateEnterMs(0), _lastEvent(Event::NONE),
      _clickCount(0), _firstClickMs(0), _brightnessIdx(2) {}

  State state() const { return _state; }
  size_t selectedIndex() const { return _selectedIndex; }
  bool confirmYes() const { return _confirmYes; }
  Action pendingAction() const { return _pendingAction; }
  uint8_t currentBrightness() const { return brightnessLevels()[_brightnessIdx]; }

  // True if the settings menu is active (BROWSING or CONFIRM).
  bool isActive() const { return _state != State::CLOSED; }

  // Call from loop() every iteration.  Reads the button and returns
  // any event that occurred.  When CLOSED, watches for triple-click
  // but does NOT consume individual presses (so CommandMenu can still
  // see them).
  Event loop() {
    _lastEvent = Event::NONE;
    bool btnUp = (digitalRead(BUTTON_PIN) == HIGH);
    uint32_t now = millis();

    // ---- Button edge detection ----
    if (btnUp && !_btnWasUp) {
      // Button just released.
      uint16_t held = now - _btnDownMs;
      if (held >= SHORT_PRESS_MS) {
        // Long press release.
        if (_state == State::BROWSING && !_longFired) {
          handleSelect();
        } else if (_state == State::CONFIRM && !_longFired) {
          _lastEvent = Event::EXECUTE;
          // State transition handled by caller via pendingAction()
        }
      } else {
        // Short press release.
        handleShortPress(now);
      }
      _longFired = false;
    }

    if (!btnUp && _btnWasUp) {
      // Button just pressed.
      _btnDownMs = now;
      _longFired = false;
    }

    // Long-press detection while holding (fires immediately).
    if (!btnUp && !_longFired) {
      if (_state == State::BROWSING &&
          now - _btnDownMs >= SHORT_PRESS_MS) {
        _longFired = true;
        handleSelect();
      } else if (_state == State::CONFIRM &&
                 now - _btnDownMs >= SHORT_PRESS_MS) {
        _longFired = true;
        _lastEvent = Event::EXECUTE;
      }
    }

    // Triple-click window expiry (reset click counter).
    if (_state == State::CLOSED && _clickCount > 0 &&
        now - _firstClickMs > TRIPLE_CLICK_WINDOW_MS) {
      _clickCount = 0;
    }

    // Timeouts.
    if (_state == State::BROWSING &&
        now - _stateEnterMs > BROWSING_TIMEOUT_MS) {
      _state = State::CLOSED;
      _lastEvent = Event::CLOSED;
      _clickCount = 0;
    }
    if (_state == State::CONFIRM &&
        now - _stateEnterMs > CONFIRM_TIMEOUT_MS) {
      _state = State::BROWSING;
      _stateEnterMs = now;
      _pendingAction = Action::NONE;
    }

    _btnWasUp = btnUp;
    return _lastEvent;
  }

  void close() {
    _state = State::CLOSED;
    _lastEvent = Event::NONE;
    _clickCount = 0;
    _pendingAction = Action::NONE;
  }

  // Cycle to the next brightness level and return the new value.
  uint8_t cycleBrightness() {
    _brightnessIdx = (_brightnessIdx + 1) % BRIGHTNESS_LEVEL_COUNT;
    return brightnessLevels()[_brightnessIdx];
  }

  // Set brightness index from a stored value (called on boot).
  void setBrightness(uint8_t value) {
    // Find the closest level.
    const uint8_t *levels = brightnessLevels();
    size_t best = 0;
    uint8_t bestDiff = 255;
    for (size_t i = 0; i < BRIGHTNESS_LEVEL_COUNT; ++i) {
      uint8_t diff = (levels[i] > value)
                       ? (levels[i] - value)
                       : (value - levels[i]);
      if (diff < bestDiff) {
        bestDiff = diff;
        best = i;
      }
    }
    _brightnessIdx = best;
  }

 private:
  State _state;
  size_t _selectedIndex;
  bool _confirmYes;
  Action _pendingAction;

  bool _btnWasUp;
  uint32_t _btnDownMs;
  bool _longFired;
  uint32_t _stateEnterMs;
  Event _lastEvent;

  // Triple-click tracking (only used when CLOSED).
  uint8_t _clickCount;
  uint32_t _firstClickMs;
  size_t _brightnessIdx;

  void handleShortPress(uint32_t now) {
    if (_state == State::CLOSED) {
      // Count clicks for triple-click detection.
      _clickCount++;
      if (_clickCount == 1) {
        _firstClickMs = now;
      } else if (_clickCount >= 3) {
        // Triple-click detected!
        _clickCount = 0;
        _state = State::BROWSING;
        _selectedIndex = 0;
        _stateEnterMs = now;
        _lastEvent = Event::OPENED;
      }
      // Individual clicks are NOT consumed here — CommandMenu
      // still sees them.  main.cpp closes CommandMenu when it
      // gets the OPENED event.
    } else if (_state == State::BROWSING) {
      _selectedIndex = (_selectedIndex + 1) % ITEM_COUNT;
      _stateEnterMs = now;
      _lastEvent = Event::NEXT;
    } else if (_state == State::CONFIRM) {
      _confirmYes = !_confirmYes;
      _stateEnterMs = now;
    }
  }

  void handleSelect() {
    const Item &item = items()[_selectedIndex];
    if (item.needsConfirm) {
      // Enter confirm screen.
      _state = State::CONFIRM;
      _confirmYes = false;
      _stateEnterMs = millis();
      _lastEvent = Event::SELECT;
      // Map index to action.
      switch (_selectedIndex) {
        case 0: _pendingAction = Action::ENABLE_WIFI_AP; break;
        case 1: _pendingAction = Action::SLEEP; break;
        case 2: _pendingAction = Action::RESTART; break;
        default: _pendingAction = Action::NONE; break;
      }
    } else {
      // No confirm needed — execute immediately (brightness cycle).
      _pendingAction = Action::BRIGHTNESS;
      _lastEvent = Event::EXECUTE;
      // Stay in BROWSING so the user can cycle again.
      _stateEnterMs = millis();
    }
  }
};
