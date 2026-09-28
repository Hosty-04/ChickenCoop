import 'dotenv/config';

function required(name) {
  const value = process.env[name];
  if (!value) {
    throw new Error(`missing environment variable ${name} (copy .env.example to .env and fill it in)`);
  }
  return value;
}

function optional(name, fallback) {
  const value = process.env[name];
  return value === undefined || value === '' ? fallback : value;
}

function trustProxy() {
  const value = optional('TRUST_PROXY', '');
  if (value === '' || value === '0' || value === 'false') return false;
  if (/^\d+$/.test(value)) return Number(value);
  if (value === 'true') return true;
  return value;
}

export const config = {
  port: Number(optional('PORT', '3000')),
  trustProxy: trustProxy(),
  dbPath: optional('DB_PATH', './data/kurnik.db'),
  auth: {
    user: optional('AUTH_USER', 'kurnik'),
    password: required('AUTH_PASSWORD')
  },
  ttn: {
    host: optional('TTN_HOST', 'eu1.cloud.thethings.network'),
    port: Number(optional('TTN_PORT', '8883')),
    appId: required('TTN_APP_ID'),
    tenant: optional('TTN_TENANT', 'ttn'),
    apiKey: required('TTN_API_KEY'),
    deviceId: required('TTN_DEVICE_ID')
  }
};

export const ttnUsername = `${config.ttn.appId}@${config.ttn.tenant}`;
