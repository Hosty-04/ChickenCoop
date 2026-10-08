const REQUEST_MS = 8000;

export const el = (id) => document.getElementById(id);

export class HttpError extends Error {
  constructor(status, message) {
    super(message);
    this.status = status;
  }
}

export async function api(url, body) {
  const res = await fetch(url, {
    method: body === undefined ? 'GET' : 'POST',
    headers: body === undefined ? {} : { 'Content-Type': 'application/json' },
    body: body === undefined ? undefined : JSON.stringify(body),
    signal: AbortSignal.timeout(REQUEST_MS)
  });
  if (res.ok) return res.json();
  const data = await res.json().catch(() => ({}));
  throw new HttpError(res.status, data.error ?? `HTTP ${res.status}`);
}

export function reason(err) {
  return err instanceof HttpError ? err.message : 'server neodpovídá';
}
