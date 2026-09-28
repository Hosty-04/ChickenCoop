import { randomBytes, createHash, timingSafeEqual } from 'node:crypto';
import { config } from './config.js';

export const SESSION_COOKIE = 'kurnik_session';

const SESSION_TTL_MS = 30 * 24 * 60 * 60 * 1000;
const SWEEP_MS = 60 * 60 * 1000;
const MAX_ATTEMPTS = 5;
const LOCKOUT_MS = 15 * 60 * 1000;

const sessions = new Map();
const failures = new Map();

function digest(value) {
  return createHash('sha256').update(String(value ?? ''), 'utf8').digest();
}

function sameSecret(given, expected) {
  return timingSafeEqual(digest(given), digest(expected));
}

export function readCookie(header, name) {
  if (!header) return null;
  for (const part of header.split(';')) {
    const split = part.indexOf('=');
    if (split < 0) continue;
    if (part.slice(0, split).trim() === name) return part.slice(split + 1).trim();
  }
  return null;
}

export function lockoutRemainingMs(ip) {
  const record = failures.get(ip);
  if (!record) return 0;
  if (record.until <= Date.now()) {
    failures.delete(ip);
    return 0;
  }
  return record.count >= MAX_ATTEMPTS ? record.until - Date.now() : 0;
}

export function checkCredentials(ip, user, password) {
  const userOk = sameSecret(user, config.auth.user);
  const passwordOk = sameSecret(password, config.auth.password);

  if (!userOk || !passwordOk) {
    const record = failures.get(ip) ?? { count: 0, until: 0 };
    record.count += 1;
    record.until = Date.now() + LOCKOUT_MS;
    failures.set(ip, record);
    return false;
  }

  failures.delete(ip);
  return true;
}

export function openSession() {
  const token = randomBytes(32).toString('base64url');
  sessions.set(token, Date.now() + SESSION_TTL_MS);
  return token;
}

export function sessionValid(token) {
  if (!token) return false;
  const expires = sessions.get(token);
  if (expires === undefined) return false;
  if (expires <= Date.now()) {
    sessions.delete(token);
    return false;
  }
  return true;
}

export function closeSession(token) {
  if (token) sessions.delete(token);
}

export function requestAuthenticated(req) {
  return sessionValid(readCookie(req.headers?.cookie, SESSION_COOKIE));
}

setInterval(() => {
  const now = Date.now();
  for (const [token, expires] of sessions) if (expires <= now) sessions.delete(token);
  for (const [ip, record] of failures) if (record.until <= now) failures.delete(ip);
}, SWEEP_MS).unref();
