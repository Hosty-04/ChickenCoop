import { EventEmitter } from 'node:events';
import mqtt from 'mqtt';
import { config, ttnUsername } from './config.js';
import { decodeUplink, encodeDownlink, UPLINK_PORT, DOWNLINK_PORT, SEQUENCE_MAX, STAMP_MASK } from './codec.js';

const NOT_CONNECTED = 'server není spojený s The Things Network';
const DOWN_EVENTS = ['sent', 'ack', 'nack', 'failed'];
const QUEUE_EVENTS = ['sent', 'failed'];
const SENT_MAX = 16;
export const COMMAND_TTL_MS = 30 * 60 * 1000;
const EXPIRY_CHECK_MS = 15 * 1000;
const CORRELATION_TAG = 'kurnik:';
const CORRELATION_PREFIX = `${CORRELATION_TAG}${Date.now().toString(36)}:`;

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

export class DuplicateCommandError extends Error {}

export class TtnBridge extends EventEmitter {
  constructor() {
    super();
    this.client = null;
    this.connected = false;
    this.pending = [];
    this.sent = new Map();
    this.nextId = 1;
    this.sequence = 1 + Math.floor(Math.random() * SEQUENCE_MAX);
    this.queueReset = false;
    this.expiring = false;
    this.expiryTimer = null;
    this.expired = new Map();
    this.clockOffset = 0;
  }

  #now() {
    return Date.now() + this.clockOffset;
  }

  #age(entry) {
    return this.#now() - Date.parse(entry.queuedAt);
  }

  #syncClock(message) {
    const at = Date.parse(message?.received_at);
    if (Number.isFinite(at)) this.clockOffset = at - Date.now();
  }

  restoreSequence(saved) {
    if (Number.isInteger(saved) && saved >= 1 && saved <= SEQUENCE_MAX) this.sequence = saved;
  }

  #nextSequence() {
    this.sequence = (this.sequence % SEQUENCE_MAX) + 1;
    this.emit('sequence', this.sequence);
    return this.sequence;
  }

  #base() {
    return `v3/${ttnUsername}/devices/${config.ttn.deviceId}`;
  }

  #setPending(list) {
    this.pending = list;
    this.emit('pending', this.pending);
  }

  #takeQueued(ids, tagged) {
    const entry = this.pending.find((queued) => ids.includes(queued.correlationId)) ?? (tagged ? null : this.pending[0]);
    if (entry) this.#setPending(this.pending.filter((queued) => queued !== entry));
    return entry ?? null;
  }

  #takeSent(ids, tagged) {
    const id = ids.find((value) => this.sent.has(value)) ?? (tagged ? undefined : this.sent.keys().next().value);
    const entry = this.sent.get(id) ?? null;
    this.sent.delete(id);
    return entry;
  }

  #downlink(entry) {
    return {
      f_port: DOWNLINK_PORT,
      frm_payload: Buffer.from(entry.payload, 'hex').toString('base64'),
      priority: 'NORMAL',
      confirmed: true,
      correlation_ids: [entry.correlationId]
    };
  }

  #publish(action, entries) {
    const body = JSON.stringify({ downlinks: entries.map((entry) => this.#downlink(entry)) });
    return new Promise((resolve, reject) => {
      this.client.publish(`${this.#base()}/down/${action}`, body, { qos: 1 }, (err) => (err ? reject(err) : resolve()));
    });
  }

  #retire(entries) {
    for (const entry of entries) this.expired.set(entry.correlationId, entry);
    while (this.expired.size > SENT_MAX) this.expired.delete(this.expired.keys().next().value);
    this.emit('expired', entries);
  }

  #purge() {
    this.#publish('replace', this.pending).catch((err) => this.emit('error', err));
  }

  #expire() {
    const expired = this.pending.filter((entry) => this.#age(entry) >= COMMAND_TTL_MS);
    if (expired.length === 0 || !this.connected || this.expiring) return;

    this.expiring = true;
    this.#publish('replace', this.pending.filter((entry) => !expired.includes(entry)))
      .then(() => {
        this.#setPending(this.pending.filter((entry) => !expired.includes(entry)));
        this.#retire(expired);
      })
      .catch((err) => this.emit('error', err))
      .finally(() => {
        this.expiring = false;
      });
  }

  #remember(entry) {
    this.sent.set(entry.correlationId, entry);
    if (this.sent.size > SENT_MAX) this.sent.delete(this.sent.keys().next().value);
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
      if (!this.queueReset) {
        this.queueReset = true;
        this.#publish('replace', []).catch((err) => this.emit('error', err));
      }
      const topics = [
        `${this.#base()}/up`,
        ...DOWN_EVENTS.map((event) => `${this.#base()}/down/${event}`)
      ];
      this.client.subscribe(topics, { qos: 0 }, (err) => {
        if (err) this.emit('error', err);
        else this.emit('ready', topics);
      });
      this.emit('state', { connected: true });
      this.#expire();
    });

    this.expiryTimer = setInterval(() => this.#expire(), EXPIRY_CHECK_MS);
    this.expiryTimer.unref();

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

    this.#syncClock(message);

    const event = DOWN_EVENTS.find((name) => topic.endsWith(`/down/${name}`));
    if (event) {
      const ids = correlationIds(message);
      const tagged = ids.some((id) => id.startsWith(CORRELATION_TAG));

      if (ids.some((id) => this.expired.has(id))) {
        if (event === 'nack') this.#purge();
        return;
      }

      const entry = QUEUE_EVENTS.includes(event) ? this.#takeQueued(ids, tagged) : this.#takeSent(ids, tagged);

      if (entry && (event === 'sent' || event === 'nack') && this.#age(entry) >= COMMAND_TTL_MS) {
        this.#retire([entry]);
        if (event === 'nack') this.#purge();
        return;
      }

      if (entry && event === 'sent') this.#remember(entry);
      if (entry && event === 'nack') this.#setPending([...this.pending, entry]);
      this.emit('downlink', { event, commands: entry?.commands ?? null, nests: entry?.nests ?? null });
      return;
    }

    const uplink = message.uplink_message;
    if (!uplink || uplink.f_port !== UPLINK_PORT || !uplink.frm_payload) return;

    let reading;
    try {
      reading = decodeUplink(Buffer.from(uplink.frm_payload, 'base64'), config.nestCount);
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

  sendCommand(names, nests) {
    const plain = Buffer.from(encodeDownlink(names, nests, config.nestCount));
    const key = plain.toString('hex');

    if (!this.connected) throw new Error(NOT_CONNECTED);
    if (this.pending.some((entry) => entry.key === key)) {
      throw new DuplicateCommandError('stejný příkaz už ve frontě čeká');
    }

    const now = this.#now();
    const sequence = this.#nextSequence();
    const stamp = Math.floor(now / 1000) & STAMP_MASK;
    const payload = Buffer.from(encodeDownlink(names, nests, config.nestCount, sequence, stamp));
    const id = this.nextId++;
    const entry = {
      id,
      correlationId: `${CORRELATION_PREFIX}${id}`,
      commands: names,
      nests: plain.length > 1 ? [...new Set(nests)].sort((a, b) => a - b) : null,
      byte: plain[0],
      sequence,
      stamp,
      key,
      payload: payload.toString('hex'),
      queuedAt: new Date(now).toISOString()
    };

    this.#setPending([...this.pending, entry]);

    return this.#publish('push', [entry]).then(() => entry, (err) => {
      this.#setPending(this.pending.filter((queued) => queued !== entry));
      throw err;
    });
  }

  async clearQueue() {
    if (!this.connected) throw new Error(NOT_CONNECTED);

    await this.#publish('replace', []);
    const cleared = this.pending.length;
    this.#setPending([]);
    return { cleared };
  }

  stop() {
    clearInterval(this.expiryTimer);
    return new Promise((resolve) => {
      if (!this.client) return resolve();
      this.client.end(false, {}, resolve);
    });
  }
}
