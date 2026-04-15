#pragma once

#include <Arduino.h>
#include "config.h"
#include "pattern_player.h"

// Identifies a saved pattern (id = filename stem, e.g. "3").
struct PatternInfo {
    char id  [12];
    char name[STORAGE_MAX_NAME_LEN + 1];
};

// LittleFS-backed pattern storage.  Patterns are stored as JSON files under
// /patterns/<id>.json.  A small counter file tracks the next id to assign.
class Storage {
public:
    // Mount LittleFS. Returns false if mounting fails.
    bool begin();

    // Fill 'out' with up to 'maxCount' saved patterns. Returns actual count.
    int listPatterns(PatternInfo* out, int maxCount);

    // Load the pattern identified by 'id' into 'player'. Returns false on error.
    bool loadPattern(const char* id, PatternPlayer& dst);

    // Save frames to a new file with the given name.
    // Writes the assigned id into outId (must be at least 12 chars).
    bool savePattern(const char* name,
                     const PatternFrame* frames, uint8_t frameCount,
                     char* outId, size_t outIdLen);

    // Delete the pattern file for 'id'. Returns false if not found.
    bool deletePattern(const char* id);
};

extern Storage storage;
