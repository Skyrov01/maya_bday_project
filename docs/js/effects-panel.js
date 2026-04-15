// effects-panel.js — renders built-in effect buttons and wires up the API.

import { api } from './api.js';

const EFFECTS = [
  { id: 'chase',     label: '🔵 Chase'     },
  { id: 'rainbow',   label: '🌈 Rainbow'   },
  { id: 'sparkle',   label: '✨ Sparkle'   },
  { id: 'comet',     label: '☄️  Comet'    },
  { id: 'colorwipe', label: '🎨 Color Wipe' },
  { id: 'pingpong',  label: '🏓 Ping Pong' },
];

export class EffectsPanel {
  /** @param {HTMLElement} container */
  constructor(container) {
    this._container   = container;
    this._activeId    = null;
    this._btnMap      = {};
    this._build();
  }

  _build() {
    this._container.innerHTML = '';

    const grid = document.createElement('div');
    grid.className = 'effects-grid';

    for (const { id, label } of EFFECTS) {
      const btn = document.createElement('button');
      btn.className   = 'effect-btn';
      btn.textContent = label;
      btn.dataset.id  = id;
      btn.addEventListener('click', () => this._activate(id));
      grid.appendChild(btn);
      this._btnMap[id] = btn;
    }
    this._container.appendChild(grid);

    // Stop button
    const stop = document.createElement('button');
    stop.className   = 'btn btn--ghost';
    stop.textContent = '⏹ Stop';
    stop.addEventListener('click', () => this._activate('stop'));
    this._container.appendChild(stop);
  }

  async _activate(id) {
    try {
      await api.runEffect(id);
      this._setActive(id === 'stop' ? null : id);
    } catch (e) {
      console.error('Effect error:', e);
    }
  }

  _setActive(id) {
    for (const [eid, btn] of Object.entries(this._btnMap)) {
      btn.classList.toggle('effect-btn--active', eid === id);
    }
    this._activeId = id;
  }

  /** Sync active state from an API status response. */
  syncFromStatus(status) {
    this._setActive(status.effect === 'none' ? null : status.effect);
  }
}
