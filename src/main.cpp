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

static AppConfig appConfig;
static DxClusterClient *cluster = nullptr;
static DxDisplay display;
static WebAdmin webAdmin;
static CommandMenu cmdMenu;
static bool configMode = false;
static String lastSentCommand;  // Title for the response screen

// BOOT button GPIO (common across most ESP32 boards)
static constexpr int BOOT_BUTTON_PIN = 0;

// AP name prefix for config mode
static constexpr const char *AP_PREFIX = "9M2PJU-DXCluster-";

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
  } else {
    cluster->loop();

    // Read the BOOT button and process menu events.
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

    delay(2);
  }
}
