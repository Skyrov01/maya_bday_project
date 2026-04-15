// pattern-editor.js — keyframe timeline: add/remove frames, set per-LED colours
//                     and duration, then save to Wemos or play directly.

import { api } from './api.js';
import { LedGrid, hexToRgb, rgbToHex } from './led-grid.js';

const LED_COUNT = 10;
const DEFAULT_DURATION = 500;

export class PatternEditor {
  /** @param {HTMLElement} container */
  constructor(container) {
    this._container   = container;
    this._frames      = [];          // Array<{leds:[{r,g,b}], duration_ms}>
    this._activeFrame = -1;
    this._editorGrid  = null;
    this._build();
  }

  _build() {
    this._container.innerHTML = `
      <div class="pattern-editor__toolbar">
        <button id="pe-add-frame"  class="btn btn--sm">+ Add Frame</button>
        <button id="pe-clone"      class="btn btn--sm btn--ghost">Clone Frame</button>
        <button id="pe-play-live"  class="btn btn--sm">▶ Play Preview</button>
        <button id="pe-stop-live"  class="btn btn--sm btn--ghost">⏹ Stop</button>
      </div>
      <div id="pe-frame-list" class="frame-list"></div>
      <div id="pe-frame-editor" class="frame-editor" style="display:none"></div>
    `;

    this._container.querySelector('#pe-add-frame').addEventListener('click', () => this._addFrame());
    this._container.querySelector('#pe-clone').addEventListener('click',     () => this._cloneFrame());
    this._container.querySelector('#pe-play-live').addEventListener('click', () => this._playLive());
    this._container.querySelector('#pe-stop-live').addEventListener('click', async () => {
      try { await api.stopEffect(); } catch (e) { console.error(e); }
    });

    this._frameListEl  = this._container.querySelector('#pe-frame-list');
    this._frameEditorEl = this._container.querySelector('#pe-frame-editor');
  }

  // ── Frame management ────────────────────────────────────────────────────────

  _blankLeds() {
    return Array.from({ length: LED_COUNT }, () => ({ r: 0, g: 0, b: 0 }));
  }

  _addFrame() {
    const f = { leds: this._blankLeds(), duration_ms: DEFAULT_DURATION };
    this._frames.push(f);
    this._rebuildList();
    this._openFrame(this._frames.length - 1);
  }

  _cloneFrame() {
    if (this._activeFrame < 0) { this._addFrame(); return; }
    const src = this._frames[this._activeFrame];
    const f   = { leds: src.leds.map(l => ({ ...l })), duration_ms: src.duration_ms };
    this._frames.splice(this._activeFrame + 1, 0, f);
    this._rebuildList();
    this._openFrame(this._activeFrame + 1);
  }

  _deleteFrame(idx) {
    this._frames.splice(idx, 1);
    const next = Math.min(idx, this._frames.length - 1);
    this._rebuildList();
    if (this._frames.length > 0) this._openFrame(next);
    else {
      this._activeFrame = -1;
      this._frameEditorEl.style.display = 'none';
    }
  }

  // ── Frame list (thumbnails) ──────────────────────────────────────────────────

  _rebuildList() {
    this._frameListEl.innerHTML = '';
    this._frames.forEach((f, i) => {
      const card = document.createElement('div');
      card.className  = 'frame-card' + (i === this._activeFrame ? ' frame-card--active' : '');

      // Mini dot grid
      const dotsWrap = document.createElement('div');
      dotsWrap.className = 'frame-card__dots';
      f.leds.forEach(l => {
        const d = document.createElement('div');
        d.className          = 'frame-card__dot';
        d.style.backgroundColor = rgbToHex(l.r, l.g, l.b);
        dotsWrap.appendChild(d);
      });

      const dur = document.createElement('div');
      dur.className   = 'frame-card__dur';
      dur.textContent = `${f.duration_ms} ms`;

      const del = document.createElement('button');
      del.className   = 'frame-card__del';
      del.textContent = '×';
      del.title       = 'Delete frame';
      del.addEventListener('click', e => { e.stopPropagation(); this._deleteFrame(i); });

      card.appendChild(del);
      card.appendChild(dotsWrap);
      card.appendChild(dur);
      card.addEventListener('click', () => this._openFrame(i));
      this._frameListEl.appendChild(card);
    });
  }

  // ── Per-frame editor ────────────────────────────────────────────────────────

  _openFrame(idx) {
    this._activeFrame = idx;
    this._rebuildList(); // re-render to update active highlight

    const frame = this._frames[idx];
    const el    = this._frameEditorEl;
    el.style.display = '';
    el.innerHTML = `
      <div class="frame-editor__title">Frame ${idx + 1}</div>
      <div class="frame-editor__led-grid" id="pe-led-grid"></div>
      <div class="frame-editor__duration">
        <label>Duration (ms)</label>
        <input type="number" id="pe-dur-input" min="50" max="60000" step="50"
               value="${frame.duration_ms}" />
        <button id="pe-send-frame" class="btn btn--sm">Send to LEDs</button>
      </div>
    `;

    // LED grid inside the frame editor.
    const gridEl = el.querySelector('#pe-led-grid');
    this._editorGrid  = new LedGrid(gridEl, LED_COUNT);
    this._editorGrid.setAll(frame.leds);

    // Colour change → update frame data.
    this._editorGrid.onChange = (i, r, g, b) => {
      frame.leds[i] = { r, g, b };
      this._rebuildList();
    };

    // Duration change → update frame data.
    el.querySelector('#pe-dur-input').addEventListener('change', e => {
      const v = parseInt(e.target.value, 10);
      if (!isNaN(v) && v >= 50) {
        frame.duration_ms = v;
        this._rebuildList();
      }
    });

    // Send this single frame's colours to the device immediately.
    el.querySelector('#pe-send-frame').addEventListener('click', async () => {
      try { await api.setLeds(frame.leds); } catch (e) { console.error(e); }
    });
  }

  // ── Play preview ──────────────────────────────────────────────────────────

  async _playLive() {
    if (this._frames.length === 0) { alert('Add at least one frame first.'); return; }
    try {
      // Save as a temp pattern, then play it.
      const { id } = await api.savePattern('__preview__', this._frames);
      await api.playPattern(id);
      // Clean up temp after a moment (non-critical).
      setTimeout(() => api.deletePattern(id).catch(() => {}), 30000);
    } catch (e) {
      console.error('Play preview error:', e);
    }
  }

  // ── Public helpers ────────────────────────────────────────────────────────

  /** Return current frames for saving. */
  getFrames() { return this._frames; }

  /** Load frames from saved pattern data. */
  loadFrames(frames) {
    this._frames      = frames.map(f => ({
      leds:        f.leds.map(l => ({ ...l })),
      duration_ms: f.duration_ms,
    }));
    this._activeFrame = -1;
    this._rebuildList();
    this._frameEditorEl.style.display = 'none';
    if (this._frames.length) this._openFrame(0);
  }
}
