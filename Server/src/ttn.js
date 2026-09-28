import { EventEmitter } from 'node:events';
import mqtt from 'mqtt';
import { config, ttnUsername } from './config.js';
import { decodeUplink, encodeDownlink, UPLINK_PORT, DOWNLINK_PORT } from './codec.js';

export class TtnBridge extends EventEmitter {
  constructor() {
    super();
    this.client = null;
    this.connected = false;
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
      const topic = `v3/${ttnUsername}/devices/+/up`;
      this.client.subscribe(topic, { qos: 0 }, (err) => {
        if (err) this.emit('error', err);
        else this.emit('ready', topic);
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
      this.emit('error', new Error(`uplink on ${topic} was not valid JSON`));
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
    const topic = `v3/${ttnUsername}/devices/${config.ttn.deviceId}/down/push`;
    const body = JSON.stringify({
      downlinks: [{
        f_port: DOWNLINK_PORT,
        frm_payload: Buffer.from(payload).toString('base64'),
        priority: 'NORMAL'
      }]
    });

    return new Promise((resolve, reject) => {
      this.client.publish(topic, body, { qos: 1 }, (err) => {
        if (err) reject(err);
        else resolve({ byte: payload[0], commands: names });
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
