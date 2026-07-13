#pragma once
/*
 * M5StickC Plus / Plus2 (ESP32-PICO, 1.14" ST7789 135x240, SPI).
 * Pinout: MOSI=15, SCLK=13, CS=5, DC=23, RST=18, BL=-1 (AXP power)
 *
 * The backlight on the StickC is controlled through the AXP192 PMU, not a
 * direct GPIO. LovyanGFX's M5 autodetect handles the AXP; here we use a
 * minimal panel config and rely on the board's default power state.
 */
#include <LovyanGFX.hpp>

class LGFX_M5STICKC_PLUS : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;

 public:
  LGFX_M5STICKC_PLUS(void) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = VSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 27000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 13;
      cfg.pin_mosi = 15;
      cfg.pin_miso = -1;
      cfg.pin_dc = 23;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.memory_width = 240;
      cfg.memory_height = 135;
      cfg.panel_width = 240;
      cfg.panel_height = 135;
      cfg.offset_x = 0;
      cfg.offset_y = 40;
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
    setPanel(&_panel);
  }
};
