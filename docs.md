# Maya Birthday Project — Documentation

## System Overview

The project runs on a **Wemos D1 Mini** (ESP8266) and controls a strip of **10 NeoPixel LEDs** over WiFi via a REST HTTP API on port 80.

### Architecture

```
┌──────────────────────────────────────────────────────────┐
│  Wemos D1 Mini                                           │
│                                                          │
│  WiFi (STA or AP)                                        │
│       │                                                  │
│  HTTP Server (port 80)  ←──── REST calls from browser   │
│       │                                                  │
│  ┌────┴─────────────────────────────────┐                │
│  │  LedController  (raw pixel writes)   │                │
│  │  LedPatterns    (built-in effects)   │──► NeoPixels   │
│  │  PatternPlayer  (saved frame loops)  │                │
│  │  Storage        (LittleFS patterns)  │                │
│  └──────────────────────────────────────┘                │
└──────────────────────────────────────────────────────────┘
```

| Module | Responsibility |
|---|---|
| `wifi_setup` | Connects to a hotspot (STA) or creates its own network (AP) |
| `led_controller` | Thin wrapper around Adafruit NeoPixel |
| `led_patterns` | Runs non-blocking built-in animations (rainbow, chase, etc.) |
| `pattern_player` | Plays back saved multi-frame patterns in a loop |
| `storage` | Persists patterns as JSON files on LittleFS |
| `http_api` | Maps REST endpoints to the modules above |

### Boot sequence

1. WiFi connects (STA mode by default; falls back to AP after 25 s).
2. LittleFS mounts and loads saved patterns metadata.
3. HTTP server starts on port 80.
4. `loop()` calls `http_api_update()`, `patterns.update()`, and `player.update()` on every iteration. No `delay()` calls are used — all animation timing is millis()-based.

### Device URL

```
http://wemos-leds.local   # mDNS — works on most networks
http://<device-ip>        # fallback
```

---

## Configuration (`config.h`)

| Define | Default | Description |
|---|---|---|
| `LED_PIN` | `D2` | Data pin for the NeoPixel strip |
| `LED_COUNT` | `10` | Number of LEDs |
| `DEFAULT_BRIGHTNESS` | `32` | Startup brightness (0–255) |
| `AP_MODE` | `false` | `true` = device creates its own WiFi |
| `WIFI_SSID` / `WIFI_PASS` | — | Credentials for STA mode |
| `AP_SSID` / `AP_PASS` | `WemosLEDs` / `ledcontrol` | Credentials for AP mode |
| `MDNS_HOSTNAME` | `wemos-leds` | mDNS hostname |
| `WIFI_TIMEOUT_MS` | `25000` | STA connection timeout before AP fallback |
| `PATTERN_MAX_FRAMES` | `16` | Max frames per saved pattern |
| `STORAGE_MAX_PATTERNS` | `32` | Max number of saved patterns |

---

## REST API Reference

All endpoints return `Content-Type: application/json` and support CORS.  
Error responses have the shape `{"error": "<message>"}`.

---

### `GET /api/status`

Returns device state.

**Response**
```json
{
  "name":       "wemos-leds",
  "led_count":  10,
  "effect":     "rainbow",
  "playing":    false,
  "brightness": 128
}
```

---

### `GET /api/leds`

Returns the current colour of every LED.

**Response**
```json
{
  "leds": [
    {"r": 255, "g": 0, "b": 0},
    {"r": 0,   "g": 0, "b": 0}
  ],
  "brightness": 128
}
```

---

### `POST /api/leds`

Sets all LEDs at once. Stops any running effect or pattern playback.

**Body** *(all fields optional)*
```json
{
  "brightness": 128,
  "leds": [
    {"r": 255, "g": 0,   "b": 0},
    {"r": 0,   "g": 255, "b": 0}
  ]
}
```

**Response** `{"ok": true}`

---

### `POST /api/led`

Sets a single LED by index. Stops any running effect or pattern playback.

**Body**
```json
{"index": 3, "r": 255, "g": 0, "b": 0}
```

`index` must be in range `0 – LED_COUNT-1`.

**Response** `{"ok": true}`

---

### `GET /api/brightness`

Returns the current brightness value.

**Response** `{"value": 128}`

---

### `POST /api/brightness`

Sets brightness without changing LED colours.

**Body**
```json
{"value": 200}
```

`value` must be `0–255`.

**Response** `{"ok": true}`

---

### `POST /api/effect`

Starts a built-in animation effect. Stops pattern playback.  
Send `"stop"` to halt all effects.

**Body**
```json
{"name": "rainbow"}
```

**Available effect names**

| Name | Description |
|---|---|
| `chase` | Single lit pixel travels along the strip |
| `rainbow` | Full-strip hue rotation |
| `sparkle` | Random pixels flash white |
| `comet` | Bright head with a fading tail |
| `color_wipe` | Strip fills with a colour one LED at a time |
| `ping_pong` | Pixel bounces back and forth |
| `pulse` | Whole strip fades in and out |
| `wave` | Sine-wave brightness across the strip |
| `strobe` | Fast on/off flash |
| `theater` | Theatre-marquee chase |
| `kaleidoscope` | Symmetrical colour pattern |
| `stop` | Halts any active effect |

**Response** `{"ok": true}`

---

### `GET /api/patterns`

Lists all saved patterns.

**Response**
```json
[
  {"id": "1", "name": "Birthday"},
  {"id": "2", "name": "Rainbow loop"}
]
```

---

### `POST /api/patterns/save`

Saves a new pattern and returns its assigned id.

**Body**
```json
{
  "name": "Birthday",
  "frames": [
    {
      "duration_ms": 500,
      "leds": [
        {"r": 255, "g": 0, "b": 0},
        {"r": 0, "g": 255, "b": 0}
      ]
    },
    {
      "duration_ms": 500,
      "leds": [
        {"r": 0, "g": 0, "b": 255},
        {"r": 255, "g": 255, "b": 0}
      ]
    }
  ]
}
```

- Max **16 frames** per pattern.
- LEDs not listed in a frame default to off `(0, 0, 0)`.
- Patterns are stored as JSON under `/patterns/<id>.json` on LittleFS.

**Response** `{"id": "3"}`

---

### `POST /api/patterns/play`

Plays a saved pattern in a continuous loop. Stops any built-in effect first.

**Body**
```json
{"id": "3"}
```

**Response** `{"ok": true}`  
Returns `404` if the id does not exist.

---

### `POST /api/patterns/delete`

Deletes a saved pattern by id.

**Body**
```json
{"id": "3"}
```

**Response** `{"ok": true}`  
Returns `404` if the id does not exist.

---

## Priority rules

Calling any static LED endpoint (`POST /api/leds`, `POST /api/led`) or starting an effect (`POST /api/effect`) will automatically stop pattern playback. Similarly, playing a pattern will stop any active built-in effect. Only one mode is active at a time.

---

## Quick-start example (curl)

```bash
BASE=http://wemos-leds.local

# Check device status
curl $BASE/api/status

# Set LED 0 to red
curl -X POST $BASE/api/led \
  -H "Content-Type: application/json" \
  -d '{"index":0,"r":255,"g":0,"b":0}'

# Start rainbow effect
curl -X POST $BASE/api/effect \
  -H "Content-Type: application/json" \
  -d '{"name":"rainbow"}'

# Stop everything
curl -X POST $BASE/api/effect \
  -H "Content-Type: application/json" \
  -d '{"name":"stop"}'

# Save a 2-frame pattern and play it
ID=$(curl -s -X POST $BASE/api/patterns/save \
  -H "Content-Type: application/json" \
  -d '{"name":"Test","frames":[{"duration_ms":300,"leds":[{"r":255,"g":0,"b":0}]},{"duration_ms":300,"leds":[{"r":0,"g":0,"b":255}]}]}' \
  | grep -oP '(?<="id":")[^"]+')

curl -X POST $BASE/api/patterns/play \
  -H "Content-Type: application/json" \
  -d "{\"id\":\"$ID\"}"
```
