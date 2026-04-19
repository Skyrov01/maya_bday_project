// api.js — HTTP client for the Wemos REST API.
// The base URL is persisted in localStorage so the user only types it once.

const STORAGE_KEY = 'wemos_base_url';

export class Api {
  constructor() {
    this._base = localStorage.getItem(STORAGE_KEY) || '';
  }

  get baseUrl() { return this._base; }

  setBaseUrl(url) {
    // Normalise: strip trailing slash, ensure http:// prefix.
    url = url.trim().replace(/\/+$/, '');
    if (url && !url.startsWith('http')) url = 'http://' + url;
    this._base = url;
    localStorage.setItem(STORAGE_KEY, url);
  }

  // ── Low-level helpers ─────────────────────────────────────────────────────

  async _get(path) {
    const res = await fetch(this._base + path);
    if (!res.ok) throw new Error(`GET ${path} → ${res.status}`);
    return res.json();
  }

  async _post(path, body) {
    const res = await fetch(this._base + path, {
      method:  'POST',
      headers: { 'Content-Type': 'application/json' },
      body:    JSON.stringify(body),
    });
    if (!res.ok) {
      const txt = await res.text().catch(() => '');
      throw new Error(`POST ${path} → ${res.status}: ${txt}`);
    }
    return res.json();
  }

  // ── Status ────────────────────────────────────────────────────────────────

  /** Returns { name, led_count, effect, playing, brightness } */
  status() { return this._get('/api/status'); }

  // ── LEDs ──────────────────────────────────────────────────────────────────

  /** Returns { leds: [{r,g,b},...], brightness } */
  getLeds() { return this._get('/api/leds'); }

  /** @param {Array<{r:number,g:number,b:number}>} leds  @param {number} [brightness] */
  setLeds(leds, brightness) {
    const body = { leds };
    if (brightness !== undefined) body.brightness = brightness;
    return this._post('/api/leds', body);
  }

  /** @param {number} index  @param {number} r  @param {number} g  @param {number} b */
  setLed(index, r, g, b) {
    return this._post('/api/led', { index, r, g, b });
  }

  // ── Brightness ────────────────────────────────────────────────────────────

  getBrightness() { return this._get('/api/brightness'); }
  setBrightness(value) { return this._post('/api/brightness', { value }); }

  // ── Effects ───────────────────────────────────────────────────────────────

  /** @param {'chase'|'rainbow'|'sparkle'|'comet'|'colorwipe'|'pingpong'|'stop'} name */
  runEffect(name)  { return this._post('/api/effect',  { name }); }
  stopEffect()     { return this.runEffect('stop'); }

  // ── Saved patterns ────────────────────────────────────────────────────────

  /** Returns [{id, name}] */
  listPatterns() { return this._get('/api/patterns'); }

  /** Returns { id, name, frames } for a single pattern. */
  getPattern(id) { return this._get(`/api/patterns/get?id=${encodeURIComponent(id)}`); }

  /**
   * @param {string} name
   * @param {Array<{leds:Array<{r,g,b}>, duration_ms:number}>} frames
   */
  savePattern(name, frames) {
    return this._post('/api/patterns/save', { name, frames });
  }

  /** @param {string} id */
  playPattern(id)   { return this._post('/api/patterns/play',   { id }); }
  deletePattern(id) { return this._post('/api/patterns/delete', { id }); }
}

export const api = new Api();
