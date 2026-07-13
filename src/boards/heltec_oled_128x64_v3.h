#pragma once
/*
 * Heltec ESP32-S3 boards with 0.96" SSD1306 OLED 128x64 via I2C.
 * Used by: WiFi Kit 32 (V3), WiFi LoRa 32 (V3).
 * Pinout: SDA=GPIO17, SCL=GPIO18, RST=GPIO21, I2C addr=0x3C
 */
#include <LovyanGFX.hpp>

class LGFX_HELTEC_OLED_128x64_V3 : public lgfx::LGFX_Device {
  lgfx::Panel_SSD1306 _panel;
  lgfx::Bus_I2C _bus;

 public:
  LGFX_HELTEC_OLED_128x64_V3(void) {
    {
      auto cfg = _bus.config();
      cfg.i2c_port = 0;
      cfg.freq_write = 400000;
      cfg.freq_read = 400000;
      cfg.pin_sda = 17;
      cfg.pin_scl = 18;
      cfg.i2c_addr = 0x3C;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.panel_width = 128;
      cfg.panel_height = 64;
      cfg.memory_width = 128;
      cfg.memory_height = 64;
      cfg.pin_rst = 21;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel.config(cfg);
    }
    setPanel(&_panel);
  }
};
