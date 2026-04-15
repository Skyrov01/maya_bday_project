#pragma once

#include <Arduino.h>
#include "led_controller.h"

enum class Effect : uint8_t {
    NONE        = 0,
    CHASE       = 1,
    RAINBOW     = 2,
    SPARKLE     = 3,
    COMET       = 4,
    COLOR_WIPE  = 5,
    PING_PONG   = 6,
    PULSE       = 7,
    WAVE        = 8,
    STROBE      = 9,
    THEATER     = 10,
    KALEIDOSCOPE= 11
};

// Convert between Effect enum and its JSON string name.
const char* effectName(Effect e);
Effect      effectFromStr(const char* name);

// Non-blocking effect engine — call update() every loop() iteration.
// Internally uses millis()-based timing; never calls delay().
class LedPatterns {
public:
    // Start (or restart) an effect.
    void startEffect(Effect e);

    // Stop the current effect; leaves LEDs in their current state.
    void stop();

    // Advance the animation by one step if enough time has elapsed.
    void update();

    Effect currentEffect() const { return _current; }

private:
    Effect        _current  = Effect::NONE;
    unsigned long _lastTick = 0;
    int           _a        = 0; // primary state counter
    int           _b        = 0; // secondary / sub-state counter

    // Returns true and resets _lastTick when 'ms' have elapsed.
    bool elapsed(unsigned int ms);

    void tick_chase();
    void tick_rainbow();
    void tick_sparkle();
    void tick_comet();
    void tick_color_wipe();
    void tick_ping_pong();
    void tick_pulse();
    void tick_wave();
    void tick_strobe();
    void tick_theater();
    void tick_kaleidoscope();
};

extern LedPatterns patterns;
