# Firmware

Particle firmware for the Muon + M404 (Device OS 6.x). Code to come.

## Planned interface (PRD Section 9A draft)

| Cloud function | Argument | Action |
| --- | --- | --- |
| `tanis` | `on` / `off` | Energize or de-energize the preheater relay. No timer, no auto-off |
| `alloff` | — | De-energize all controlled outlets |
| `cabin` | `on`, `on,<min>`, `off` | Phase 2 — cabin heater with a firmware-enforced maximum run time |

Return codes: `1` changed, `0` already in that state, `-1` invalid argument, `-2` rejected (ARM/SAFE in SAFE).

Cloud variable `status` holds a JSON summary (output states, temperatures, ARM/SAFE, power, signal, firmware version). Events are published private under the `hm/` prefix and forwarded to Pushover by a Particle webhook.

## Pin map (minimum build)

| Function | Muon pin | Header pin | HAT label |
| --- | --- | --- | --- |
| Preheater relay IN | D2 | 11 | IO17 |
| ARM/SAFE sense (10k/15k divider from switched 5 V rail) | D4 | 33 | IO13 |
| DS2484 1-Wire bridge | I2C (Qwiic) | 3 / 5 | SDA / SCL |
| Cabin-heater relay (Phase 2) | D3 | 36 | IO16 |

## Rules the firmware must keep

- Drive relay pins LOW in `STARTUP()` and again in `setup()`.
- Never de-energize the Tanis on a timer (PRD TSR-001/002); notify after 24 h instead.
- On ARM/SAFE going LOW: drive relays LOW, clear commanded state, publish an event, reject "on" commands. Re-arming does not restore the prior state.
- Do not depend on the unmaintained OneWire library or on Device OS 5.9.0.
