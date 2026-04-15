// maya_bday_project.ino
// Wemos D1 Mini — NeoPixel LED controller with WiFi REST API.
//
// Dependencies (install via Arduino Library Manager):
//   • Adafruit NeoPixel
//   • ArduinoJson  (v6.x)
//   • ESP8266 board package (includes ESP8266WiFi, ESP8266WebServer,
//     ESP8266mDNS, LittleFS)
//
// Before flashing, edit config.h:
//   • Set WIFI_SSID / WIFI_PASS to your hotspot credentials.
//   • Or set AP_MODE to true so the Wemos creates its own WiFi.

#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "config.h"
#include "wifi_setup.h"
#include "led_controller.h"
#include "led_patterns.h"
#include "pattern_player.h"
#include "storage.h"
#include "http_api.h"

void setup() {
    Serial.begin(115200);
    Serial.println(F("\n[Boot] Starting Wemos LED Controller..."));
    
    
    Serial.println(F("[DEBUG] Starting WiFi scan..."));
    int n = WiFi.scanNetworks();
    Serial.print(F("[DEBUG] Found networks: "));
    Serial.println(n);
    for (int i = 0; i < n; i++) {
        Serial.print(F("  "));
        Serial.println(WiFi.SSID(i));
    }
    
    leds.begin();
    wifi_setup_init();
    storage.begin();
    http_api_init();

    Serial.println(F("[Boot] Ready. Open the web app and enter this device's IP."));
}

void loop() {
    wifi_setup_update();
    http_api_update();
    patterns.update();
    player.update();
}
