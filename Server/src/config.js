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

export const config = {
  port: Number(optional('PORT', '3000')),
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
  },
  influx: {
    url: optional('INFLUX_URL', 'http://localhost:8086'),
    token: required('INFLUX_TOKEN'),
    org: required('INFLUX_ORG'),
    bucket: optional('INFLUX_BUCKET', 'coop')
  }
};

export const ttnUsername = `${config.ttn.appId}@${config.ttn.tenant}`;
