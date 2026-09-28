import test from 'node:test';
import assert from 'node:assert/strict';
import { decodeUplink, encodeDownlink } from '../src/codec.js';

const hex = (s) => Uint8Array.from(s.split(' ').map((b) => parseInt(b, 16)));

test('decodes the payloads observed on the real device', () => {
  assert.deepEqual(decodeUplink(hex('00 FD')), {
    panelMv: 0, panelSaturated: false,
    batteryMv: null, batterySaturated: false,
    batteryCritical: false, door: 'open'
  });

  assert.deepEqual(decodeUplink(hex('82 7D')), {
    panelMv: 6500, panelSaturated: false,
    batteryMv: 6550, batterySaturated: false,
    batteryCritical: false, door: 'open'
  });

  assert.deepEqual(decodeUplink(hex('82 7E')), {
    panelMv: 6500, panelSaturated: false,
    batteryMv: 6550, batterySaturated: false,
    batteryCritical: false, door: 'fault'
  });

  assert.deepEqual(decodeUplink(hex('82 79')), {
    panelMv: 6500, panelSaturated: false,
    batteryMv: 6500, batterySaturated: false,
    batteryCritical: false, door: 'open'
  });

  assert.deepEqual(decodeUplink(hex('82 7A')), {
    panelMv: 6500, panelSaturated: false,
    batteryMv: 6500, batterySaturated: false,
    batteryCritical: false, door: 'fault'
  });
});

test('decodes the saturation and dead-sensor codes the firmware emits', () => {
  assert.deepEqual(decodeUplink(hex('FB F3')), {
    panelMv: 12500, panelSaturated: true,
    batteryMv: 8000, batterySaturated: true,
    batteryCritical: true, door: 'unknown'
  });

  assert.deepEqual(decodeUplink(hex('FF FF')), {
    panelMv: null, panelSaturated: false,
    batteryMv: null, batterySaturated: false,
    batteryCritical: true, door: 'unknown'
  });
});

test('covers every door state', () => {
  assert.equal(decodeUplink(hex('00 00')).door, 'closed');
  assert.equal(decodeUplink(hex('00 01')).door, 'open');
  assert.equal(decodeUplink(hex('00 02')).door, 'fault');
  assert.equal(decodeUplink(hex('00 03')).door, 'unknown');
});

test('rejects a payload that is too short', () => {
  assert.throws(() => decodeUplink(hex('00')), /at least 2 bytes/);
});

test('encodes the command bytes the firmware expects', () => {
  assert.deepEqual(encodeDownlink(['systemOn']), Uint8Array.of(0x01));
  assert.deepEqual(encodeDownlink(['systemOff']), Uint8Array.of(0x02));
  assert.deepEqual(encodeDownlink(['doorOpen']), Uint8Array.of(0x04));
  assert.deepEqual(encodeDownlink(['doorClose']), Uint8Array.of(0x08));
  assert.deepEqual(encodeDownlink(['block']), Uint8Array.of(0x10));
  assert.deepEqual(encodeDownlink(['unblock']), Uint8Array.of(0x20));
});

test('combines commands from different pairs', () => {
  assert.deepEqual(encodeDownlink(['unblock', 'doorOpen']), Uint8Array.of(0x24));
  assert.deepEqual(encodeDownlink(['systemOn', 'doorClose']), Uint8Array.of(0x09));
});

test('refuses combinations the firmware would silently ignore', () => {
  assert.throws(() => encodeDownlink(['doorOpen', 'doorClose']), /cancel each other out/);
  assert.throws(() => encodeDownlink(['systemOn', 'systemOff']), /cancel each other out/);
  assert.throws(() => encodeDownlink(['block', 'unblock']), /cancel each other out/);
});

test('refuses an unknown or empty command', () => {
  assert.throws(() => encodeDownlink(['doorSlam']), /unknown command/);
  assert.throws(() => encodeDownlink([]), /at least one command/);
});
