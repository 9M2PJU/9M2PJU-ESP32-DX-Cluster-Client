#pragma once

/*
 * DxSpot — parsed DX cluster spot line.
 *
 * DXSpider emits lines of the form:
 *
 *   DX de <spotter>: <freq> <dxcall> <comment>
 *
 * e.g.
 *   DX de 9M2PJU:    14.074  JA1ABC    FT8, strong signal in EU
 *
 * The optional trailing clock HHMMZ is stripped from the comment.
 */

#include <Arduino.h>
#include <time.h>

struct DxSpot {
  String spotter;
  String frequency;   // MHz, as a string to preserve formatting
  String dxCall;
  String comment;
  time_t receivedAt = 0;
  bool valid = false;
};

// Parse a single telnet line into a DxSpot. Returns true on success.
bool parseDxSpot(const String &line, DxSpot &spot);

// Return an RGB565 color hinting at the band of a frequency (in MHz).
uint16_t bandColor(float mhz);

// Truncate a string to fit maxChars visible characters, appending an ellipsis.
String truncateForDisplay(const String &text, size_t maxChars);
