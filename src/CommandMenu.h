#pragma once

/*
 * CommandMenu — one-button menu for sending DX cluster commands.
 *
 * Uses the BOOT button (GPIO0, active LOW) to navigate and send commands:
 *
 *   CLOSED    →  short press  →  BROWSING (menu opens)
 *   BROWSING  →  short press  →  next item
 *   BROWSING  →  long press   →  SEND (sends command, shows response)
 *   BROWSING  →  10s timeout  →  CLOSED
 *   SHOWING   →  short press  →  CLOSED
 *   SHOWING   →  8s timeout   →  CLOSED
 *
 * "Short press"  = button held < 800ms then released.
 * "Long press"   = button held ≥ 800ms (fires on release).
 *
 * The class only produces events; main.cpp decides what to do with them.
 */

#include <Arduino.h>

struct MenuItem {
  const char *label;    // Short display label, e.g. "sh/dx"
  const char *command;  // Full command to send, e.g. "sh/dx"
  const char *desc;     // One-line description, e.g. "Recent spots"
};

class CommandMenu {
 public:
  enum class State { CLOSED, BROWSING, SHOWING };
  enum class Event { NONE, OPENED, NEXT, SEND, CLOSED };

  static constexpr int BUTTON_PIN = 0;
  static constexpr uint16_t SHORT_PRESS_MS = 800;
  static constexpr uint16_t BROWSING_TIMEOUT_MS = 10000;
  static constexpr uint16_t SHOWING_TIMEOUT_MS = 8000;

  // Common DXSpider commands.
  static constexpr size_t ITEM_COUNT = 12;
  static const MenuItem *items();
  static size_t itemCount();

  CommandMenu() : _state(State::CLOSED), _selectedIndex(0),
                  _btnWasUp(true), _btnDownMs(0), _longFired(false),
                  _stateEnterMs(0), _lastEvent(Event::NONE) {}

  State state() const { return _state; }
  size_t selectedIndex() const { return _selectedIndex; }
  const MenuItem &selectedItem() const { return items()[_selectedIndex]; }

  // Call from loop(). Reads the button and returns any event that occurred.
  Event loop() {
    _lastEvent = Event::NONE;
    bool btnUp = (digitalRead(BUTTON_PIN) == HIGH);
    uint32_t now = millis();

    if (btnUp && !_btnWasUp) {
      // Button just released.
      uint16_t held = now - _btnDownMs;
      if (held >= SHORT_PRESS_MS) {
        // Long press release.
        if (_state == State::BROWSING && !_longFired) {
          _lastEvent = Event::SEND;
          _state = State::SHOWING;
          _stateEnterMs = now;
        }
      } else {
        // Short press release.
        if (_state == State::CLOSED) {
          _state = State::BROWSING;
          _stateEnterMs = now;
          _selectedIndex = 0;
          _lastEvent = Event::OPENED;
        } else if (_state == State::BROWSING) {
          _selectedIndex = (_selectedIndex + 1) % ITEM_COUNT;
          _stateEnterMs = now;
          _lastEvent = Event::NEXT;
        } else if (_state == State::SHOWING) {
          _state = State::CLOSED;
          _lastEvent = Event::CLOSED;
        }
      }
      _longFired = false;
    }

    if (!btnUp && _btnWasUp) {
      // Button just pressed.
      _btnDownMs = now;
      _longFired = false;
    }

    // Long-press detection while holding (fires immediately, not on release).
    if (!btnUp && !_longFired && _state == State::BROWSING) {
      if (now - _btnDownMs >= SHORT_PRESS_MS) {
        _longFired = true;
        _lastEvent = Event::SEND;
        _state = State::SHOWING;
        _stateEnterMs = now;
      }
    }

    // Timeouts.
    if (_state == State::BROWSING &&
        now - _stateEnterMs > BROWSING_TIMEOUT_MS) {
      _state = State::CLOSED;
      _lastEvent = Event::CLOSED;
    }
    if (_state == State::SHOWING &&
        now - _stateEnterMs > SHOWING_TIMEOUT_MS) {
      _state = State::CLOSED;
      _lastEvent = Event::CLOSED;
    }

    _btnWasUp = btnUp;
    return _lastEvent;
  }

  // Force-close the menu (e.g. if connection drops).
  void close() {
    _state = State::CLOSED;
    _lastEvent = Event::NONE;
  }

 private:
  State _state;
  size_t _selectedIndex;
  bool _btnWasUp;
  uint32_t _btnDownMs;
  bool _longFired;
  uint32_t _stateEnterMs;
  Event _lastEvent;
};

inline const MenuItem *CommandMenu::items() {
  static const MenuItem table[] = {
    {"sh/dx",       "sh/dx",        "Recent spots"},
    {"sh/dx 20",    "sh/dx 20",     "Last 20 spots"},
    {"sh/dx/ft8",   "sh/dx/ft8",    "FT8 spots"},
    {"sh/dx/cw",    "sh/dx/cw",     "CW spots"},
    {"sh/dx/ssb",   "sh/dx/ssb",    "SSB spots"},
    {"sh/wwv",      "sh/wwv",       "Solar / geo"},
    {"sh/muf",      "sh/muf",       "MUF info"},
    {"sh/qtc",      "sh/qtc",       "QTC bulletins"},
    {"sh/ann",      "sh/ann",       "Announcements"},
    {"sh/u",        "sh/u",         "Users online"},
    {"sh/c",        "sh/c",         "Cluster links"},
    {"sh/h",        "sh/h",         "Help"},
  };
  return table;
}

inline size_t CommandMenu::itemCount() {
  return ITEM_COUNT;
}
