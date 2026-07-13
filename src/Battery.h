#pragma once

/*
 * Battery — simple ADC-based battery voltage monitoring.
 *
 * Reads the battery voltage via a board-specific ADC pin and voltage
 * divider.  Provides both voltage (in volts) and an estimated charge
 * percentage for a single-cell LiPo (3.0 V = 0%, 4.2 V = 100%).
 *
 * Boards with a PMU (AXP192/AXP202) are not supported here — their
 * BOARD_BATTERY_ADC_PIN is set to 255 and hasBattery() returns false.
 */

#include <Arduino.h>
#include "BoardConfig.h"

class Battery {
 public:
  // Initialize the ADC pin (called once from setup()).
  static void begin() {
#if BOARD_BATTERY_ADC_PIN != 255
    analogReadResolution(12);
    pinMode(BOARD_BATTERY_ADC_PIN, INPUT);
    cacheLastReadMs() = 0;
    cacheVoltage() = 0.0f;
    cacheValid() = false;
#endif
  }

  // True if this board has a battery ADC pin.
  static bool hasBattery() {
    return BOARD_BATTERY_ADC_PIN != 255;
  }

  // Read the battery voltage in volts.  Averages 16 samples for
  // noise reduction.  Call at most a few times per second.
  static float readVoltage() {
#if BOARD_BATTERY_ADC_PIN == 255
    return 0.0f;
#else
    // Dummy read to settle the ADC.
    analogRead(BOARD_BATTERY_ADC_PIN);
    delay(2);
    uint32_t sum = 0;
    constexpr int N = 16;
    for (int i = 0; i < N; ++i) {
      sum += analogRead(BOARD_BATTERY_ADC_PIN);
      delayMicroseconds(500);
    }
    float avg = (float)sum / N;
    // Convert 12-bit ADC (0-4095) to voltage (0-3.3 V), then apply
    // the board-specific voltage divider ratio.
    float v = (avg / 4095.0f) * 3.3f * BOARD_BATTERY_DIVIDER;
    cacheVoltage() = v;
    cacheValid() = (v > 1.5f);  // below 1.5 V = no battery connected
    cacheLastReadMs() = millis();
    return v;
#endif
  }

  // Estimate charge percentage from voltage (LiPo: 3.0 V - 4.2 V).
  static int voltageToPercent(float v) {
    if (v <= 3.0f) return 0;
    if (v >= 4.2f) return 100;
    // Use a slightly nonlinear curve for better real-world accuracy.
    // 3.0-3.5 V = 0-50%, 3.5-4.2 V = 50-100%.
    if (v < 3.5f) {
      return (int)((v - 3.0f) / 0.5f * 50.0f);
    }
    return (int)(50.0f + (v - 3.5f) / 0.7f * 50.0f);
  }

  // Get the last cached voltage (or read if never read).
  static float voltage() {
#if BOARD_BATTERY_ADC_PIN == 255
    return 0.0f;
#else
    float &v = cacheVoltage();
    uint32_t &lastMs = cacheLastReadMs();
    if (lastMs == 0) return readVoltage();
    return v;
#endif
  }

  // Get the last cached percentage.
  static int percent() {
#if BOARD_BATTERY_ADC_PIN == 255
    return -1;
#else
    if (cacheLastReadMs() == 0) readVoltage();
    return voltageToPercent(cacheVoltage());
#endif
  }

  // True if a battery was detected (voltage > 1.5 V on last read).
  static bool connected() {
#if BOARD_BATTERY_ADC_PIN == 255
    return false;
#else
    if (cacheLastReadMs() == 0) readVoltage();
    return cacheValid();
#endif
  }

  // Update the cache if enough time has passed (call from loop()).
  // intervalMs defaults to 30 seconds — the ADC reading is slow.
  static void update(uint32_t intervalMs = 30000) {
#if BOARD_BATTERY_ADC_PIN != 255
    uint32_t &lastMs = cacheLastReadMs();
    if (lastMs == 0 || (millis() - lastMs) > intervalMs) {
      readVoltage();
    }
#endif
  }

 private:
#if BOARD_BATTERY_ADC_PIN != 255
  // Use static-local references to avoid C++17 inline variable warnings.
  static float &cacheVoltage() { static float v = 0.0f; return v; }
  static uint32_t &cacheLastReadMs() { static uint32_t m = 0; return m; }
  static bool &cacheValid() { static bool b = false; return b; }
#endif
};
