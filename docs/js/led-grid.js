// led-grid.js — renders the LED dot grid and wires up per-LED colour pickers.
//
// Usage:
//   const grid = new LedGrid(containerEl, ledCount);
//   grid.onChange = (index, r, g, b) => { ... };   // fires on user interaction
//   grid.setColor(index, r, g, b);                  // update UI without firing onChange
//   grid.setAll(ledsArray);                         // [{r,g,b}, ...] from API

export class LedGrid {
  /**
   * @param {HTMLElement} container
   * @param {number} count  Number of LEDs.
   */
  constructor(container, count) {
    this._container = container;
    this._count     = count;
    this._dots      = [];
    this._inputs    = [];
    /** @type {function(number,number,number,number): void} */
    this.onChange = null;
    this._build();
  }

  _build() {
    this._container.innerHTML = '';
    for (let i = 0; i < this._count; i++) {
      const dot = document.createElement('div');
      dot.className   = 'led-dot';
      dot.textContent = i;

      // Native colour input overlaid on the dot.
      const input = document.createElement('input');
      input.type  = 'color';
      input.value = '#000000';
      input.title = `LED ${i}`;

      // Debounced change: fire onChange while dragging the colour wheel.
      let debounceId = null;
      input.addEventListener('input', () => {
        dot.style.backgroundColor = input.value;
        clearTimeout(debounceId);
        debounceId = setTimeout(() => {
          const [r, g, b] = hexToRgb(input.value);
          if (this.onChange) this.onChange(i, r, g, b);
        }, 80);
      });

      dot.appendChild(input);
      this._container.appendChild(dot);
      this._dots.push(dot);
      this._inputs.push(input);
    }
  }

  /** Update one LED's colour in the UI (does NOT fire onChange). */
  setColor(index, r, g, b) {
    if (index < 0 || index >= this._count) return;
    const hex = rgbToHex(r, g, b);
    this._inputs[index].value             = hex;
    this._dots[index].style.backgroundColor = hex;
  }

  /** Bulk update from API response array. */
  setAll(leds) {
    leds.forEach((c, i) => this.setColor(i, c.r, c.g, c.b));
  }

  /** Return current colours as [{r,g,b}] */
  getAll() {
    return this._inputs.map(inp => {
      const [r, g, b] = hexToRgb(inp.value);
      return { r, g, b };
    });
  }
}

// ── Colour utilities ──────────────────────────────────────────────────────────

export function hexToRgb(hex) {
  const n = parseInt(hex.replace('#', ''), 16);
  return [(n >> 16) & 0xFF, (n >> 8) & 0xFF, n & 0xFF];
}

export function rgbToHex(r, g, b) {
  return '#' + [r, g, b].map(v => v.toString(16).padStart(2, '0')).join('');
}
