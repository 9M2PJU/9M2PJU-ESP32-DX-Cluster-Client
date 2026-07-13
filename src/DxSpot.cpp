#include "DxSpot.h"

bool parseDxSpot(const String &line, DxSpot &spot) {
  // DXSpider spot lines start with "DX de ".
  if (!line.startsWith("DX de ")) {
    return false;
  }

  int colon = line.indexOf(':');
  if (colon < 6) {
    return false;
  }

  spot.spotter = line.substring(6, colon);
  spot.spotter.trim();

  int index = colon + 1;

  // Skip whitespace
  while (index < (int)line.length() && isspace((unsigned char)line[index])) {
    index++;
  }

  // frequency token
  int fstart = index;
  while (index < (int)line.length() && !isspace((unsigned char)line[index])) {
    index++;
  }
  spot.frequency = line.substring(fstart, index);

  // skip whitespace
  while (index < (int)line.length() && isspace((unsigned char)line[index])) {
    index++;
  }

  // dx call token
  int cstart = index;
  while (index < (int)line.length() && !isspace((unsigned char)line[index])) {
    index++;
  }
  spot.dxCall = line.substring(cstart, index);

  // remainder is the comment
  spot.comment = line.substring(index);
  spot.comment.trim();

  // DXSpider often appends a clock like "1234Z" at the very end of the comment.
  // Strip a trailing token that looks like 4 digits + Z.
  int len = spot.comment.length();
  if (len >= 5) {
    if (spot.comment[len - 1] == 'Z' &&
        isdigit((unsigned char)spot.comment[len - 2]) &&
        isdigit((unsigned char)spot.comment[len - 3]) &&
        isdigit((unsigned char)spot.comment[len - 4]) &&
        isdigit((unsigned char)spot.comment[len - 5])) {
      spot.comment.remove(len - 5);
      spot.comment.trim();
    }
  }

  if (spot.frequency.isEmpty() || spot.dxCall.isEmpty()) {
    return false;
  }

  spot.receivedAt = time(nullptr);
  spot.valid = true;
  return true;
}

uint16_t bandColor(float mhz) {
  if (mhz < 2.0f)   return 0xB81F;  // 160m  - magenta
  if (mhz < 4.0f)   return 0x07FF;  // 80m   - cyan
  if (mhz < 8.0f)   return 0x07E0;  // 40m   - green
  if (mhz < 15.0f)  return 0xFFE0;  // 20m   - yellow
  if (mhz < 22.0f)  return 0xFD20;  // 15m   - orange
  if (mhz < 30.0f)  return 0xF81F;  // 10m   - red-blue
  if (mhz < 55.0f)  return 0xF800;  // 6m    - red
  if (mhz < 250.0f) return 0x7BEF;  // VHF   - light grey
  if (mhz < 1500.0f)return 0x8410;  // UHF   - dark teal
  return 0xFFFF;                       // SHF   - white
}

String truncateForDisplay(const String &text, size_t maxChars) {
  if (text.length() <= maxChars) {
    return text;
  }
  if (maxChars < 2) {
    return text.substring(0, maxChars);
  }
  return text.substring(0, maxChars - 1) + F(".");
}
