// storage-panel.js — list, play, and delete patterns saved on the Wemos.

import { api } from './api.js';

export class StoragePanel {
  /**
   * @param {HTMLElement} container
   * @param {import('./pattern-editor.js').PatternEditor} patternEditor
   */
  constructor(container, patternEditor) {
    this._container    = container;
    this._patternEditor = patternEditor;
    this._build();
  }

  _build() {
    this._container.innerHTML = `
      <div class="section-title">Saved Patterns
        <button id="sp-refresh" class="btn btn--sm btn--ghost">↻ Refresh</button>
      </div>
      <div class="storage-panel__save">
        <input id="sp-name" type="text" placeholder="Pattern name" maxlength="32" />
        <button id="sp-save" class="btn btn--sm">💾 Save from Editor</button>
      </div>
      <ul id="sp-list" class="pattern-list"></ul>
    `;

    this._listEl = this._container.querySelector('#sp-list');

    this._container.querySelector('#sp-refresh').addEventListener('click', () => this.refresh());
    this._container.querySelector('#sp-save').addEventListener('click',    () => this._saveFromEditor());
  }

  async refresh() {
    this._listEl.innerHTML = '<li style="color:var(--text-dim);font-size:13px">Loading…</li>';
    try {
      const patterns = await api.listPatterns();
      this._render(patterns);
    } catch (e) {
      this._listEl.innerHTML = `<li style="color:var(--danger);font-size:13px">Error: ${e.message}</li>`;
    }
  }

  _render(patterns) {
    this._listEl.innerHTML = '';
    if (patterns.length === 0) {
      this._listEl.innerHTML = '<li style="color:var(--text-dim);font-size:13px">No saved patterns.</li>';
      return;
    }
    for (const p of patterns) {
      this._listEl.appendChild(this._makeItem(p));
    }
  }

  _makeItem(p) {
    const li = document.createElement('li');
    li.className = 'pattern-item';

    const nameSpan = document.createElement('span');
    nameSpan.className   = 'pattern-item__name';
    nameSpan.textContent = p.name;

    const playBtn = document.createElement('button');
    playBtn.className   = 'btn btn--sm pattern-item__play';
    playBtn.textContent = '▶ Play';
    playBtn.addEventListener('click', async () => {
      try { await api.playPattern(p.id); }
      catch (e) { alert('Play failed: ' + e.message); }
    });

    const delBtn = document.createElement('button');
    delBtn.className   = 'btn btn--sm btn--danger';
    delBtn.textContent = '🗑';
    delBtn.title       = 'Delete';
    delBtn.addEventListener('click', async () => {
      if (!confirm(`Delete "${p.name}"?`)) return;
      try {
        await api.deletePattern(p.id);
        li.remove();
      } catch (e) {
        alert('Delete failed: ' + e.message);
      }
    });

    li.appendChild(nameSpan);
    li.appendChild(playBtn);
    li.appendChild(delBtn);
    return li;
  }

  async _saveFromEditor() {
    const frames = this._patternEditor.getFrames();
    if (frames.length === 0) { alert('Add at least one frame in the pattern editor first.'); return; }

    const nameInput = this._container.querySelector('#sp-name');
    const name      = nameInput.value.trim() || 'Unnamed';

    try {
      await api.savePattern(name, frames);
      nameInput.value = '';
      await this.refresh();
    } catch (e) {
      alert('Save failed: ' + e.message);
    }
  }
}
