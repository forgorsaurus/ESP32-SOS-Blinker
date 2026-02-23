#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "SystemConfig.h"

class ConfigManager {
public:
    ConfigManager();
    void begin();
    SystemConfig load();
    void save(const SystemConfig& config);
    void reset();
    
private:
    Preferences _prefs;
};
