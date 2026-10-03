export const UPLINK_PORT = 2;
export const DOWNLINK_PORT = 2;
const STATUS_LENGTH = 2;

const PANEL_STEP_MV = 100;
const PANEL_CODE_MAX = 125;
const PANEL_CODE_NODATA = 127;

const BATTERY_OFFSET_MV = 5000;
const BATTERY_STEP_MV = 50;
const BATTERY_CODE_MAX = 60;
const BATTERY_CODE_NODATA = 63;

export const EGGS_MAX = 10;
const NEST_CODE_BROODY = 11;
const NEST_CODE_UNCALIBRATED = 12;
const NEST_CODE_FAULT = 14;
const NEST_CODE_NODATA = 15;

export const DOOR_STATES = ['closed', 'open', 'fault', 'unknown'];
export const NEST_STATES = ['ok', 'broody', 'uncalibrated', 'fault', 'offline'];

export const COMMANDS = {
  systemOn: 0x01,
  systemOff: 0x02,
  doorOpen: 0x04,
  doorClose: 0x08,
  block: 0x10,
  unblock: 0x20
};

export const NEST_COMMANDS = {
  tare: 0x01,
  calibrate: 0x02
};

const NESTS_PER_BYTE = 4;

const EXCLUSIVE_PAIRS = [
  ['systemOn', 'systemOff'],
  ['doorOpen', 'doorClose'],
  ['block', 'unblock'],
  ['tare', 'calibrate']
];

function decodeNest(code) {
  if (code <= EGGS_MAX) return { eggs: code, state: 'ok' };
  if (code === NEST_CODE_BROODY) return { eggs: null, state: 'broody' };
  if (code === NEST_CODE_UNCALIBRATED) return { eggs: null, state: 'uncalibrated' };
  if (code === NEST_CODE_FAULT) return { eggs: null, state: 'fault' };
  return { eggs: null, state: 'offline' };
}

function nestCode(bytes, index) {
  const byte = bytes[STATUS_LENGTH + (index >> 1)];
  if (byte === undefined) return NEST_CODE_NODATA;
  return index % 2 === 0 ? byte >> 4 : byte & 0x0f;
}

export function decodeUplink(bytes, nestCount) {
  if (!(bytes instanceof Uint8Array) || bytes.length < STATUS_LENGTH) {
    throw new Error(`payload must be at least ${STATUS_LENGTH} bytes, got ${bytes?.length ?? 0}`);
  }

  const panelCode = bytes[0] >> 1;
  const batteryCode = bytes[1] >> 2;
  const doorCode = bytes[1] & 0x03;

  return {
    panelMv: panelCode === PANEL_CODE_NODATA ? null : panelCode * PANEL_STEP_MV,
    panelSaturated: panelCode === PANEL_CODE_MAX,
    batteryMv: batteryCode === BATTERY_CODE_NODATA
      ? null
      : BATTERY_OFFSET_MV + batteryCode * BATTERY_STEP_MV,
    batterySaturated: batteryCode === BATTERY_CODE_MAX,
    batteryCritical: (bytes[0] & 0x01) === 0x01,
    door: DOOR_STATES[doorCode],
    nests: bytes.length > STATUS_LENGTH
      ? Array.from({ length: nestCount }, (_, index) => decodeNest(nestCode(bytes, index)))
      : null
  };
}

export function encodeDownlink(names, nests, nestCount) {
  if (!Array.isArray(names) || names.length === 0) {
    throw new Error('je potřeba alespoň jeden příkaz');
  }

  const unknown = names.filter((name) => !(name in COMMANDS) && !(name in NEST_COMMANDS));
  if (unknown.length > 0) {
    throw new Error(`neznámý příkaz: ${unknown.join(', ')}`);
  }

  for (const [a, b] of EXCLUSIVE_PAIRS) {
    if (names.includes(a) && names.includes(b)) {
      throw new Error(`příkazy ${a} a ${b} se navzájem ruší, kurník by je ignoroval`);
    }
  }

  const byte = names.reduce((acc, name) => acc | (COMMANDS[name] ?? 0), 0);
  const nestCommand = names.find((name) => name in NEST_COMMANDS);

  if (!nestCommand) return Uint8Array.of(byte);

  if (!Array.isArray(nests) || nests.length === 0) {
    throw new Error('vyberte aspoň jedno hnízdo');
  }

  if (nests.some((nest) => !Number.isInteger(nest) || nest < 1 || nest > nestCount)) {
    throw new Error(`hnízdo musí být číslo od 1 do ${nestCount}`);
  }

  const bytes = new Uint8Array(1 + Math.ceil(Math.max(...nests) / NESTS_PER_BYTE));
  bytes[0] = byte;
  for (const nest of nests) {
    const index = nest - 1;
    bytes[1 + Math.floor(index / NESTS_PER_BYTE)] |= NEST_COMMANDS[nestCommand] << (2 * (index % NESTS_PER_BYTE));
  }

  return bytes;
}
