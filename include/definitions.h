#pragma once
#include <Arduino.h>

// Pins
#define PIN_BTN_CONFIG 9
#define PIN_LED_SOS    12
#define PIN_LED_STATUS 13

// Timing (ms)
#define MORSE_UNIT     250
#define DOT_DURATION   MORSE_UNIT
#define DASH_DURATION  (3 * MORSE_UNIT)
#define GAP_SYMBOL     MORSE_UNIT
#define GAP_LETTER     (4 * MORSE_UNIT)
#define GAP_WORD       (12 * MORSE_UNIT)
