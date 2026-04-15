#pragma once

// ─── LED Strip ────────────────────────────────────────────────────────────────
#define LED_PIN              D2
#define LED_COUNT            10
#define LED_TYPE             (NEO_GRB + NEO_KHZ800)
#define DEFAULT_BRIGHTNESS   32

// ─── WiFi ─────────────────────────────────────────────────────────────────────
// Set AP_MODE to false  → Wemos joins your hotspot (recommended for phone use).
// Set AP_MODE to true   → Wemos creates its own "WemosLEDs" WiFi network.
#define AP_MODE  false

// Station mode: credentials for the network the Wemos should join.
#define WIFI_SSID  "char"
#define WIFI_PASS  "qhskgz8byrsun"

// AP mode: credentials for the network the Wemos will create.
#define AP_SSID  "WemosLEDs"
#define AP_PASS  "ledcontrol"

// mDNS hostname → accessible as http://wemos-leds.local once connected.
#define MDNS_HOSTNAME  "wemos-leds"

// Time (ms) to wait for STA connection before falling back to AP mode.
#define WIFI_TIMEOUT_MS  25000

// ─── Pattern storage ──────────────────────────────────────────────────────────
#define PATTERN_MAX_FRAMES   16
#define STORAGE_MAX_PATTERNS 32
#define STORAGE_MAX_NAME_LEN 32
