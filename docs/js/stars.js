// stars.js — twinkling star particles for the landing page and controller background.

function createStars(container, count) {
  for (let i = 0; i < count; i++) {
    const star = document.createElement('span');
    const large = Math.random() < 0.12;
    const size = large
      ? (6 + Math.random() * 4).toFixed(1)
      : (1 + Math.random() * 2.5).toFixed(1);
    star.className = large ? 'star star--sparkle' : 'star';
    star.style.cssText =
      `top:${(Math.random() * 100).toFixed(1)}%;` +
      `left:${(Math.random() * 100).toFixed(1)}%;` +
      `width:${size}px;` +
      `height:${size}px;` +
      `animation-delay:${(Math.random() * 6).toFixed(2)}s;` +
      `animation-duration:${(2.5 + Math.random() * 4).toFixed(2)}s;`;
    container.appendChild(star);
  }
}

// ── Landing page ──────────────────────────────────────────────────────────────

const landing = document.getElementById('landing');
const bgAudio = document.getElementById('bg-audio');

// Play on the very first user interaction with the page.
if (bgAudio) {
  const startAudio = () => {
    bgAudio.play().catch(() => {});
    document.removeEventListener('pointerdown', startAudio);
  };
  document.addEventListener('pointerdown', startAudio);
}

if (landing) {
  createStars(landing.querySelector('.stars-layer'), 120);

  if (sessionStorage.getItem('enteredApp')) {
    landing.style.display = 'none';
  }

  document.getElementById('btn-open').addEventListener('click', () => {
    if (bgAudio) bgAudio.play().catch(() => {});
    landing.classList.add('landing--fade');
    landing.addEventListener('transitionend', () => {
      landing.style.display = 'none';
      sessionStorage.setItem('enteredApp', 'true');
    }, { once: true });
  });
}

const btnBack = document.getElementById('btn-back');
if (btnBack && landing) {
  btnBack.addEventListener('click', () => {
    landing.style.display = '';
    landing.offsetHeight; // force reflow so transition plays
    landing.classList.remove('landing--fade');
    // Resume audio when returning to landing.
    if (bgAudio) bgAudio.play().catch(() => {});
    sessionStorage.removeItem('enteredApp');
  });
}

// ── Controller background ─────────────────────────────────────────────────────

const controllerStars = document.getElementById('controller-stars');
if (controllerStars) createStars(controllerStars, 60);
