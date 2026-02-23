#pragma once
#include "definitions.h"

class SOSBlinker {
public:
    SOSBlinker(uint8_t pin);
    void begin();
    void update();
    bool isRunning() const;

private:
    uint8_t _pin;
    unsigned long _lastUpdate;
    int _state; // Current step in the sequence
};
