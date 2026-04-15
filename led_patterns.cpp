#include "led_patterns.h"

LedPatterns patterns;

// ─── Static colour tables ─────────────────────────────────────────────────────

static const uint32_t SPARKLE_COLORS[7] = {
    0xFF0000, 0x00FF00, 0x0000FF,
    0xFFC800, 0x00FFFF, 0xFF00FF, 0xFFFFFF
};

static const uint32_t WIPE_COLORS[4] = {
    0xFF0000, 0x00FF00, 0x0000FF, 0xFF5000
};

// ─── Helpers ──────────────────────────────────────────────────────────────────

static inline uint8_t hi(uint32_t c) { return (c >> 16) & 0xFF; }
static inline uint8_t mi(uint32_t c) { return (c >>  8) & 0xFF; }
static inline uint8_t lo(uint32_t c) { return  c        & 0xFF; }

bool LedPatterns::elapsed(unsigned int ms) {
    unsigned long now = millis();
    if (now - _lastTick >= ms) {
        _lastTick = now;
        return true;
    }
    return false;
}

// ─── Public API ───────────────────────────────────────────────────────────────

void LedPatterns::startEffect(Effect e) {
    _current  = e;
    _a        = 0;
    _b        = 0;
    _lastTick = 0; // causes first tick to fire immediately
}

void LedPatterns::stop() {
    _current = Effect::NONE;
}

void LedPatterns::update() {
    if (_current == Effect::NONE) return;
    switch (_current) {
        case Effect::CHASE:      tick_chase();      break;
        case Effect::RAINBOW:    tick_rainbow();    break;
        case Effect::SPARKLE:    tick_sparkle();    break;
        case Effect::COMET:      tick_comet();      break;
        case Effect::COLOR_WIPE: tick_color_wipe(); break;
        case Effect::PING_PONG:  tick_ping_pong();  break;
        default: break;
    }
}

// ─── Effect ticks ─────────────────────────────────────────────────────────────

void LedPatterns::tick_chase() {
    if (!elapsed(50)) return;
    leds.clear();
    leds.setPixel((uint16_t)_a, 0, 0, 255);
    leds.show();
    _a = (_a + 1) % LED_COUNT;
}

void LedPatterns::tick_rainbow() {
    if (!elapsed(10)) return;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        leds.raw().setPixelColor(i, leds.raw().ColorHSV((uint16_t)((_a + i * 25) * 256)));
    }
    leds.show();
    _a = (_a + 1) % 256;
}

void LedPatterns::tick_sparkle() {
    if (_b == 0) {
        // Waiting phase: pick and light a random pixel.
        if (!elapsed(25)) return;
        _a = random(LED_COUNT);
        uint32_t c = SPARKLE_COLORS[random(7)];
        leds.setPixel((uint16_t)_a, hi(c), mi(c), lo(c));
        leds.show();
        _b = 1;
    } else {
        // Lit phase: extinguish after the same delay.
        if (!elapsed(25)) return;
        leds.setPixel((uint16_t)_a, 0, 0, 0);
        leds.show();
        _b = 0;
    }
}

void LedPatterns::tick_comet() {
    if (!elapsed(55)) return;
    leds.clear();
    for (int t = 0; t < 5; t++) {
        int pos = _a - t;
        if (pos >= 0 && pos < LED_COUNT) {
            uint8_t bright = (uint8_t)(255 - t * 50);
            leds.setPixel((uint16_t)pos, bright, bright / 2, 0);
        }
    }
    leds.show();
    if (++_a >= LED_COUNT + 5) _a = 0;
}

void LedPatterns::tick_color_wipe() {
    // _b = colour index (0-3)
    // _a = 0..LED_COUNT-1 → lighting phase
    //      LED_COUNT       → first pause (colour on)
    //      LED_COUNT+1     → second pause (strip cleared)
    if (_a < LED_COUNT) {
        if (!elapsed(55)) return;
        uint32_t c = WIPE_COLORS[_b];
        leds.setPixel((uint16_t)_a, hi(c), mi(c), lo(c));
        leds.show();
        _a++;
    } else if (_a == LED_COUNT) {
        if (!elapsed(150)) return;
        leds.clear();
        leds.show();
        _a++;
    } else {
        if (!elapsed(80)) return;
        _b = (_b + 1) % 4;
        _a = 0;
    }
}

void LedPatterns::tick_ping_pong() {
    if (!elapsed(35)) return;
    leds.clear();
    leds.setPixel((uint16_t)_a, 255, 255, 255);
    leds.show();
    if (_b == 0) {          // forward
        if (++_a >= LED_COUNT) { _a = LED_COUNT - 2; _b = 1; }
    } else {                // backward
        if (--_a < 0)           { _a = 1;             _b = 0; }
    }
}

// ─── Name helpers ─────────────────────────────────────────────────────────────

const char* effectName(Effect e) {
    switch (e) {
        case Effect::CHASE:      return "chase";
        case Effect::RAINBOW:    return "rainbow";
        case Effect::SPARKLE:    return "sparkle";
        case Effect::COMET:      return "comet";
        case Effect::COLOR_WIPE: return "colorwipe";
        case Effect::PING_PONG:  return "pingpong";
        default:                 return "none";
    }
}

Effect effectFromStr(const char* name) {
    if (strcmp(name, "chase")     == 0) return Effect::CHASE;
    if (strcmp(name, "rainbow")   == 0) return Effect::RAINBOW;
    if (strcmp(name, "sparkle")   == 0) return Effect::SPARKLE;
    if (strcmp(name, "comet")     == 0) return Effect::COMET;
    if (strcmp(name, "colorwipe") == 0) return Effect::COLOR_WIPE;
    if (strcmp(name, "pingpong")  == 0) return Effect::PING_PONG;
    return Effect::NONE;
}
