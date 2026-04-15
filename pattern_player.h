#pragma once

#include <Arduino.h>
#include "config.h"

// One frame: colours for all LEDs + how long to show them.
struct PatternFrame {
    uint8_t  r[LED_COUNT];
    uint8_t  g[LED_COUNT];
    uint8_t  b[LED_COUNT];
    uint16_t duration_ms;
};

// Plays a sequence of PatternFrames without blocking (millis()-based).
class PatternPlayer {
public:
    // Populate frames before calling play().
    void setFrame(uint8_t idx, const PatternFrame& frame);
    void setFrameCount(uint8_t count);

    uint8_t          frameCount()            const { return _count; }
    const PatternFrame& frame(uint8_t idx)   const { return _frames[idx]; }

    // Start playback. Pass loop=true to repeat indefinitely.
    void play(bool loop = true);

    // Stop playback; leaves LEDs showing the last frame.
    void stop();

    // Call every loop() iteration.
    void update();

    bool    isPlaying()    const { return _playing; }
    uint8_t currentFrame() const { return _currentFrame; }

private:
    PatternFrame  _frames[PATTERN_MAX_FRAMES];
    uint8_t       _count        = 0;
    uint8_t       _currentFrame = 0;
    bool          _playing      = false;
    bool          _loop         = true;
    unsigned long _frameStart   = 0;

    void applyFrame(uint8_t idx);
};

extern PatternPlayer player;
