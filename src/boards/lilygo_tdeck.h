#pragma once
/*
 * LilyGO T-Deck (ESP32-S3FN16R8, 2.8" ST7789 320x240, SPI).
 *
 * The T-Deck is a handheld device with a 2.8" display, physical keyboard
 * (I2C), trackball navigation, LoRa (SX1262), GPS, and microphone.
 *
 * GPIO 10 is the peripheral power-enable pin — it must be set HIGH before
 * the display (and other peripherals) will receive power. Call
 * initTDeckPower() once in setup() before display.begin().
 *
 * Display pinout:
 *   MOSI=41, MISO=38, SCK=40, CS=12, DC=11, BL=42
 *   Power Enable=10
 *
 * Trackball (not used by this firmware, but documented for reference):
 *   Left=1, Right=2, Up=3, Down=15, Press=0 (BOOT)
 *
 * Keyboard (I2C, not used by this firmware):
 *   SDA=18, SCL=8, INT=46
 */
#include <LovyanGFX.hpp>

// T-Deck peripheral power-enable pin.
static constexpr int TDECK_POWER_PIN = 10;

// Enable peripheral power (display, keyboard, LoRa, etc.) on the T-Deck.
// Call this once in setup() before display.begin().
inline void initTDeckPower() {
  pinMode(TDECK_POWER_PIN, OUTPUT);
  digitalWrite(TDECK_POWER_PIN, HIGH);
  Serial.println(F("T-Deck peripheral power enabled (GPIO 10)"));
}

class LGFX_TDECK : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;
  lgfx::Light_PWM _light;

 public:
  LGFX_TDECK(void) {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI3_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 40;
      cfg.pin_mosi = 41;
      cfg.pin_miso = 38;
      cfg.pin_dc = 11;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.memory_width = 320;
      cfg.memory_height = 240;
      cfg.panel_width = 320;
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
      cfg.pin_bl = 42;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    setPanel(&_panel);
  }
};
