const REQUEST_MS = 8000;

export const el = (id) => document.getElementById(id);

export class HttpError extends Error {
  constructor(status, message) {
    super(message);
    this.status = status;
  }
}

export async function api(url, body) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), REQUEST_MS);
  try {
    const res = await fetch(url, {
      method: body === undefined ? 'GET' : 'POST',
      headers: body === undefined ? {} : { 'Content-Type': 'application/json' },
      body: body === undefined ? undefined : JSON.stringify(body),
      signal: controller.signal
    });
    if (res.ok) return await res.json();
    const data = await res.json().catch(() => ({}));
    throw new HttpError(res.status, data.error ?? `HTTP ${res.status}`);
  } finally {
    clearTimeout(timer);
  }
}

export function reason(err) {
  return err instanceof HttpError ? err.message : 'server neodpovídá';
}
