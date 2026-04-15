#include "led_controller.h"

LedController leds;

LedController::LedController()
    : _strip(LED_COUNT, LED_PIN, LED_TYPE),
      _brightness(DEFAULT_BRIGHTNESS) {}

void LedController::begin() {
    _strip.begin();
    _strip.setBrightness(_brightness);
    _strip.clear();
    _strip.show();
}

void LedController::setPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < LED_COUNT) {
        _strip.setPixelColor(index, r, g, b);
    }
}

void LedController::fill(uint8_t r, uint8_t g, uint8_t b) {
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        _strip.setPixelColor(i, r, g, b);
    }
}

void LedController::clear() {
    _strip.clear();
}

void LedController::show() {
    _strip.show();
}

void LedController::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    _strip.setBrightness(brightness);
}

uint8_t LedController::getBrightness() const {
    return _brightness;
}

uint32_t LedController::getPixelColor(uint16_t index) const {
    return _strip.getPixelColor(index);
}
