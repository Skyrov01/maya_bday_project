#pragma once

#include <Arduino.h>
#include <ESP8266WebServer.h>

// Initialise and start the HTTP server on port 80.
void http_api_init();

// Call every loop() to process incoming HTTP requests.
void http_api_update();
