import { el, api, HttpError } from '/common.js';

const form = el('form');
const user = el('user');
const password = el('password');
const submit = el('submit');
const error = el('error');

form.addEventListener('submit', async (event) => {
  event.preventDefault();
  error.hidden = true;
  submit.disabled = true;
  submit.textContent = 'Přihlašuji…';

  try {
    await api('/api/login', { user: user.value, password: password.value });
    location.replace('/');
  } catch (err) {
    const rejected = err instanceof HttpError;
    error.textContent = rejected ? err.message : 'Server neodpovídá.';
    error.hidden = false;
    submit.disabled = false;
    submit.textContent = 'Přihlásit';
    if (rejected) password.value = '';
    password.focus();
  }
});
