#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

// Thin wrapper around Adafruit_NeoPixel.
// All set* calls are buffered; call show() to push to the strip.
class LedController {
public:
    LedController();

    // Must be called once in setup().
    void begin();

    // Set a single pixel (0-based). No-op if index >= LED_COUNT.
    void setPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);

    // Fill every pixel with the same colour.
    void fill(uint8_t r, uint8_t g, uint8_t b);

    // Turn off all pixels.
    void clear();

    // Push buffered colours to the physical strip.
    void show();

    // Global brightness 0-255. Affects all pixels on next show().
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;

    // Returns packed 0x00RRGGBB colour for pixel at index.
    uint32_t getPixelColor(uint16_t index) const;

    uint16_t numPixels() const { return LED_COUNT; }

    // Direct access needed by the pattern engine.
    Adafruit_NeoPixel& raw() { return _strip; }

private:
    Adafruit_NeoPixel _strip;
    uint8_t           _brightness;
};

extern LedController leds;
