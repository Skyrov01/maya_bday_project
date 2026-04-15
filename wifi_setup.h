#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

// Initialise WiFi (STA or AP, depending on config.h).
// Prints IP address and mDNS URL to Serial.
void wifi_setup_init();

// Call every loop() to keep mDNS responsive.
void wifi_setup_update();
