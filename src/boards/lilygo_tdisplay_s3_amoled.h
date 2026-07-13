#pragma once
/*
 * LilyGO T-Display-S3 AMOLED (1.91" RM67162 240x536, ESP32-S3, QSPI).
 * Pinout (QSPI): SCK=39, CS=40, D0=46, D1=38, D2=45, D3=48
 */
#include <LovyanGFX.hpp>

class LGFX_TDISPLAY_S3_AMOLED : public lgfx::LGFX_Device {
  lgfx::Panel_RM67162 _panel;
  lgfx::Bus_SPI _bus;

 public:
  LGFX_TDISPLAY_S3_AMOLED(void) {
    {
      auto cfg = _bus.config();
      cfg.freq_write = 24000000;
      cfg.freq_read = 16000000;
      cfg.pin_sclk = 39;
      cfg.pin_mosi = 46;  // D0
      cfg.pin_miso = -1;
      cfg.pin_dc   = -1;
      cfg.pin_io0  = 46;  // D0
      cfg.pin_io1  = 38;  // D1
      cfg.pin_io2  = 45;  // D2
      cfg.pin_io3  = 48;  // D3
      cfg.spi_mode = 0;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = 40;
      cfg.memory_width = 536;
      cfg.memory_height = 240;
      cfg.panel_width = 536;
      cfg.panel_height = 240;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
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
