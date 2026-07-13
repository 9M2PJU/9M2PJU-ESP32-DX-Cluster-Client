#pragma once
/*
 * Heltec Wireless Tracker (0.96" ST7735 160x80, ESP32-S3, SPI).
 * Also includes SX1262 LoRa + UC6580 GNSS (unused by this firmware).
 * Pinout (SPI): MOSI=42, SCLK=41, DC=40, RST=39, CS=38
 * Backlight: GPIO21 (active low), TFT power ctrl: GPIO3 (active low)
 */
#include <LovyanGFX.hpp>

class LGFX_HELTEC_WIRELESS_TRACKER : public lgfx::LGFX_Device {
  lgfx::Panel_ST7735 _panel;
  lgfx::Bus_SPI _bus;

 public:
  LGFX_HELTEC_WIRELESS_TRACKER(void) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 20000000;
      cfg.pin_sclk = 41;
      cfg.pin_mosi = 42;
      cfg.pin_miso = -1;
      cfg.pin_dc = 40;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = 38;
      cfg.pin_rst = 39;
      cfg.memory_width = 160;
      cfg.memory_height = 80;
      cfg.panel_width = 160;
      cfg.panel_height = 80;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel.config(cfg);
    }
    setPanel(&_panel);
  }
};
