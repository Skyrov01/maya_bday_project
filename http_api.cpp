#include "http_api.h"
#include "config.h"
#include "led_controller.h"
#include "led_patterns.h"
#include "pattern_player.h"
#include "storage.h"
#include <ArduinoJson.h>

static ESP8266WebServer server(80);

// ─── CORS / response helpers ──────────────────────────────────────────────────

static void addCors() {
    server.sendHeader("Access-Control-Allow-Origin",  "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.sendHeader("Access-Control-Max-Age",       "86400");
}

static void sendJson(int code, const String& body) {
    addCors();
    server.send(code, "application/json", body);
}

static void sendOk() {
    sendJson(200, "{\"ok\":true}");
}

static void sendError(int code, const char* msg) {
    String body = "{\"error\":\"";
    body += msg;
    body += "\"}";
    sendJson(code, body);
}

// Respond to every preflight OPTIONS request.
static void handleOptions() {
    addCors();
    server.send(204);
}

// Stop effects and pattern player (used when static LED commands arrive).
static void stopAll() {
    patterns.stop();
    player.stop();
}

// ─── GET /api/status ─────────────────────────────────────────────────────────

static void handleGetStatus() {
    DynamicJsonDocument doc(256);
    doc["name"]       = MDNS_HOSTNAME;
    doc["led_count"]  = LED_COUNT;
    doc["effect"]     = effectName(patterns.currentEffect());
    doc["playing"]    = player.isPlaying();
    doc["brightness"] = leds.getBrightness();

    String out;
    serializeJson(doc, out);
    sendJson(200, out);
}

// ─── GET /api/leds ───────────────────────────────────────────────────────────

static void handleGetLeds() {
    DynamicJsonDocument doc(1024);
    JsonArray arr = doc.createNestedArray("leds");
    for (uint16_t i = 0; i < LED_COUNT; i++) {
        uint32_t c = leds.getPixelColor(i);
        JsonObject o = arr.createNestedObject();
        o["r"] = (c >> 16) & 0xFF;
        o["g"] = (c >>  8) & 0xFF;
        o["b"] =  c        & 0xFF;
    }
    doc["brightness"] = leds.getBrightness();

    String out;
    serializeJson(doc, out);
    sendJson(200, out);
}

// ─── POST /api/leds ──────────────────────────────────────────────────────────
// Body: { "leds": [{"r":0,"g":0,"b":0}, ...], "brightness": 128 }

static void handlePostLeds() {
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    stopAll();

    if (doc.containsKey("brightness")) {
        leds.setBrightness((uint8_t)(int)doc["brightness"]);
    }

    JsonArray arr = doc["leds"].as<JsonArray>();
    if (!arr.isNull()) {
        uint8_t i = 0;
        for (JsonObject o : arr) {
            if (i >= LED_COUNT) break;
            leds.setPixel(i, o["r"] | 0, o["g"] | 0, o["b"] | 0);
            i++;
        }
    }
    leds.show();
    sendOk();
}

// ─── POST /api/led ────────────────────────────────────────────────────────────
// Body: { "index": 3, "r": 255, "g": 0, "b": 0 }

static void handlePostLed() {
    DynamicJsonDocument doc(128);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    int index = doc["index"] | -1;
    if (index < 0 || index >= LED_COUNT) { sendError(400, "index out of range"); return; }

    stopAll();
    leds.setPixel((uint16_t)index, doc["r"] | 0, doc["g"] | 0, doc["b"] | 0);
    leds.show();
    sendOk();
}

// ─── GET /api/brightness ─────────────────────────────────────────────────────

static void handleGetBrightness() {
    String body = "{\"value\":";
    body += leds.getBrightness();
    body += "}";
    sendJson(200, body);
}

// ─── POST /api/brightness ────────────────────────────────────────────────────
// Body: { "value": 128 }

static void handlePostBrightness() {
    DynamicJsonDocument doc(64);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    int val = doc["value"] | -1;
    if (val < 0 || val > 255) { sendError(400, "value must be 0-255"); return; }

    leds.setBrightness((uint8_t)val);
    leds.show();
    sendOk();
}

// ─── POST /api/effect ────────────────────────────────────────────────────────
// Body: { "name": "rainbow" }  — use "stop" to halt.

static void handlePostEffect() {
    DynamicJsonDocument doc(128);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    const char* name = doc["name"] | "stop";

    player.stop();
    if (strcmp(name, "stop") == 0) {
        patterns.stop();
    } else {
        Effect e = effectFromStr(name);
        if (e == Effect::NONE) { sendError(400, "Unknown effect"); return; }
        patterns.startEffect(e);
    }
    sendOk();
}

// ─── GET /api/patterns ───────────────────────────────────────────────────────

static void handleGetPatterns() {
    PatternInfo infos[STORAGE_MAX_PATTERNS];
    int count = storage.listPatterns(infos, STORAGE_MAX_PATTERNS);

    DynamicJsonDocument doc(2048);
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < count; i++) {
        JsonObject o = arr.createNestedObject();
        o["id"]   = infos[i].id;
        o["name"] = infos[i].name;
    }

    String out;
    serializeJson(doc, out);
    sendJson(200, out);
}

// ─── POST /api/patterns/save ─────────────────────────────────────────────────
// Body: { "name": "...", "frames": [{ "leds": [{r,g,b},...], "duration_ms": 500 }, ...] }

static void handleSavePattern() {
    DynamicJsonDocument doc(8192);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    const char* name = doc["name"] | "Unnamed";
    JsonArray jframes = doc["frames"].as<JsonArray>();
    if (jframes.isNull()) { sendError(400, "frames required"); return; }

    PatternFrame frames[PATTERN_MAX_FRAMES];
    uint8_t count = 0;

    for (JsonObject jf : jframes) {
        if (count >= PATTERN_MAX_FRAMES) break;
        PatternFrame& pf = frames[count];
        pf.duration_ms = jf["duration_ms"] | 500;

        JsonArray jleds = jf["leds"].as<JsonArray>();
        uint8_t li = 0;
        for (JsonObject jled : jleds) {
            if (li >= LED_COUNT) break;
            pf.r[li] = jled["r"] | 0;
            pf.g[li] = jled["g"] | 0;
            pf.b[li] = jled["b"] | 0;
            li++;
        }
        while (li < LED_COUNT) { pf.r[li] = pf.g[li] = pf.b[li] = 0; li++; }
        count++;
    }

    char newId[12];
    if (!storage.savePattern(name, frames, count, newId, sizeof(newId))) {
        sendError(500, "Save failed");
        return;
    }

    String resp = "{\"id\":\"";
    resp += newId;
    resp += "\"}";
    sendJson(200, resp);
}

// ─── POST /api/patterns/play ─────────────────────────────────────────────────
// Body: { "id": "3" }

static void handlePlayPattern() {
    DynamicJsonDocument doc(64);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    const char* id = doc["id"] | "";
    if (strlen(id) == 0) { sendError(400, "id required"); return; }

    patterns.stop();
    player.stop();
    player.setFrameCount(0);

    if (!storage.loadPattern(id, player)) {
        sendError(404, "Pattern not found");
        return;
    }
    player.play(true);
    sendOk();
}

// ─── POST /api/patterns/delete ───────────────────────────────────────────────
// Body: { "id": "3" }

static void handleDeletePattern() {
    DynamicJsonDocument doc(64);
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { sendError(400, "Invalid JSON"); return; }

    const char* id = doc["id"] | "";
    if (strlen(id) == 0) { sendError(400, "id required"); return; }

    if (!storage.deletePattern(id)) { sendError(404, "Pattern not found"); return; }
    sendOk();
}

// ─── 404 fallback + OPTIONS catch-all ────────────────────────────────────────

static void handleNotFound() {
    if (server.method() == HTTP_OPTIONS) {
        handleOptions();
        return;
    }
    sendError(404, "Not found");
}

// ─── Init / update ────────────────────────────────────────────────────────────

void http_api_init() {
    // OPTIONS preflight routes (registered explicitly for known paths).
    server.on("/api/status",          HTTP_OPTIONS, handleOptions);
    server.on("/api/leds",            HTTP_OPTIONS, handleOptions);
    server.on("/api/led",             HTTP_OPTIONS, handleOptions);
    server.on("/api/brightness",      HTTP_OPTIONS, handleOptions);
    server.on("/api/effect",          HTTP_OPTIONS, handleOptions);
    server.on("/api/patterns",        HTTP_OPTIONS, handleOptions);
    server.on("/api/patterns/save",   HTTP_OPTIONS, handleOptions);
    server.on("/api/patterns/play",   HTTP_OPTIONS, handleOptions);
    server.on("/api/patterns/delete", HTTP_OPTIONS, handleOptions);

    // Data routes.
    server.on("/api/status",          HTTP_GET,  handleGetStatus);
    server.on("/api/leds",            HTTP_GET,  handleGetLeds);
    server.on("/api/leds",            HTTP_POST, handlePostLeds);
    server.on("/api/led",             HTTP_POST, handlePostLed);
    server.on("/api/brightness",      HTTP_GET,  handleGetBrightness);
    server.on("/api/brightness",      HTTP_POST, handlePostBrightness);
    server.on("/api/effect",          HTTP_POST, handlePostEffect);
    server.on("/api/patterns",        HTTP_GET,  handleGetPatterns);
    server.on("/api/patterns/save",   HTTP_POST, handleSavePattern);
    server.on("/api/patterns/play",   HTTP_POST, handlePlayPattern);
    server.on("/api/patterns/delete", HTTP_POST, handleDeletePattern);

    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println(F("[HTTP] Server started on port 80."));
}

void http_api_update() {
    server.handleClient();
}
