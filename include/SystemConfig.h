#pragma once
#include <Arduino.h>

struct SystemConfig {
    // Device
    String device_name;
    
    // WiFi
    String wifi_ssid;
    String wifi_pass;
    bool wifi_dhcp;
    String wifi_ip;
    String wifi_gateway;
    String wifi_subnet;
    String wifi_dns;
    
    // AP
    String ap_ssid;
    String ap_pass;
    uint16_t ap_timeout;
    
    // OTA
    bool ota_enabled;
    String ota_url;
    uint32_t ota_check_interval;
};

// Default config generator
inline SystemConfig getDefaultConfig() {
    SystemConfig cfg;
    cfg.device_name = "blinker-esp32";
    cfg.wifi_ssid = "";
    cfg.wifi_pass = "";
    cfg.wifi_dhcp = true;
    cfg.wifi_ip = "";
    cfg.wifi_gateway = "";
    cfg.wifi_subnet = "";
    cfg.wifi_dns = "";
    
    uint32_t mac = 0;
    // We can't easily get MAC here without WiFi init, so use placeholder
    cfg.ap_ssid = "SOSBLINK-ESP32"; 
    cfg.ap_pass = "";
    cfg.ap_timeout = 300;
    
    cfg.ota_enabled = true;
    cfg.ota_url = "";
    cfg.ota_check_interval = 86400;
    
    return cfg;
}
