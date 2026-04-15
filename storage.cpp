#include "storage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

Storage storage;

static const char* PATTERNS_DIR  = "/patterns";
static const char* COUNTER_PATH  = "/patterns/.counter";

// ─── Helpers ──────────────────────────────────────────────────────────────────

static int readCounter() {
    File f = LittleFS.open(COUNTER_PATH, "r");
    if (!f) return 1;
    int v = f.readString().toInt();
    f.close();
    return (v < 1) ? 1 : v;
}

static void writeCounter(int value) {
    File f = LittleFS.open(COUNTER_PATH, "w");
    if (f) { f.print(value); f.close(); }
}

static String patternPath(const char* id) {
    return String(PATTERNS_DIR) + "/" + id + ".json";
}

// ─── Public API ───────────────────────────────────────────────────────────────

bool Storage::begin() {
    if (!LittleFS.begin()) {
        Serial.println(F("[Storage] LittleFS mount failed — formatting..."));
        LittleFS.format();
        if (!LittleFS.begin()) {
            Serial.println(F("[Storage] Format failed."));
            return false;
        }
    }
    if (!LittleFS.exists(PATTERNS_DIR)) {
        LittleFS.mkdir(PATTERNS_DIR);
    }
    Serial.println(F("[Storage] Ready."));
    return true;
}

int Storage::listPatterns(PatternInfo* out, int maxCount) {
    Dir dir = LittleFS.openDir(PATTERNS_DIR);
    int count = 0;
    while (dir.next() && count < maxCount) {
        String fname = dir.fileName(); // e.g. "3.json"
        if (fname.startsWith(".")) continue;
        if (!fname.endsWith(".json")) continue;

        // Read name field from file without loading the full frame data.
        String fullPath = String(PATTERNS_DIR) + "/" + fname;
        File f = LittleFS.open(fullPath, "r");
        if (!f) continue;

        DynamicJsonDocument doc(256);
        DeserializationError err = deserializeJson(doc, f);
        f.close();
        if (err) continue;

        // id is the stem of the filename (strip ".json")
        String stem = fname.substring(0, fname.length() - 5);
        stem.toCharArray(out[count].id, sizeof(out[count].id));

        const char* name = doc["name"] | stem.c_str();
        strncpy(out[count].name, name, STORAGE_MAX_NAME_LEN);
        out[count].name[STORAGE_MAX_NAME_LEN] = '\0';

        count++;
    }
    return count;
}

bool Storage::loadPattern(const char* id, PatternPlayer& dst) {
    File f = LittleFS.open(patternPath(id), "r");
    if (!f) return false;

    // Use a larger doc for frame data.
    DynamicJsonDocument doc(8192);
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) return false;

    JsonArray frames = doc["frames"].as<JsonArray>();
    if (frames.isNull()) return false;

    uint8_t count = 0;
    for (JsonObject jf : frames) {
        if (count >= PATTERN_MAX_FRAMES) break;
        PatternFrame pf;
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
        // Zero-fill any missing LEDs.
        while (li < LED_COUNT) { pf.r[li] = pf.g[li] = pf.b[li] = 0; li++; }

        dst.setFrame(count++, pf);
    }
    dst.setFrameCount(count);
    return count > 0;
}

bool Storage::savePattern(const char* name,
                          const PatternFrame* frames, uint8_t frameCount,
                          char* outId, size_t outIdLen) {
    int id = readCounter();
    writeCounter(id + 1);

    DynamicJsonDocument doc(8192);
    doc["name"] = name;

    JsonArray jframes = doc.createNestedArray("frames");
    for (uint8_t fi = 0; fi < frameCount && fi < PATTERN_MAX_FRAMES; fi++) {
        JsonObject jf = jframes.createNestedObject();
        jf["duration_ms"] = frames[fi].duration_ms;
        JsonArray jleds = jf.createNestedArray("leds");
        for (uint8_t li = 0; li < LED_COUNT; li++) {
            JsonObject jled = jleds.createNestedObject();
            jled["r"] = frames[fi].r[li];
            jled["g"] = frames[fi].g[li];
            jled["b"] = frames[fi].b[li];
        }
    }

    snprintf(outId, outIdLen, "%d", id);
    File f = LittleFS.open(patternPath(outId), "w");
    if (!f) return false;
    serializeJson(doc, f);
    f.close();
    return true;
}

bool Storage::deletePattern(const char* id) {
    String path = patternPath(id);
    if (!LittleFS.exists(path)) return false;
    return LittleFS.remove(path);
}
