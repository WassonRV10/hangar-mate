# Firmware

Particle firmware for the Muon + M404 (Device OS 6.x).

**Current code:** [`src/hangar_mate.ino`](src/hangar_mate.ino), v0.2.0. It covers relay control with toggle, ARM/SAFE reporting, temperatures from the three DS18B20 probes, the `status` variable, `alloff`, the FR-003a preheat effectiveness check and the 24 h long-run notice. Still to come: CT current confirmation, `setprobe`, and Pushover forwarding (see below).

To flash from the Web IDE: create a new app, paste `src/hangar_mate.ino`, select your device and flash. No libraries are needed.

**Bring-up and probe diagnostics:** [`bringup/onewire_bringup.ino`](bringup/onewire_bringup.ino) is a separate sketch for testing the DS2484 and the probes and for finding their ROM IDs. See [`bringup/README.md`](bringup/README.md).

## Interface (implemented)

| Cloud function | Argument | Action |
| --- | --- | --- |
| `tanis` | `on` / `off` / `toggle` | Energize, de-energize or flip the preheater relay. No timer, no auto-off |
| `cabin` | `on` / `off` / `toggle` | Cabin-heater relay (Phase 2), with a firmware-enforced maximum run time (120 min placeholder, value to be decided). `on` while already on starts a new run-time window |
| `alloff` | none | Turn every controlled output off |

**Return codes** report the resulting state, so a phone can show it directly:

| Code | Meaning |
| --- | --- |
| `1` | Output is now ON |
| `0` | Output is now OFF (`alloff`: all outputs now off) |
| `-1` | Invalid argument |
| `-2` | Refused: ARM/SAFE is SAFE, or not yet read after boot. Applies to every command; outputs stay off |

A `1` means the relay is on, not that current is flowing; the phone labels it "heat not yet confirmed" until CT confirmation exists (PRD IF-003).

| Cloud variable | Values |
| --- | --- |
| `status` | JSON summary, updated on every change, after each temperature read (every 30 s), and at least once a minute |
| `tanisState`, `cabinState` | `on` / `off` |
| `armState` | `ARMED` / `SAFE` / `unknown` |

Example `status`:

```json
{"t":"2026-11-13T12:05Z","tanis":{"st":"COMMANDED_ON","since":"11:30Z"},"cabin":{"st":"OFF","rem":0},"temp":{"eng":67.9,"cab":67.6,"hgr":69.0},"arm":"ARMED","pwr":"VIN","sig":63,"fw":"0.2.0"}
```

- Temperatures are °F. A probe that is unplugged or not answering reads `null`. That is normal for the engine probe when the Tanis cord is unplugged for flight.
- `st` is `COMMANDED_ON` or `OFF` until the CT is installed (PRD 9A.5). `since` and `t` are UTC.
- `rem` is the number of minutes left on the cabin heater's run time.
- `pwr` is `VIN`, `USB`, `BATT` or `unknown`. `sig` is cellular signal strength in %, or `-1` if unknown, refreshed every 5 min.

### Events (private, `hm/` prefix)

| Event | Trigger |
| --- | --- |
| `hm/boot` | Controller started and connected to the cloud |
| `hm/arm` | ARM/SAFE switch changed |
| `hm/cabin` | Cabin heater reached its maximum run time and was turned off |
| `hm/eng/norise` | FR-003a: 30 min after Tanis ON, the engine probe rose less than 5 °F, was not responding, or had no baseline. The message gives baseline, current, rise and hangar temperature |
| `hm/eng/ok` | FR-003a passed. Same message format |
| `hm/tanis/longrun` | Tanis has been ON for more than 24 h. It stays ON |

The FR-003a check runs once per off-to-on change. It is cancelled silently if Tanis is turned off or the switch goes to SAFE first. Neither check ever changes the Tanis relay.

## Probe map

The engine, cabin and hangar probes are identified by ROM ID in `PROBE_ROM` in `src/hangar_mate.ino`. Each build has different probes: find yours with the bring-up sketch and copy them in. The prototype's IDs are in [`bringup/README.md`](bringup/README.md).

## Still to come (PRD Section 9A)

- CT current confirmation (IF-004/IF-005) and the `CONFIRMED_ON` / fault states. Waiting on the CT bias circuit and calibration
- `setprobe` function, to assign probe IDs without reflashing
- Pushover forwarding via a Particle webhook on the `hm/` prefix
- Tune the FR-003a defaults (5 °F at 30 min) from logged heat cycles (PRD Section 25 item 35)

## Pin map (minimum build)

| Function | Muon pin | Header pin | HAT label |
| --- | --- | --- | --- |
| Preheater relay IN | D2 | 11 | IO17 |
| ARM/SAFE sense (10k/15k divider from switched 5 V rail) | D4 | 33 | IO13 |
| DS2484 1-Wire bridge, address 0x18 | I2C (Qwiic) | 3 / 5 | SDA / SCL |
| Cabin-heater relay (Phase 2) | D3 | 36 | IO16 |

The DS18B20 probes are wired in powered mode (VDD, GND and data) to the DS2484's 1-Wire terminal. The DS2484's active pull-up is enabled in firmware, so no external pull-up resistor is needed.

## Rules the firmware must keep

- Drive relay pins LOW in `STARTUP()` and again in `setup()`.
- Read the ARM/SAFE sense pin with `INPUT_PULLDOWN`, so a broken or missing sense wire reads SAFE.
- Never de-energize the Tanis on a timer (PRD TSR-001/002); notify after 24 h instead. The FR-003a check is also notification-only.
- On ARM/SAFE going LOW: drive relays LOW, clear commanded state, publish an event, and refuse every command with `-2`. Re-arming does not restore the prior state.
- Do not block the loop: temperature conversions are started and read on later passes, so ARM/SAFE is still checked during the 800 ms conversion.
- Do not depend on the unmaintained OneWire library or on Device OS 5.9.0. The DS2484 is driven directly through `Wire`.
