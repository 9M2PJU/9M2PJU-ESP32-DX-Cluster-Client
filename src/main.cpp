#include <Arduino.h>

#if __has_include("config.h")
#include "config.h"
#else
#error "Missing include/config.h. Copy include/config.example.h to include/config.h."
#endif

#include "BoardConfig.h"
#include "AppConfig.h"
#include "DxClusterClient.h"
#include "DxDisplay.h"
#include "WebAdmin.h"
#include "CommandMenu.h"
#include "SettingsMenu.h"

#include <esp_sleep.h>

static AppConfig appConfig;
static DxClusterClient *cluster = nullptr;
static DxDisplay display;
static WebAdmin webAdmin;
static CommandMenu cmdMenu;
static SettingsMenu settingsMenu;
static bool configMode = false;
static String lastSentCommand;  // Title for the response screen

// BOOT button GPIO (common across most ESP32 boards)
static constexpr int BOOT_BUTTON_PIN = 0;

// AP name prefix for config mode
static constexpr const char *AP_PREFIX = "9M2PJU-DXCluster-";

// Forward declarations
void handleSettingsEvents(SettingsMenu::Event evt);

String makeApName() {
  uint64_t mac = ESP.getEfuseMac();
  char suffix[8];
  snprintf(suffix, sizeof(suffix), "%02X%02X",
           (uint8_t)(mac >> 8), (uint8_t)mac);
  return String(AP_PREFIX) + String(suffix);
}

bool isBootButtonHeld() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  delay(50);
  return digitalRead(BOOT_BUTTON_PIN) == LOW;
}

void startNormalMode() {
  Serial.println(F("=== Normal mode ==="));
  cluster = new DxClusterClient(MAX_STORED_DX_SPOTS);
  cluster->setWifi(appConfig.wifiSsid, appConfig.wifiPassword);
  cluster->setCluster(appConfig.clusterHost, appConfig.clusterPort,
                      appConfig.callsign, appConfig.callsignPassword,
                      appConfig.postLoginCommand);
  cluster->setNtp(appConfig.ntpPrimary, appConfig.ntpSecondary,
                  appConfig.tzOffsetSeconds, appConfig.dstOffsetSeconds);
  cluster->setReconnectDelayMs(appConfig.reconnectDelayMs);

  // Try to connect to Wi-Fi with a 30s timeout
  if (!cluster->connectWifi(30000)) {
    Serial.println(F("Wi-Fi connection failed, entering setup mode"));
    configMode = true;
    String apName = makeApName();
    WiFi.disconnect(true);
    delay(100);
    webAdmin.begin(appConfig, apName);
    display.renderConfigMode(apName, "192.168.4.1");
    return;
  }

  cluster->syncClock();
  display.invalidate();

  // Ensure BOOT button is ready for menu use (input with pull-up).
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
}

void startConfigMode() {
  Serial.println(F("=== Setup mode ==="));
  configMode = true;
  String apName = makeApName();
  webAdmin.begin(appConfig, apName);
  display.renderConfigMode(apName, "192.168.4.1");
}

void setup() {
  Serial.begin(115200);
  delay(300);

  // Load config from NVS (with compile-time defaults as fallback)
  appConfig.load();

  // Boards with a PMU (e.g. T-Watch 2020 AXP202) need it initialised
  // before the display will receive power.
#if defined(BOARD_NEEDS_PMU_INIT)
  initTWatch2020PMU();
  delay(100);
#endif

  // Boards with a peripheral power-enable pin (e.g. T-Deck GPIO 10)
  // need it set HIGH before the display will receive power.
#if defined(BOARD_NEEDS_POWER_INIT)
  initTDeckPower();
  delay(100);
#endif

  // Initialise display
  display.begin();
  // Apply saved backlight brightness
  settingsMenu.setBrightness(appConfig.backlightBrightness);
  display.setBrightness(settingsMenu.currentBrightness());
  display.lcd().setTextSize(1);
  display.lcd().setTextColor(0xFFFF);
  display.lcd().setCursor(6, 6);
  display.lcd().print("9M2PJU DX Cluster Client");
  display.lcd().setCursor(6, 20);
  display.lcd().print(BOARD_NAME);
  display.lcd().setCursor(6, 34);
  display.lcd().print("Booting...");

  // Decide: config mode or normal mode?
  // 1. BOOT button held → config mode
  // 2. No Wi-Fi SSID in NVS → config mode
  // 3. Otherwise → normal mode (falls back to config on Wi-Fi failure)
  bool buttonHeld = isBootButtonHeld();
  bool hasWifi = appConfig.hasWifiConfig();

  Serial.printf("BOOT button: %s, WiFi configured: %s\n",
                buttonHeld ? "HELD" : "no",
                hasWifi ? "yes" : "no");

  if (buttonHeld || !hasWifi) {
    startConfigMode();
  } else {
    startNormalMode();
  }
}

void loop() {
  if (configMode) {
    webAdmin.loop();
    // Refresh config mode display periodically (for the pulsing animation)
    static uint32_t lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 100) {
      lastDisplayUpdate = millis();
      String apName = makeApName();
      display.renderConfigMode(apName, "192.168.4.1");
    }
    if (webAdmin.shouldReboot()) {
      Serial.println(F("Rebooting after config save..."));
      delay(500);
      ESP.restart();
    }
    delay(2);
    return;
  }

  // ---- Normal mode ----
  cluster->loop();

  // Always run the settings menu button watcher.  When CLOSED, it
  // counts clicks for triple-click detection but does not consume
  // individual presses (CommandMenu still sees them).  When a
  // triple-click is detected, it opens and takes over the button.
  SettingsMenu::Event sEvt = settingsMenu.loop();

  if (sEvt == SettingsMenu::Event::OPENED) {
    // Triple-click detected — close command menu if it opened
    // during the click sequence.
    cmdMenu.close();
  }

  if (settingsMenu.isActive()) {
    // ---- Settings menu is active ----
    handleSettingsEvents(sEvt);

    // Render settings UI
    switch (settingsMenu.state()) {
      case SettingsMenu::State::BROWSING:
        display.renderSettings(settingsMenu);
        break;
      case SettingsMenu::State::CONFIRM:
        display.renderSettingsConfirm(settingsMenu);
        break;
      default:
        break;
    }
  } else {
    // ---- Normal spot viewing / command menu ----
    CommandMenu::Event evt = cmdMenu.loop();
    switch (evt) {
      case CommandMenu::Event::SEND:
        if (cluster->isConnected()) {
          lastSentCommand = String(cmdMenu.selectedItem().label);
          cluster->sendCommand(cmdMenu.selectedItem().command);
          Serial.printf("Menu: sending '%s'\n", cmdMenu.selectedItem().command);
        } else {
          Serial.println(F("Menu: not connected, cannot send"));
          cmdMenu.close();
        }
        break;
      case CommandMenu::Event::CLOSED:
        display.invalidate();  // Force full redraw when returning to spots
        break;
      default:
        break;
    }

    // Render based on menu state.
    switch (cmdMenu.state()) {
      case CommandMenu::State::CLOSED:
        display.render(*cluster, millis());
        break;
      case CommandMenu::State::BROWSING:
        display.renderMenu(cmdMenu);
        break;
      case CommandMenu::State::SHOWING:
        display.renderResponse(*cluster, lastSentCommand);
        break;
    }
  }

  delay(2);
}

// Handle settings menu events (WiFi AP, Sleep, Restart, Brightness).
void handleSettingsEvents(SettingsMenu::Event evt) {
  switch (evt) {
    case SettingsMenu::Event::OPENED:
      Serial.println(F("Settings menu opened (triple-click)"));
      break;

    case SettingsMenu::Event::EXECUTE: {
      SettingsMenu::Action action = settingsMenu.pendingAction();
      switch (action) {
        case SettingsMenu::Action::BRIGHTNESS: {
          // Cycle to next brightness level
          uint8_t level = settingsMenu.cycleBrightness();
          display.setBrightness(level);
          appConfig.backlightBrightness = level;
          appConfig.save();
          Serial.printf("Brightness set to %u\n", level);
          break;
        }
        case SettingsMenu::Action::ENABLE_WIFI_AP:
          if (settingsMenu.confirmYes()) {
            Serial.println(F("Settings: rebooting into setup mode"));
            display.lcd().fillScreen(0x0008);
            display.lcd().setTextSize(1);
            display.lcd().setTextColor(0x07FF);
            display.lcd().setCursor(6, BOARD_DISPLAY_HEIGHT / 2 - 4);
            display.lcd().print("Rebooting to setup...");
            delay(800);
            // Clear WiFi config so it boots into setup mode
            appConfig.wifiSsid = "";
            appConfig.save();
            ESP.restart();
          }
          break;
        case SettingsMenu::Action::SLEEP:
          if (settingsMenu.confirmYes()) {
            Serial.println(F("Settings: entering deep sleep"));
            display.lcd().fillScreen(0x0008);
            display.lcd().setTextSize(1);
            display.lcd().setTextColor(0xBDF7);
            display.lcd().setCursor(6, BOARD_DISPLAY_HEIGHT / 2 - 4);
            display.lcd().print("Going to sleep...");
            delay(800);
            // Wake on BOOT button (GPIO 0, active low)
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
            esp_deep_sleep_start();
          }
          break;
        case SettingsMenu::Action::RESTART:
          if (settingsMenu.confirmYes()) {
            Serial.println(F("Settings: restarting"));
            display.lcd().fillScreen(0x0008);
            display.lcd().setTextSize(1);
            display.lcd().setTextColor(0x07FF);
            display.lcd().setCursor(6, BOARD_DISPLAY_HEIGHT / 2 - 4);
            display.lcd().print("Restarting...");
            delay(800);
            ESP.restart();
          }
          break;
        default:
          break;
      }
      // Return to browsing after execute (unless we rebooted/slept)
      break;
    }

    case SettingsMenu::Event::CLOSED:
      Serial.println(F("Settings menu closed"));
      display.invalidate();  // Force full redraw of spot view
      break;

    default:
      break;
  }
}
