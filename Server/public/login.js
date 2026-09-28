const form = document.getElementById('form');
const error = document.getElementById('error');
const submit = document.getElementById('submit');

form.addEventListener('submit', async (event) => {
  event.preventDefault();
  error.hidden = true;
  submit.disabled = true;
  submit.textContent = 'Přihlašuji…';

  try {
    const res = await fetch('/api/login', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        user: document.getElementById('user').value,
        password: document.getElementById('password').value
      })
    });
    const body = await res.json().catch(() => ({}));

    if (res.ok) {
      location.replace('/');
      return;
    }
    throw new Error(body.error ?? 'Přihlášení se nezdařilo.');
  } catch (err) {
    error.textContent = err.message;
    error.hidden = false;
    document.getElementById('password').value = '';
    document.getElementById('password').focus();
  } finally {
    submit.disabled = false;
    submit.textContent = 'Přihlásit';
  }
});
