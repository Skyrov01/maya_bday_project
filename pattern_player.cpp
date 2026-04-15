#include "pattern_player.h"
#include "led_controller.h"

PatternPlayer player;

void PatternPlayer::setFrame(uint8_t idx, const PatternFrame& frame) {
    if (idx < PATTERN_MAX_FRAMES) {
        _frames[idx] = frame;
    }
}

void PatternPlayer::setFrameCount(uint8_t count) {
    _count = (count <= PATTERN_MAX_FRAMES) ? count : PATTERN_MAX_FRAMES;
}

void PatternPlayer::play(bool loop) {
    if (_count == 0) return;
    _loop         = loop;
    _currentFrame = 0;
    _playing      = true;
    _frameStart   = millis();
    applyFrame(0);
}

void PatternPlayer::stop() {
    _playing = false;
}

void PatternPlayer::update() {
    if (!_playing || _count == 0) return;

    unsigned long elapsed = millis() - _frameStart;
    if (elapsed < _frames[_currentFrame].duration_ms) return;

    uint8_t next = _currentFrame + 1;
    if (next >= _count) {
        if (!_loop) { _playing = false; return; }
        next = 0;
    }
    _currentFrame = next;
    _frameStart   = millis();
    applyFrame(_currentFrame);
}

void PatternPlayer::applyFrame(uint8_t idx) {
    const PatternFrame& f = _frames[idx];
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        leds.setPixel(i, f.r[i], f.g[i], f.b[i]);
    }
    leds.show();
}
