# Hangar Mate

Remote preheat control and monitoring for an aircraft in a hangar with no Wi-Fi. A Particle Muon + M404 LTE-M controller switches the engine preheater (Tanis) from an iPhone and reports engine-compartment and hangar temperature, so you can confirm the engine is actually warming before you leave home.

**Status:** prototype. Cellular link, both relays, the ARM/SAFE hardware cutoff and the temperature probes (three DS18B20s on the DS2484 I2C-to-1-Wire bridge) are bench-verified. Firmware v0.2.0 adds the `status` readout, `alloff`, the preheat effectiveness check and the 24 h long-run notice; these are implemented but not yet verified on hardware. Still to come: current-sensor (CT) confirmation, Pushover alert forwarding and shareable Shortcuts. The builder's guide is **preliminary — do not build from it yet.**

## Repository layout

| Path | Contents |
| --- | --- |
| `docs/PRD.md` | Project requirements document (v0.21) — the design of record. Section 9A defines the command and notification interface (iPhone Shortcuts + Particle Cloud + Pushover) |
| `docs/builders-guide.md` | Step-by-step guide for the full build, with a minimum build in Section 13 (preliminary) |
| `presentation/` | EAA chapter presentation (PDF) |
| `firmware/` | Particle firmware — `src/hangar_mate.ino` v0.2.0 (relay control with toggle, ARM/SAFE lockout, temperatures, `status`, `alloff`, preheat-check and long-run events) and `bringup/` (probe diagnostic sketch) |
| `.github/workflows/` | Compiles the firmware on every push; flashes Hangar_Mate only when run by hand |

## Full build (matches the prototype)

- Particle Muon carrier + M404 LTE-M module
- Two 5 V-trigger opto-isolated relay modules (active-high, 15 kΩ pull-down, load on COM→NO): preheater and cabin heater
- Non-metallic electrical enclosure (listed polycarbonate recommended; see the builder's guide for the alternate used on the prototype) with two 12-ga switched pigtail cords through watertight glands, WAGO DIN-rail terminal blocks, and a DIN-rail 12 V supply
- Three DS18B20 probes on an Adafruit DS2484 bridge: engine (rides on the preheater cord), cabin and hangar
- Latching ARM/SAFE button in both relays' 5 V feed — SAFE cuts both relays in hardware
- Listed ceramic cabin heater on its lowest setting, with a firmware-enforced maximum run time

## Minimum build

Just the controller and one controllable outlet for the preheater: no probes, no cabin heat, no ARM/SAFE switch. Its own firmware is a separate project and is not written yet. See Section 13 of the builder's guide.

## Safety

This device switches 120 V near an aircraft and fuel. The Tanis guide says not to cycle the preheater or put it on a timer or thermostat, and to disconnect it before fueling or engine start. The firmware never turns the Tanis off on a timer, and the remote system does not replace the preflight disconnect. Each builder is responsible for their own installation.

## Reference documents (not stored here — copyrighted by their publishers)

- Tanis TPG0001 piston engine preheat guide (Tanis Aircraft Products)
- Particle [Muon datasheet](https://docs.particle.io/reference/datasheets/m-series/muon-datasheet/)
- AOPA, "Crimes and Misdemeanors" (April 2019) — cold-start engine damage
