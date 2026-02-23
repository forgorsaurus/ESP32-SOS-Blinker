#include <Arduino.h>
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
    Serial.println("\n--- ESP32 SOS Blinker ---");

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
