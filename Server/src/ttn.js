import { EventEmitter } from 'node:events';
import mqtt from 'mqtt';
import { config, ttnUsername } from './config.js';
import { decodeUplink, encodeDownlink, UPLINK_PORT, DOWNLINK_PORT } from './codec.js';

const DOWN_EVENTS = ['sent', 'ack', 'nack', 'failed'];
const CORRELATION_PREFIX = 'kurnik';

function correlationIds(message) {
  const found = [];
  const visit = (value, depth) => {
    if (!value || typeof value !== 'object' || depth > 4) return;
    if (Array.isArray(value.correlation_ids)) found.push(...value.correlation_ids);
    for (const nested of Object.values(value)) visit(nested, depth + 1);
  };
  visit(message, 0);
  return found;
}

export class TtnBridge extends EventEmitter {
  constructor() {
    super();
    this.client = null;
    this.connected = false;
    this.pending = [];
    this.nextId = 1;
  }

  #base() {
    return `v3/${ttnUsername}/devices/${config.ttn.deviceId}`;
  }

  #setPending(list) {
    this.pending = list;
    this.emit('pending', this.pending);
  }

  start() {
    const url = `mqtts://${config.ttn.host}:${config.ttn.port}`;

    this.client = mqtt.connect(url, {
      username: ttnUsername,
      password: config.ttn.apiKey,
      reconnectPeriod: 5000,
      clean: true
    });

    this.client.on('connect', () => {
      this.connected = true;
      const topics = [
        `v3/${ttnUsername}/devices/+/up`,
        ...DOWN_EVENTS.map((event) => `v3/${ttnUsername}/devices/+/down/${event}`)
      ];
      this.client.subscribe(topics, { qos: 0 }, (err) => {
        if (err) this.emit('error', err);
        else this.emit('ready', topics);
      });
      this.emit('state', { connected: true });
    });

    this.client.on('reconnect', () => this.emit('state', { connected: false, reconnecting: true }));
    this.client.on('close', () => {
      this.connected = false;
      this.emit('state', { connected: false });
    });
    this.client.on('error', (err) => this.emit('error', err));
    this.client.on('message', (topic, buffer) => this.#onMessage(topic, buffer));
  }

  #onMessage(topic, buffer) {
    let message;
    try {
      message = JSON.parse(buffer.toString());
    } catch {
      this.emit('error', new Error(`message on ${topic} was not valid JSON`));
      return;
    }

    const event = DOWN_EVENTS.find((name) => topic.endsWith(`/down/${name}`));
    if (event) {
      const ids = correlationIds(message);
      const matched = this.pending.findIndex((entry) => ids.includes(entry.correlationId));
      const index = matched >= 0 ? matched : 0;
      const done = this.pending[index];

      if (done) this.#setPending(this.pending.filter((_, position) => position !== index));
      this.emit('downlink', { event, commands: done?.commands ?? null });
      return;
    }

    const uplink = message.uplink_message;
    if (!uplink || uplink.f_port !== UPLINK_PORT || !uplink.frm_payload) return;

    let reading;
    try {
      reading = decodeUplink(new Uint8Array(Buffer.from(uplink.frm_payload, 'base64')));
    } catch (err) {
      this.emit('error', err);
      return;
    }

    const best = (uplink.rx_metadata ?? [])
      .reduce((a, b) => ((b.rssi ?? -999) > (a?.rssi ?? -999) ? b : a), null);

    this.emit('uplink', {
      deviceId: message.end_device_ids?.device_id ?? config.ttn.deviceId,
      receivedAt: message.received_at ?? new Date().toISOString(),
      fCnt: uplink.f_cnt ?? null,
      reading,
      radio: {
        rssi: best?.rssi ?? null,
        snr: best?.snr ?? null,
        spreadingFactor: uplink.settings?.data_rate?.lora?.spreading_factor ?? null,
        gateway: best?.gateway_ids?.gateway_id ?? null
      }
    });
  }

  sendCommand(names) {
    const payload = encodeDownlink(names);

    if (!this.connected) throw new Error('not connected to TTN');

    const id = this.nextId++;
    const entry = {
      id,
      correlationId: `${CORRELATION_PREFIX}:${id}`,
      commands: names,
      byte: payload[0],
      queuedAt: new Date().toISOString()
    };

    const body = JSON.stringify({
      downlinks: [{
        f_port: DOWNLINK_PORT,
        frm_payload: Buffer.from(payload).toString('base64'),
        priority: 'NORMAL',
        correlation_ids: [entry.correlationId]
      }]
    });

    return new Promise((resolve, reject) => {
      this.client.publish(`${this.#base()}/down/push`, body, { qos: 1 }, (err) => {
        if (err) return reject(err);
        this.#setPending([...this.pending, entry]);
        resolve(entry);
      });
    });
  }

  clearQueue() {
    if (!this.connected) throw new Error('not connected to TTN');

    return new Promise((resolve, reject) => {
      const body = JSON.stringify({ downlinks: [] });
      this.client.publish(`${this.#base()}/down/replace`, body, { qos: 1 }, (err) => {
        if (err) return reject(err);
        const cleared = this.pending.length;
        this.#setPending([]);
        resolve({ cleared });
      });
    });
  }

  stop() {
    return new Promise((resolve) => {
      if (!this.client) return resolve();
      this.client.end(false, {}, resolve);
    });
  }
}
