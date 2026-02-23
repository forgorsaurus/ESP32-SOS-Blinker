#include <Arduino.h>
#include <esp_ota_ops.h>
#include "definitions.h"
#include "SOSBlinker.h"
#include "ConfigManager.h"
#include "NetworkManager.h"

// Components
SOSBlinker sosBlinker(PIN_LED_SOS);
ConfigManager configMgr;
NetworkManager netMgr(configMgr);

void setup() {
    Serial.begin(115200);
    delay(100); // Give serial some time
    Serial.println("Booting...");

    // OTA Rollback protection: Mark this image as valid
    esp_ota_mark_app_valid_cancel_rollback();

    // Initialize Configuration
    configMgr.begin();

    // Initialize Network (handles Status LED)
    netMgr.begin();

    // Initialize SOS Blinker (handles SOS LED)
    sosBlinker.begin();
    
    Serial.println("System Initialized");
}

void loop() {
    // Network tasks (Web server, WiFi, Button check)
    netMgr.update();
    
    // Blinker tasks
    sosBlinker.update();
    
    // Small yield to prevent watchdog trigger if loop is tight, 
    // though WiFi libs usually yield.
    delay(1); 
}
