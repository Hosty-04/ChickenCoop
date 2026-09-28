export const UPLINK_PORT = 2;
export const DOWNLINK_PORT = 2;
export const STATUS_LENGTH = 2;

const PANEL_STEP_MV = 100;
const PANEL_CODE_MAX = 125;
const PANEL_CODE_NODATA = 127;

const BATTERY_OFFSET_MV = 5000;
const BATTERY_STEP_MV = 50;
const BATTERY_CODE_MAX = 60;
const BATTERY_CODE_NODATA = 63;

export const DOOR_STATES = ['closed', 'open', 'fault', 'unknown'];

export const COMMANDS = {
  systemOn: 0x01,
  systemOff: 0x02,
  doorOpen: 0x04,
  doorClose: 0x08,
  block: 0x10,
  unblock: 0x20
};

const EXCLUSIVE_PAIRS = [
  ['systemOn', 'systemOff'],
  ['doorOpen', 'doorClose'],
  ['block', 'unblock']
];

export function decodeUplink(bytes) {
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
    door: DOOR_STATES[doorCode]
  };
}

export function encodeDownlink(names) {
  if (!Array.isArray(names) || names.length === 0) {
    throw new Error('je potřeba alespoň jeden příkaz');
  }

  const unknown = names.filter((name) => !(name in COMMANDS));
  if (unknown.length > 0) {
    throw new Error(`neznámý příkaz: ${unknown.join(', ')}`);
  }

  for (const [a, b] of EXCLUSIVE_PAIRS) {
    if (names.includes(a) && names.includes(b)) {
      throw new Error(`příkazy ${a} a ${b} se navzájem ruší, kurník by je ignoroval`);
    }
  }

  const byte = names.reduce((acc, name) => acc | COMMANDS[name], 0);

  return Uint8Array.of(byte);
}
