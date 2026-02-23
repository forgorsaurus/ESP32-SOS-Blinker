#include "SOSBlinker.h"

struct PatternStep {
    bool on;
    uint8_t units;
};

// Define the SOS pattern
// S: ...
// O: ---
// S: ...
// Timings per FSD:
// Dot: 1 unit
// Dash: 3 units
// Gap (symbol): 1 unit
// Gap (letter): 4 units
// Gap (word): 12 units
const PatternStep SOS_PATTERN[] = {
    // S
    {true, 1}, {false, 1}, // .
    {true, 1}, {false, 1}, // .
    {true, 1}, {false, 4}, // . (End of S, Gap Letter 4)
    
    // O
    {true, 3}, {false, 1}, // -
    {true, 3}, {false, 1}, // -
    {true, 3}, {false, 4}, // - (End of O, Gap Letter 4)
    
    // S
    {true, 1}, {false, 1}, // .
    {true, 1}, {false, 1}, // .
    {true, 1}, {false, 12} // . (End of S, Gap Word 12)
};

const int PATTERN_LEN = sizeof(SOS_PATTERN) / sizeof(PatternStep);

SOSBlinker::SOSBlinker(uint8_t pin) : _pin(pin), _state(0) {}

void SOSBlinker::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _lastUpdate = millis();
    _state = 0;
}

void SOSBlinker::update() {
    unsigned long currentMillis = millis();
    unsigned long elapsed = currentMillis - _lastUpdate;
    
    unsigned long duration = SOS_PATTERN[_state].units * MORSE_UNIT;
    
    if (elapsed >= duration) {
        _state = (_state + 1) % PATTERN_LEN;
        _lastUpdate = currentMillis;
    }
    
    bool on = SOS_PATTERN[_state].on;
    digitalWrite(_pin, on ? HIGH : LOW);
}

bool SOSBlinker::isRunning() const {
    return true;
}
