#include "wifi_setup.h"
#include "config.h"

static void start_ap() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.println(F("[WiFi] AP mode  SSID: " AP_SSID "  pass: " AP_PASS));
    Serial.print(F("[WiFi] AP IP: "));
    Serial.println(WiFi.softAPIP());
}

void wifi_setup_init() {
    Serial.println(F("[WiFi] Initialising..."));

#if AP_MODE
    start_ap();
#else
    WiFi.persistent(false);   // never write credentials to flash
    WiFi.disconnect(true);    // clear any cached SSID/pass + turn radio off
    delay(200);               // let the radio fully reset
    WiFi.mode(WIFI_STA);
    delay(100);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(F("[WiFi] Connecting to " WIFI_SSID));

    bool connected = false;
    unsigned long start = millis();
    while (millis() - start < WIFI_TIMEOUT_MS) {
        if (WiFi.status() == WL_CONNECTED) { connected = true; break; }
        delay(500);
        Serial.print('.');
    }
    Serial.println();
    Serial.print(F("[WiFi] status code: "));
    Serial.println(WiFi.status());
    if (connected) {
        Serial.print(F("[WiFi] Connected. IP: "));
        Serial.println(WiFi.localIP());
        
    } else {
        Serial.println(F("[WiFi] Timeout — falling back to AP mode."));
        start_ap();
    }
#endif

    if (MDNS.begin(MDNS_HOSTNAME)) {
        MDNS.addService("http", "tcp", 80);
        Serial.println(F("[WiFi] mDNS: http://" MDNS_HOSTNAME ".local"));
    }
}

void wifi_setup_update() {
    MDNS.update();
}
