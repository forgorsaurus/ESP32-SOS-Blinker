#include "ConfigManager.h"
#include <Esp.h>

ConfigManager::ConfigManager() {}

void ConfigManager::begin() {
    _prefs.begin("blinker", false);
}

SystemConfig ConfigManager::load() {
    SystemConfig cfg;
    
    cfg.device_name = _prefs.getString("dev_name", "blinker-esp32");
    
    cfg.wifi_ssid = _prefs.getString("wifi_ssid", "");
    cfg.wifi_pass = _prefs.getString("wifi_pass", "");
    cfg.wifi_dhcp = _prefs.getBool("wifi_dhcp", true);
    cfg.wifi_ip = _prefs.getString("wifi_ip", "");
    cfg.wifi_gateway = _prefs.getString("wifi_gateway", "");
    cfg.wifi_subnet = _prefs.getString("wifi_subnet", "");
    cfg.wifi_dns = _prefs.getString("wifi_dns", "");
    
    cfg.ap_ssid = _prefs.getString("ap_ssid", "");
    if (cfg.ap_ssid.length() == 0) {
        // Construct default AP SSID with MAC suffix
        uint64_t mac = ESP.getEfuseMac();
        char buf[32];
        // Use last 2 bytes of MAC for uniqueness
        sprintf(buf, "SOSBLINK-ESP32-%04X", (uint16_t)(mac >> 32)); 
        // Actually, getEfuseMac returns MAC address. The high bytes are usually OUI.
        // Let's use low bytes for uniqueness.
        sprintf(buf, "SOSBLINK-ESP32-%04X", (uint16_t)(mac & 0xFFFF));
        cfg.ap_ssid = String(buf);
    }
    
    cfg.ap_pass = _prefs.getString("ap_pass", "");
    cfg.ap_timeout = _prefs.getUShort("ap_to", 300);
    
    cfg.ota_enabled = _prefs.getBool("ota_en", true);
    cfg.ota_url = _prefs.getString("ota_url", "");
    cfg.ota_check_interval = _prefs.getUInt("ota_int", 86400);
    
    return cfg;
}

void ConfigManager::save(const SystemConfig& cfg) {
    _prefs.putString("dev_name", cfg.device_name);
    _prefs.putString("wifi_ssid", cfg.wifi_ssid);
    _prefs.putString("wifi_pass", cfg.wifi_pass);
    _prefs.putBool("wifi_dhcp", cfg.wifi_dhcp);
    _prefs.putString("wifi_ip", cfg.wifi_ip);
    _prefs.putString("wifi_gateway", cfg.wifi_gateway);
    _prefs.putString("wifi_subnet", cfg.wifi_subnet);
    _prefs.putString("wifi_dns", cfg.wifi_dns);
    _prefs.putString("ap_ssid", cfg.ap_ssid);
    _prefs.putString("ap_pass", cfg.ap_pass);
    _prefs.putUShort("ap_to", cfg.ap_timeout);
    _prefs.putBool("ota_en", cfg.ota_enabled);
    _prefs.putString("ota_url", cfg.ota_url);
    _prefs.putUInt("ota_int", cfg.ota_check_interval);
}

void ConfigManager::reset() {
    _prefs.clear();
}
