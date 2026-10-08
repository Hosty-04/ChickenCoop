const THEME_COLORS = { light: '#f9f9f7', dark: '#0d0d0d' };

function applyTheme(theme) {
  document.documentElement.dataset.theme = theme;
  for (const meta of document.querySelectorAll('meta[name="theme-color"]')) meta.content = THEME_COLORS[theme];
}

try {
  const stored = localStorage.getItem('theme');
  if (Object.hasOwn(THEME_COLORS, stored)) applyTheme(stored);
} catch {}
