#include "led_patterns.h"

LedPatterns patterns;

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
        case Effect::CHASE:        tick_chase();        break;
        case Effect::RAINBOW:      tick_rainbow();      break;
        case Effect::SPARKLE:      tick_sparkle();      break;
        case Effect::COMET:        tick_comet();        break;
        case Effect::COLOR_WIPE:   tick_color_wipe();   break;
        case Effect::PING_PONG:    tick_ping_pong();    break;
        case Effect::PULSE:        tick_pulse();        break;
        case Effect::WAVE:         tick_wave();         break;
        case Effect::STROBE:       tick_strobe();       break;
        case Effect::THEATER:      tick_theater();      break;
        case Effect::KALEIDOSCOPE:   tick_kaleidoscope();    break;
        case Effect::METEOR_SHOWER:   tick_meteor_shower();   break;
        default: break;
    }
}

// ─── Effect ticks ─────────────────────────────────────────────────────────────

// Shooting Star: warm gold/white head crosses the strip with a 4-pixel fading tail.
void LedPatterns::tick_chase() {
    if (!elapsed(60)) return;
    leds.clear();
    const uint8_t TR[4] = {255, 160,  80, 30};
    const uint8_t TG[4] = {245, 150,  75, 28};
    const uint8_t TB[4] = {180, 100,  45, 15};
    for (int t = 0; t < 4; t++) {
        int pos = _a - t;
        if (pos >= 0 && pos < LED_COUNT)
            leds.setPixel((uint16_t)pos, TR[t], TG[t], TB[t]);
    }
    leds.show();
    if (++_a >= LED_COUNT + 4) _a = 0;
}

// Aurora: slowly drifting hues through the green-teal-blue-violet band of the northern lights.
void LedPatterns::tick_rainbow() {
    if (!elapsed(20)) return;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        // Hue band 14000-46000: green → teal → blue → violet (aurora range)
        uint16_t hue = (uint16_t)(14000 + (uint32_t)(_a * 200 + i * 5000) % 32000);
        uint32_t c = leds.raw().ColorHSV(hue, 230, 200);
        leds.raw().setPixelColor(i, c);
    }
    leds.show();
    _a = (_a + 1) % 160;
}

// Stardust: nebula sparks igniting in deep-space emission colors — crimson, cyan, violet, amber, and electric blue.
void LedPatterns::tick_sparkle() {
    static const uint32_t STAR_COLORS[5] = {
        0xFF1466, // crimson rose (Carina Nebula)
        0x00E5FF, // bright cyan (ionised hydrogen)
        0xAA00FF, // electric violet (emission nebula)
        0xFF8C00, // deep amber (stellar nursery)
        0x00AAFF  // sky blue (Pleiades haze)
    };
    if (_b == 0) {
        if (!elapsed(30)) return;
        _a = random(LED_COUNT);
        uint32_t c = STAR_COLORS[random(5)];
        leds.setPixel((uint16_t)_a, hi(c), mi(c), lo(c));
        leds.show();
        _b = 1 + random(2); // stay lit 1-2 ticks
    } else {
        if (!elapsed(30)) return;
        _b--;
        if (_b == 0) {
            leds.setPixel((uint16_t)_a, 0, 0, 0);
            leds.show();
        }
    }
}

// Comet Tail: icy blue-white comet crossing the strip with a long cooling trail.
void LedPatterns::tick_comet() {
    if (!elapsed(55)) return;
    leds.clear();
    const uint8_t TR[5] = {220, 140,  70, 25,  8};
    const uint8_t TG[5] = {240, 170,  95, 40, 15};
    const uint8_t TB[5] = {255, 220, 160, 90, 35};
    for (int t = 0; t < 5; t++) {
        int pos = _a - t;
        if (pos >= 0 && pos < LED_COUNT)
            leds.setPixel((uint16_t)pos, TR[t], TG[t], TB[t]);
    }
    leds.show();
    if (++_a >= LED_COUNT + 5) _a = 0;
}

// Twilight Sweep: wipes through deep purple, rose, indigo, and midnight blue.
void LedPatterns::tick_color_wipe() {
    // _b = colour index (0-3), _a = LED position or pause phase
    static const uint32_t TWILIGHT[4] = {
        0x500078, // deep purple
        0xB42850, // rose
        0x281488, // indigo
        0x141E78  // midnight blue
    };
    if (_a < LED_COUNT) {
        if (!elapsed(55)) return;
        uint32_t c = TWILIGHT[_b];
        leds.setPixel((uint16_t)_a, hi(c), mi(c), lo(c));
        leds.show();
        _a++;
    } else if (_a == LED_COUNT) {
        if (!elapsed(200)) return;
        leds.clear();
        leds.show();
        _a++;
    } else {
        if (!elapsed(80)) return;
        _b = (_b + 1) % 4;
        _a = 0;
    }
}

// Firefly: warm yellow-green glow drifting back and forth with soft neighbor halo.
void LedPatterns::tick_ping_pong() {
    if (!elapsed(45)) return;
    leds.clear();
    if (_a > 0)             leds.setPixel((uint16_t)(_a - 1), 25, 45,  5);
    leds.setPixel((uint16_t)_a,            180, 220, 40);
    if (_a < LED_COUNT - 1) leds.setPixel((uint16_t)(_a + 1), 25, 45,  5);
    leds.show();
    if (_b == 0) {          // forward
        if (++_a >= LED_COUNT) { _a = LED_COUNT - 2; _b = 1; }
    } else {                // backward
        if (--_a < 0)          { _a = 1;             _b = 0; }
    }
}

// Heartbeat: warm rose double-beat pulse (lub-dub) with a long restful pause.
void LedPatterns::tick_pulse() {
    if (!elapsed(22)) return;
    uint8_t brightness = 0;
    if (_a < 8) {
        // First beat: rise to 150 and fall
        brightness = (_a < 4) ? (uint8_t)(_a * 38) : (uint8_t)((8 - _a) * 38);
    } else if (_a < 16) {
        // Second beat: slightly stronger, rise to 180 and fall
        int sub = _a - 8;
        brightness = (sub < 4) ? (uint8_t)(sub * 45) : (uint8_t)((8 - sub) * 45);
    }
    // _a 16-31: dark rest before next beat
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        leds.setPixel(i, brightness, brightness / 3, brightness / 4);
    }
    leds.show();
    _a = (_a + 1) % 32;
}

// Solar Flare: a surging wave of deep orange and crimson rolling like a coronal mass ejection across the strip.
void LedPatterns::tick_wave() {
    if (!elapsed(45)) return;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        float phase = (float)_a + i * (6.28318f / (float)LED_COUNT);
        uint8_t v = (uint8_t)(128.0f + 120.0f * sinf(phase));
        // Solar flare tint: deep orange-red dominant, trace green, near-zero blue
        leds.setPixel(i, v, (uint8_t)(v * 0.22f), (uint8_t)(v * 0.04f));
    }
    leds.show();
    _a = (_a + 1) % 63;
}

// Plasma Arc: crackling violet and electric blue bursts like force lightning or sci-fi plasma weapons.
void LedPatterns::tick_strobe() {
    if (_b == 0) {
        if (!elapsed(60)) return;
        // Deep violet or electric blue plasma burst
        uint8_t r = (random(2) == 0) ? 160 : 20;
        for (uint16_t i = 0; i < LED_COUNT; i++) {
            leds.setPixel(i, r, 0, 255);
        }
        leds.show();
        _b = random(2, 5); // dark for 2-4 ticks
    } else {
        if (!elapsed(30)) return;
        leds.clear();
        leds.show();
        _b--;
    }
}

// Milky Way: three dim star-clusters drift slowly across the strip in warm starlight.
void LedPatterns::tick_theater() {
    if (!elapsed(70)) return;
    leds.clear();
    for (int s = 0; s < 3; s++) {
        int pos = (_a + s * 3) % LED_COUNT;
        uint8_t w = (s == 0) ? 200 : (s == 1 ? 100 : 50);
        // Warm star-white: slightly golden
        leds.setPixel((uint16_t)pos, w, (uint8_t)(w * 0.95f), (uint8_t)(w * 0.75f));
        // Soft halo on neighbor
        int prev = (pos - 1 + LED_COUNT) % LED_COUNT;
        uint8_t h = w / 5;
        leds.setPixel((uint16_t)prev, h, (uint8_t)(h * 0.95f), (uint8_t)(h * 0.75f));
    }
    leds.show();
    _a = (_a + 1) % LED_COUNT;
}

// Meteor Shower: three ion-drive streaks in cyan and electric violet, like spacecraft exhaust trails in deep space.
void LedPatterns::tick_meteor_shower() {
    if (!elapsed(38)) return;
    leds.clear();
    const int travel = LED_COUNT + 3; // head travels this far before reset
    int heads[3];
    heads[0] = _a % travel;
    heads[1] = (_a + travel / 3)     % travel;
    heads[2] = (_a + 2 * travel / 3) % travel;
    for (int m = 0; m < 3; m++) {
        const uint8_t HR[3] = {  0,  40,  30}; // near-zero red
        const uint8_t HG[3] = {220,  90,  20}; // cyan-green fading to dark
        const uint8_t HB[3] = {255, 200,  70}; // bright blue fading to deep violet
        for (int t = 0; t < 3; t++) {
            int pos = heads[m] - t;
            if (pos >= 0 && pos < LED_COUNT)
                leds.setPixel((uint16_t)pos, HR[t], HG[t], HB[t]);
        }
    }
    leds.show();
    _a = (_a + 1) % travel;
}

// Nebula: slowly morphing deep-space colors drifting through violet, rose, and deep blue.
void LedPatterns::tick_kaleidoscope() {
    if (!elapsed(80)) return;
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        // Hue band 40000-64000: violet → magenta → pink → near-red (nebula range)
        uint16_t hue = (uint16_t)(40000 + (uint32_t)(_a * 180 + i * 2400) % 24000);
        uint32_t c = leds.raw().ColorHSV(hue, 255, 180);
        leds.raw().setPixelColor(i, c);
    }
    leds.show();
    _a = (_a + 1) % 180;
}

// ─── Name helpers ─────────────────────────────────────────────────────────────

const char* effectName(Effect e) {
    switch (e) {
        case Effect::CHASE:        return "chase";
        case Effect::RAINBOW:      return "rainbow";
        case Effect::SPARKLE:      return "sparkle";
        case Effect::COMET:        return "comet";
        case Effect::COLOR_WIPE:   return "colorwipe";
        case Effect::PING_PONG:    return "pingpong";
        case Effect::PULSE:        return "pulse";
        case Effect::WAVE:         return "wave";
        case Effect::STROBE:       return "strobe";
        case Effect::THEATER:      return "theater";
        case Effect::KALEIDOSCOPE:  return "kaleidoscope";
        case Effect::METEOR_SHOWER:  return "meteorshower";
        default:                     return "none";
    }
}

Effect effectFromStr(const char* name) {
    if (strcmp(name, "chase")        == 0) return Effect::CHASE;
    if (strcmp(name, "rainbow")      == 0) return Effect::RAINBOW;
    if (strcmp(name, "sparkle")      == 0) return Effect::SPARKLE;
    if (strcmp(name, "comet")        == 0) return Effect::COMET;
    if (strcmp(name, "colorwipe")    == 0) return Effect::COLOR_WIPE;
    if (strcmp(name, "pingpong")     == 0) return Effect::PING_PONG;
    if (strcmp(name, "pulse")        == 0) return Effect::PULSE;
    if (strcmp(name, "wave")         == 0) return Effect::WAVE;
    if (strcmp(name, "strobe")       == 0) return Effect::STROBE;
    if (strcmp(name, "theater")      == 0) return Effect::THEATER;
    if (strcmp(name, "kaleidoscope")  == 0) return Effect::KALEIDOSCOPE;
    if (strcmp(name, "meteorshower")   == 0) return Effect::METEOR_SHOWER;
    return Effect::NONE;
}
