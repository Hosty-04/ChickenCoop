const REQUEST_MS = 8000;

const form = document.getElementById('form');
const error = document.getElementById('error');
const submit = document.getElementById('submit');

form.addEventListener('submit', async (event) => {
  event.preventDefault();
  error.hidden = true;
  submit.disabled = true;
  submit.textContent = 'Přihlašuji…';

  try {
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), REQUEST_MS);
    const res = await fetch('/api/login', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      signal: controller.signal,
      body: JSON.stringify({
        user: document.getElementById('user').value,
        password: document.getElementById('password').value
      })
    }).finally(() => clearTimeout(timer));
    const body = await res.json().catch(() => ({}));

    if (res.ok) {
      location.replace('/');
      return;
    }
    throw new Error(body.error ?? 'Přihlášení se nezdařilo.');
  } catch (err) {
    const lost = err instanceof TypeError || err?.name === 'AbortError' || err?.name === 'TimeoutError';
    error.textContent = lost ? 'Server neodpovídá.' : err.message;
    error.hidden = false;
    document.getElementById('password').value = '';
    document.getElementById('password').focus();
  } finally {
    submit.disabled = false;
    submit.textContent = 'Přihlásit';
  }
});
