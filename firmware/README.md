# Firmware

Particle firmware for the Muon + M404 (Device OS 6.x).

**Current code:** [`src/hangar_mate.ino`](src/hangar_mate.ino) — relay control with toggle and ARM/SAFE reporting. Bench firmware, not yet the production release: temperature probes, the `status` variable, `alloff` and alerts are still to come (see below).

To flash from the Web IDE: create a new app, paste `src/hangar_mate.ino`, select your device and flash. No libraries are needed.

## Interface (implemented)

| Cloud function | Argument | Action |
| --- | --- | --- |
| `tanis` | `on` / `off` / `toggle` | Energize, de-energize or flip the preheater relay. No timer, no auto-off |
| `cabin` | `on` / `off` / `toggle` | Cabin-heater relay (Phase 2), with a firmware-enforced maximum run time (120 min placeholder, value to be decided) |

**Return codes** report the resulting state, so a phone can show it directly:

| Code | Meaning |
| --- | --- |
| `1` | Output is now ON |
| `0` | Output is now OFF |
| `-1` | Invalid argument |
| `-2` | Refused: ARM/SAFE is SAFE, or not yet read after boot. Applies to every command (on, off, toggle); outputs stay off |

This replaces the earlier draft definition (`1` changed, `0` already in that state). The PRD Section 9A draft still shows the old definition and needs the same update.

| Cloud variable | Values |
| --- | --- |
| `tanisState`, `cabinState` | `on` / `off` |
| `armState` | `ARMED` / `SAFE` / `unknown` |

Events (private, `hm/` prefix): `hm/arm` on every ARM/SAFE change, `hm/cabin` when the cabin heater hits its run-time limit.

## Still to come (planned in PRD Section 9A)

- `alloff` function
- `status` variable: JSON summary (output states, temperatures, ARM/SAFE, power, signal, firmware version)
- DS18B20 temperatures through the DS2484 I2C bridge (waiting on bench test), and the `setprobe` function
- 24-hour Tanis long-run notification and "not warming" alert
- Pushover forwarding via a Particle webhook on the `hm/` prefix

## Pin map (minimum build)

| Function | Muon pin | Header pin | HAT label |
| --- | --- | --- | --- |
| Preheater relay IN | D2 | 11 | IO17 |
| ARM/SAFE sense (10k/15k divider from switched 5 V rail) | D4 | 33 | IO13 |
| DS2484 1-Wire bridge | I2C (Qwiic) | 3 / 5 | SDA / SCL |
| Cabin-heater relay (Phase 2) | D3 | 36 | IO16 |

## Rules the firmware must keep

- Drive relay pins LOW in `STARTUP()` and again in `setup()`.
- Read the ARM/SAFE sense pin with `INPUT_PULLDOWN`, so a broken or missing sense wire reads SAFE.
- Never de-energize the Tanis on a timer (PRD TSR-001/002); notify after 24 h instead.
- On ARM/SAFE going LOW: drive relays LOW, clear commanded state, publish an event, and refuse every command with `-2`. Re-arming does not restore the prior state.
- Do not depend on the unmaintained OneWire library or on Device OS 5.9.0.
