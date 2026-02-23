#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "SystemConfig.h"
#include "ConfigManager.h"

class NetworkManager {
public:
    NetworkManager(ConfigManager& configMgr);
    void begin();
    void update();
    bool isConnected();
    bool isAPMode();

private:
    ConfigManager& _configMgr;
    SystemConfig _config;
    
    WebServer _server;
    DNSServer _dnsServer;
    
    bool _apMode;
    unsigned long _lastWifiCheck;
    
    void startSTA();
    void startAP();
    void setupWebServer();
    
    // Web Handlers
    void handleRoot();
    void handleSave();
    void handleNotFound();
    
    // OTA Handlers
    void handleUpdate();
    void handleUpload();
};
