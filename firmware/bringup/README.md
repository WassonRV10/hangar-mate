# 1-Wire bring-up sketch

`onewire_bringup.ino` is a standalone diagnostic for the DS2484 I2C-to-1-Wire bridge and the DS18B20 probes. Flash it **instead of** the main firmware when building or troubleshooting the probe bus. It does not control any relays.

It runs these checks automatically and prints them to USB serial (115200), and also reports through the Particle Cloud:

1. I2C scan: the DS2484 should answer at 0x18.
2. DS2484 reset and configuration.
3. 1-Wire ROM search: lists every probe ID, CRC errors and searches that stopped early.
4. Readings every ~3 s, with the spread between probes.
5. Unplugged probes show `DISCONNECTED` by name.

| Cloud item | Name | Use |
| --- | --- | --- |
| Function | `report` | Search again, read, and publish now (argument ignored; returns 0) |
| Variable | `temps` | Latest readings |
| Variable | `search` | Latest search: `n=<count> crcFail=<n> abort=<n> \| <IDs>` |
| Events | `hm/temps`, `hm/search` | Once a minute, and after each `report` |

## Builder tips

- Bench-check each probe **alone** at the DS2484 terminal before building it into a cable assembly. One probe in the prototype's 5-pack was dead out of the box.
- Identify probes by connecting them one at a time, or by warming one probe and watching which reading rises.
- Copy the IDs into `PROBE_MAP` here and into `PROBE_ROM` in the main firmware (`src/hangar_mate.ino`). `PROBE_ROM` takes the 8 bytes only, in ENGINE, CABIN, HANGAR order.

## Prototype probe map (2026-11-12)

| Role | ROM ID |
| --- | --- |
| ENGINE | `2867612500000075` |
| CABIN | `28581724000000C2` |
| HANGAR | `280B522500000001` |
