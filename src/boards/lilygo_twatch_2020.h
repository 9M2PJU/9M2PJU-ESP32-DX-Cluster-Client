#pragma once
/*
 * LilyGO T-Watch 2020 V1 (ESP32, 1.54" ST7789 240x240, SPI).
 *
 * The T-Watch 2020 uses an AXP202 PMU for power management. The display
 * backlight is powered by AXP202 LDO2, which must be enabled via I2C
 * before the display will show anything. Call initTWatch2020PMU() once
 * in setup() before display.begin().
 *
 * Pinout (V1, most common version):
 *   MOSI=19, SCLK=18, CS=5, DC=27, RST=N/A, BL=12
 *   AXP202 I2C: SDA=21, SCL=22, address=0x35
 *
 * V3 variant uses BL=15 instead of 12 — see TWATCH_BL_PIN below.
 *
 * Display: ST7789, 240x240, 1.54 inch.
 */
#include <LovyanGFX.hpp>
#include <Wire.h>

// Backlight pin. V1 = GPIO12, V3 = GPIO15. Change if you have a V3.
#ifndef TWATCH_BL_PIN
#define TWATCH_BL_PIN 12
#endif

// AXP202 I2C address and registers.
static constexpr uint8_t AXP202_I2C_ADDR = 0x35;
static constexpr uint8_t AXP202_REG_POWER_ENABLE = 0x12;   // LDO2 enable = bit 2
static constexpr uint8_t AXP202_REG_LDO23_VOLTAGE = 0x28;  // LDO2 voltage [7:4]

// Enable AXP202 LDO2 (display backlight power) via I2C.
// Call this once in setup() before display.begin().
inline void initTWatch2020PMU() {
  Wire.begin(21, 22);  // SDA=21, SCL=22
  Wire.beginTransmission(AXP202_I2C_ADDR);

  // Read current power-enable register.
  Wire.write(AXP202_REG_POWER_ENABLE);
  Wire.endTransmission(false);
  Wire.requestFrom((int)AXP202_I2C_ADDR, 1);
  uint8_t powerEnable = Wire.read();

  // Set bit 2 to enable LDO2.
  powerEnable |= 0x04;
  Wire.beginTransmission(AXP202_I2C_ADDR);
  Wire.write(AXP202_REG_POWER_ENABLE);
  Wire.write(powerEnable);
  Wire.endTransmission();

  // Set LDO2 voltage to 3.3V (value 0xF in bits [7:4]).
  Wire.beginTransmission(AXP202_I2C_ADDR);
  Wire.write(AXP202_REG_LDO23_VOLTAGE);
  Wire.write(0xF0);  // LDO2 = 3.3V, LDO3 = 1.8V (unused)
  Wire.endTransmission();

  Serial.println(F("T-Watch AXP202 LDO2 enabled (display power)"));
}

class LGFX_TWATCH_2020 : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Light_PWM _light;

 public:
  LGFX_TWATCH_2020(void) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 18;
      cfg.pin_mosi = 19;
      cfg.pin_miso = -1;
      cfg.pin_dc = 27;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.memory_width = 240;
      cfg.memory_height = 240;
      cfg.panel_width = 240;
      cfg.panel_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel.config(cfg);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl = TWATCH_BL_PIN;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    setPanel(&_panel);
  }
};
