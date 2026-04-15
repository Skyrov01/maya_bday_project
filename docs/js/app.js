// app.js — main entry point. Wires all modules together.

import { api }            from './api.js';
import { LedGrid, hexToRgb } from './led-grid.js';
import { EffectsPanel }   from './effects-panel.js';
import { PatternEditor }  from './pattern-editor.js';
import { StoragePanel }   from './storage-panel.js';

const LED_COUNT = 10;

// ── Element refs ──────────────────────────────────────────────────────────────

const ipInput        = document.getElementById('ip-input');
const btnConnect     = document.getElementById('btn-connect');
const connStatus     = document.getElementById('conn-status');
const banner         = document.getElementById('mixed-content-banner');
const dismissBanner  = document.getElementById('dismiss-banner');

const btnAllOff      = document.getElementById('btn-all-off');
const btnFill        = document.getElementById('btn-fill');
const fillColor      = document.getElementById('fill-color');
const brightnessSlider = document.getElementById('brightness-slider');
const brightnessValue  = document.getElementById('brightness-value');

const tabs           = document.querySelectorAll('.tab');
const tabPanels      = document.querySelectorAll('.tab-panel');

// ── Initialise modules ────────────────────────────────────────────────────────

const ledGrid      = new LedGrid(document.getElementById('led-grid'), LED_COUNT);
const effectsPanel = new EffectsPanel(document.getElementById('effects-panel'));
const patternEditor = new PatternEditor(document.getElementById('pattern-editor'));
const storagePanel  = new StoragePanel(document.getElementById('storage-panel'), patternEditor);

// ── Restore saved IP ──────────────────────────────────────────────────────────

if (api.baseUrl) {
  ipInput.value = api.baseUrl.replace(/^https?:\/\//, '');
}

// ── Mixed-content banner ──────────────────────────────────────────────────────
// Show once if we're on HTTPS (GitHub Pages) since the Wemos is HTTP.

if (location.protocol === 'https:' && !localStorage.getItem('mc_dismissed')) {
  banner.hidden = false;
}
dismissBanner.addEventListener('click', () => {
  banner.hidden = true;
  localStorage.setItem('mc_dismissed', '1');
});

// ── Connection ────────────────────────────────────────────────────────────────

function setStatus(state) {
  connStatus.className = `status status--${state}`;
  connStatus.title = { 'off': 'Disconnected', 'pending': 'Connecting…', 'on': 'Connected' }[state] ?? '';
}

async function connect() {
  const raw = ipInput.value.trim();
  if (!raw) { alert('Enter the Wemos IP address.'); return; }
  api.setBaseUrl(raw);
  setStatus('pending');
  try {
    const status = await api.status();
    setStatus('on');
    brightnessSlider.value = status.brightness;
    brightnessValue.textContent = status.brightness;
    effectsPanel.syncFromStatus(status);
    // Fetch and display current LED colours.
    const { leds } = await api.getLeds();
    ledGrid.setAll(leds);
  } catch (e) {
    setStatus('off');
    console.error('Connection failed:', e);
    alert(`Could not connect to ${api.baseUrl}.\n\nCheck:\n• IP is correct\n• Wemos is online and on the same network\n• If this page is HTTPS, allow insecure content in browser settings (see banner above)`);
  }
}

btnConnect.addEventListener('click', connect);
ipInput.addEventListener('keydown', e => { if (e.key === 'Enter') connect(); });

// Auto-connect if we have a saved URL.
if (api.baseUrl) connect();

// ── Tab switching ─────────────────────────────────────────────────────────────

tabs.forEach(tab => {
  tab.addEventListener('click', () => {
    const target = tab.dataset.tab;
    tabs.forEach(t => t.classList.toggle('tab--active', t.dataset.tab === target));
    tabPanels.forEach(p => {
      const panelId = p.id.replace('tab-', '');
      p.hidden = panelId !== target;
    });
    // Refresh pattern list when switching to the patterns tab.
    if (target === 'patterns') storagePanel.refresh();
  });
});

// ── Control tab: per-LED colour ───────────────────────────────────────────────

ledGrid.onChange = async (index, r, g, b) => {
  try { await api.setLed(index, r, g, b); }
  catch (e) { console.error('setLed error:', e); }
};

// ── Control tab: fill all ─────────────────────────────────────────────────────

btnFill.addEventListener('click', async () => {
  const [r, g, b] = hexToRgb(fillColor.value);
  const leds = Array.from({ length: LED_COUNT }, () => ({ r, g, b }));
  ledGrid.setAll(leds);
  try { await api.setLeds(leds); }
  catch (e) { console.error('fill error:', e); }
});

btnAllOff.addEventListener('click', async () => {
  const leds = Array.from({ length: LED_COUNT }, () => ({ r: 0, g: 0, b: 0 }));
  ledGrid.setAll(leds);
  try { await api.setLeds(leds); }
  catch (e) { console.error('all-off error:', e); }
});

// ── Control tab: brightness ───────────────────────────────────────────────────

let brightnessDebounce = null;
brightnessSlider.addEventListener('input', () => {
  const v = parseInt(brightnessSlider.value, 10);
  brightnessValue.textContent = v;
  clearTimeout(brightnessDebounce);
  brightnessDebounce = setTimeout(async () => {
    try { await api.setBrightness(v); }
    catch (e) { console.error('brightness error:', e); }
  }, 120);
});
