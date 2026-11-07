# Hangar Mate

Remote preheat control and monitoring for an aircraft in a hangar with no Wi-Fi. A Particle Muon + M404 LTE-M controller switches the engine preheater (Tanis) from an iPhone and reports engine-compartment and hangar temperature, so you can confirm the engine is actually warming before you leave home.

**Status:** prototype. Cellular link, both relays and the ARM/SAFE hardware cutoff are bench-verified. Temperature probes are waiting on the DS2484 I2C-to-1-Wire bridge. Relay-control firmware with a one-tap iPhone toggle is working on the bench; the status readout, alerts and shareable Shortcuts are in progress. The builder's guide is **preliminary — do not build from it yet.**

## Repository layout

| Path | Contents |
| --- | --- |
| `docs/PRD.md` | Project requirements document (v0.19) — the design of record. Section 9A defines the command and notification interface (iPhone Shortcuts + Particle Cloud + Pushover) |
| `docs/builders-guide.md` | Step-by-step guide for the minimum build (preliminary) |
| `presentation/` | EAA chapter presentation (PDF) |
| `firmware/` | Particle firmware — `src/hangar_mate.ino` (relay control, toggle, ARM/SAFE reporting) |

## Minimum build

- Particle Muon carrier + M404 LTE-M module
- One 5 V-trigger opto-isolated relay module (active-high, 15 kΩ pull-down, load on COM→NO)
- One duplex receptacle in a non-metallic electrical enclosure (listed polycarbonate recommended; see the builder's guide for the alternate used on the prototype) — one half switched, one half always on
- Two DS18B20 probes on an Adafruit DS2484 bridge: engine (rides on the preheater cord) and hangar
- Latching ARM/SAFE button in the relay's 5 V feed — SAFE cuts the relay in hardware

## Safety

This device switches 120 V near an aircraft and fuel. The Tanis guide says not to cycle the preheater or put it on a timer or thermostat, and to disconnect it before fueling or engine start. The firmware never turns the Tanis off on a timer, and the remote system does not replace the preflight disconnect. Each builder is responsible for their own installation.

## Reference documents (not stored here — copyrighted by their publishers)

- Tanis TPG0001 piston engine preheat guide (Tanis Aircraft Products)
- Particle [Muon datasheet](https://docs.particle.io/reference/datasheets/m-series/muon-datasheet/)
- AOPA, "Crimes and Misdemeanors" (April 2019) — cold-start engine damage
