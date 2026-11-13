# Hangar Preheat Controller — Builder's Guide

Oct 31, 2026 (updated Nov 13, 2026: full build is now the main guide; firmware v0.2.0) · Brian

**Status: PRELIMINARY — do not distribute yet.** This guide describes the **full build**, which matches Brian's prototype: two opto-isolated relay modules switching two short pigtail cords (preheater and cabin heater) from a non-metallic electrical enclosure (listed polycarbonate recommended), three temperature probes (engine, cabin and hangar), and a latching ARM/SAFE switch that cuts relay power in hardware. Both relays, the ARM/SAFE switch and all three probes are bench-verified on Brian's unit.

A **minimum build** — just the controller and one controllable outlet — is described in Section 13. Its firmware is a separate project and is not written yet.

Release this guide only after these are closed on Brian's bench:

- [x] DS2484 I2C-to-1-Wire bridge reading the probes on the M404 (bench-verified Nov 12, 2026; no library needed)
- [ ] Firmware v0.2.0 features (`status`, `alloff`, "not warming", long-run and cabin run-time alerts) verified on hardware
- [ ] Engine and cabin probes read reliably at full cable length
- [ ] Cabin-heater maximum run time set (firmware placeholder is 120 min)
- [ ] Production firmware published as a shared app, with a version number
- [ ] iPhone Shortcuts published with a token prompt at install
- [x] Controller power supply choice confirmed for builders: 8–12 V DIN-rail supply to VIN (Section 4, item 7)
- [ ] Fill in the remaining bracketed prices and links in Section 4

## 1. What you're building

You'll build a cellular controller that turns your Tanis (or similar plug-in preheater) and a small cabin heater on and off from an iPhone, and shows engine, cabin and hangar temperatures, so you can confirm the airplane is warming before you leave home. It needs no hangar Wi-Fi and no monthly fee.

**How it works:** an iPhone Shortcut calls the Particle Cloud, which reaches the controller over LTE-M cellular. The controller drives two relays, each switching a short pigtail cord: one powers your preheater extension cord, the other powers a cabin heater. Three waterproof probes report back: one taped to the preheater cord so it sits inside the cowl, one in the cabin, and one for hangar air. The engine-minus-hangar difference is your proof of heat.

**What the full build does:**

- Turns the preheater outlet and the cabin-heater outlet on and off from your phone, with the controller's reply showing the state it actually set
- Reports engine-compartment, cabin and hangar temperature on demand
- Sends a push alert if the engine isn't warming 30 minutes after preheat starts, or if the preheater has been on longer than a set time (default 24 h)
- Turns the cabin heater off by itself after a fixed maximum run time, and tells you
- Provides a latching ARM/SAFE button at the box that blocks both outlets in hardware

**What it does not do (yet):** confirm current flow, alert on hangar power loss, or flood and activity alerts. See Section 12.

## 2. Safety rules

You own your installation. This device switches 120 V in a hangar with fuel and an aircraft nearby; follow these rules without exception.

1. **Read your preheater's manual.** Tanis systems are self-regulating; the Tanis guide says not to cycle them and not to use timers or thermostats. The firmware never turns the Tanis off on a timer. Other preheaters may have different rules.
2. **Unplug the preheater before fueling or starting the engine.** The Tanis guide requires it. The remote system does not replace this step — put it on your checklist.
3. **Load through COM and NO only, never NC.** Wired through NC, the outlet is live whenever the controller is off or dead, which inverts every fail-safe in this design.
4. **Relay trigger jumpers on active-high, with the 15 kΩ pull-downs fitted.** In active-low, Brian's outlet energized at power-up before the firmware ran.
5. **Electrical enclosure, 15 A rated cords, wiring and terminal blocks.** A listed polycarbonate enclosure is recommended, because it is tested to contain a 120 V fault near fuel. If you use an unlisted plastic box (see the alternate in Section 4), mount it away from fuel and the aircraft. No homemade wooden boxes.
6. **One plug, one circuit.** Plug the enclosure's cord directly into the hangar receptacle, not into another power strip, and stay within your circuit's capacity.
7. **Use a grounded 12- or 14-gauge outdoor-rated extension cord** for the preheater. Tanis's cord table supports 14-gauge with margin for typical runs; 12-gauge adds more.
8. **Mount everything off the floor** and away from where water collects.
9. **Check your hangar lease and airport rules** on unattended electrical devices before installing.
10. **Cabin heater rules.** The heater runs unattended in a closed airplane, so:
    - Use a listed (UL/ETL) ceramic heater with tip-over and overheat shutoff, and one that starts heating whenever it gets power (no button to press).
    - Leave it on its **lowest setting** whenever it may run unattended, and put a placard on it saying so. Brian's heater has 700 / 900 / 1500 W settings; at 700 W it draws about 5.8 A, about 9.8 A with the Tanis. The higher settings are for attended use only: 1500 W plus the Tanis exceeds what a shared 20 A circuit should carry continuously.
    - Keep it clear of fabric, charts and fuel lines. The firmware's maximum run time is a backstop, not a substitute for the heater's own protections.
    - Remove the heater from the airplane before flight.

## 3. Before you buy: hangar check

Confirm your hangar can support this before ordering parts. Ten minutes on site answers all of it.

1. **Cellular signal.** Ask Brian to bring his controller, or use any LTE phone as a rough first check. The controller uses LTE-M and its SIM picks the carrier automatically (Brian's has used both AT&T and T-Mobile). Check at the spot you plan to mount it, with the hangar door closed. In Brian's metal T-hangar, the best spot was beside the man door.
2. **Outlet.** Find the receptacle you'll use and its breaker size. You need one free receptacle for the enclosure's cord.
3. **Cord routes.** Measure the run from the outlet to your preheater plug on the airplane, and to where the cabin heater will sit (Brian's goes in the baggage compartment through an existing cord opening). These set your cord lengths and the probe cable lengths.
4. **Mounting spot.** Pick a dry spot off the floor near the outlet, with room for the controller enclosure, and a place to anchor a short fall tether.
5. **Preheater.** Note its make, model and current draw from its label or manual.
6. **Phone.** The interface is built on iPhone Shortcuts. There is no Android version yet.

## 4. Bill of materials

Prices are what Brian paid (Nov 2026). Total for items 1–13 is about **$284**, with no monthly fee. The cabin heater, magnets, tether and small supplies are extra.

| # | Part | Qty | Price | Notes |
| --- | --- | --- | --- | --- |
| 1 | [Particle Muon kit, North America (M404 LTE-M/2G)](https://store.particle.io/products/particle-maker-kit) | 1 | $85 | Controller + cellular module. Confirm the kit includes the LTE antenna |
| 2 | 5 V-trigger, 30 A single-channel relay module, opto-isolated input, NO/COM/NC terminals, trigger-polarity jumper | 2-pack | $5 | One for the preheater, one for the cabin heater. Brian used a hiBCTR 2-pack |
| 3 | [Adafruit DS2484 I2C-to-1-Wire bridge (5976)](https://www.adafruit.com/product/5976) + STEMMA QT / Qwiic cable (100 mm or longer) | 1 each | $15 | Probes connect to the bridge; it plugs into the Muon's Qwiic port |
| 4 | Waterproof DS18B20 temperature probes | 3-pack | $6 | Engine, cabin and hangar. Consider a 5-pack: one of Brian's was dead out of the box |
| 5 | 3-conductor 22 AWG cable | 50 ft | $20 | Extends the engine and cabin probes (data, power, ground) |
| 6 | Deutsch connectors, 3-pin | 1 set | $10 | Disconnect between the box-side probe cable and the probe that rides on the preheater cord |
| 7 | Mean Well 12 V DIN-rail power supply | 1 | $14 | Feeds the Muon's VIN (6–12 V). 12 V is the top of that range: adjust the output trimmer down to about 11 V and measure before connecting the Muon. For bench work, a USB-C PD supply and C-to-C cable also work; a USB-A charger will not |
| 8 | 8-output DIN-rail power distribution module | 1 | $15 | More 5 V and GND connection points than the Muon header provides. Low-voltage side only; relay DC+ must still come from the ARM/SAFE switch (6.3) |
| 9 | WAGO DIN-rail terminal blocks | 1 set | $15 | Internal AC wiring connections |
| 10 | 19 mm illuminated latching (push-on/push-off) button (switch rated 1 A+, LED isolated from switch), 40-pin screw-terminal HAT, resistors (15 kΩ ×3, 10 kΩ ×1) | 1 set | $19 | ARM/SAFE switch (a 12 V LED is dimmer at 5 V but visible); no soldering on the header; relay input pull-downs and ARM/SAFE sense divider |
| 11 | Enclosure and 14/3 grounded supply cord with 15 A plug | 1 each | $38 | **Recommended enclosure:** listed (UL/ETL) polycarbonate, NEMA-rated, with cord grips; expect it to cost more than the alternate. **Alternate (what Brian used):** Joinfworld lockable NEMA 4 plastic enclosure, 11.4 × 7.5 × 5.5 in, clear ABS cover, grid plate, DIN rail, 1/2" NPT glands. It is not listed and states no flame rating; Brian accepted that because his box is at the man door, 10+ ft from fuel. Decide for your own hangar. Non-metallic either way, so the antenna works. Needs one gland per cord, plus one for probe cables |
| 12 | 12-ga extension cords, 2 ft (female end) | 2 | $17 | Preheat and cabin-heat pigtails: male plug cut off, enters through a gland, wired to a relay |
| 13 | 12-ga grounded outdoor extension cord | 1 | $25 | Preheater cord. Skip if you already have a 12- or 14-ga grounded outdoor cord |
| 14 | Ceramic space heater, listed, tip-over and overheat shutoff, low setting about 700 W | 1 | [$__] | Must start heating whenever it gets power. See Section 2, rule 10 |
| 15 | Magnets rated for the box weight (in shear), magnetic hook, short tether | 1 set | [$__] | No-drill mounting on steel walls |
| 16 | 14 AWG wire, 22–24 AWG hookup wire, zip ties, electrical tape, labels | — | [$__] | |

**Accounts (free):** Particle (Free Plan: up to 100 devices and 100K data operations per month, cellular data included — [plan details](https://docs.particle.io/notes/plans)) and Pushover for push alerts (one-time app purchase; check current price).

## 5. Tools

- Small flat and Phillips screwdrivers (screw terminals)
- Wire strippers and flush cutters
- Digital multimeter (continuity and DC volts)
- Plug-in receptacle tester (3-light)
- Drill and step bit for the enclosure (button, extra glands)
- Deutsch crimp tool for the contacts you buy
- Computer with Chrome or Edge (device setup and firmware flashing)
- iPhone with the Shortcuts and Pushover apps
- A lamp or other small 120 V load for bench testing (two are handy)

## 6. Step-by-step build

Build and test the low-voltage side first, then the AC side. Keep the enclosure cord unplugged whenever the lid is open.

### 6.1 Set up the controller (30 min)

1. Attach the M404 module and the LTE antenna to the Muon as shown in Particle's quick-start.
2. For bench setup, power the Muon from a USB-C PD supply and C-to-C cable (a USB-A-to-USB-C cable will not work; the Muon negotiates its voltage over USB-C PD), or from the DIN-rail supply (BOM item 7) to VIN once its output is set. VIN accepts 6–12 V.
3. On a computer, go to setup.particle.io, create or sign in to your Particle account, and follow the prompts. Particle recommends this route because it updates Device OS, activates the SIM and turns on the 3V3_AUX supply the header needs.
4. Name the device (e.g., `Hangar_<tail number>`). Confirm it shows online in the Particle Console.

### 6.2 Fit the screw-terminal HAT

1. Power off. Press the HAT onto the Muon's 40-pin header, aligned pin 1 to pin 1.
2. **The HAT's labels use Raspberry Pi numbering, not Particle names.** Use this map:

| Function | Muon pin | Header pin | HAT label |
| --- | --- | --- | --- |
| Preheat relay control out | D2 | 11 | IO17 |
| Cabin-heat relay control out | D3 | 36 | IO16 |
| ARM/SAFE sense in | D4 | 33 | IO13 |
| 5 V (relay power, via ARM/SAFE) | 5V | 2 or 4 | 5V |
| Ground | GND | 6, 9, 14, 20, 25 | GND |

Pins 1 and 17 are 3V3. **Do not power the relay modules from 3V3** — they will not pull in.

### 6.3 Configure and wire the relay modules (low-voltage side)

Do this for both modules. Label them **PREHEAT** and **CABIN**.

1. Set the trigger jumper to **active-high** (high = on).
2. Fit a **15 kΩ resistor from the module's IN terminal to its GND (DC–) terminal.** It holds the relay off before firmware runs or if the control wire comes loose.
3. Wire **D2 (IO17) → PREHEAT IN** and **D3 (IO16) → CABIN IN**.
4. Wire **Muon GND → each module's DC–**.
5. Each module's **DC+** connects to the ARM/SAFE switched rail in 6.4, not directly to 5V.

**If you use the DIN-rail power distribution module** to get more 5 V and GND connections than the Muon header has: it may carry GND and the *unswitched* 5 V that feeds the ARM/SAFE switch, but **relay DC+ must come only from switch lead B**. If a relay's DC+ is on an unswitched 5 V terminal, SAFE will not cut that relay. Keep this module on the low-voltage side, never on the AC blocks. Bench test 5 (Section 8) proves it.

### 6.4 Wire the ARM/SAFE switch

The latching button sits in both relay modules' 5 V feed. Latched (LED lit) = ARMED; released (LED dark) = SAFE, and neither relay coil has power no matter what the firmware does.

1. With the meter, find the two switch leads that close when latched and open when released. Call them A and B. Identify the LED + and – leads.
2. Wire **Muon 5V → switch lead A**.
3. From **switch lead B** (the switched rail), run four wires: to **PREHEAT DC+**, to **CABIN DC+**, to **LED +**, and to a **10 kΩ resistor**.
4. Join the other end of the 10 kΩ to **D4 (IO13)** and to a **15 kΩ resistor to GND**. This divider puts about 3.0 V on D4 when ARMED.
5. Wire **LED – → GND**.

```
Muon 5V (pin 2/4) ── switch A
switch B ──┬── PREHEAT relay DC+
           ├── CABIN relay DC+
           ├── LED +         (LED – to GND)
           └── 10k ──┬── D4 / IO13 (pin 33)
                     └── 15k ── GND
D2 / IO17 (pin 11) ──┬── PREHEAT relay IN
                     └── 15k ── PREHEAT relay DC–
D3 / IO16 (pin 36) ──┬── CABIN relay IN
                     └── 15k ── CABIN relay DC–
Muon GND ─────────────── both relays' DC–
```

The firmware reads D4 continuously: HIGH = ARMED, LOW = SAFE. The pin uses an internal pull-down, so a missing or broken sense wire reads SAFE, never ARMED. On SAFE it drives D2 and D3 low, clears the commanded state and sends an alert. While SAFE it refuses **every** relay command (on, off or toggle) and returns `-2`, so the phone reports "relays not active" instead of pretending the command worked. Re-arming leaves both outlets off until a new command.

### 6.5 Wire the AC side

There are no receptacles in the box. The supply cord comes in through one gland, and two short 12-gauge cords with female ends (the "pigtails") go out through two more. WAGO DIN-rail terminal blocks make the internal hot, neutral and ground connections.

1. Cut the male plugs off both 2 ft 12-gauge cords. Bring each cut end in through its own watertight gland, and the 14/3 supply cord in through another. Check that each gland's clamping range fits the cord's outside diameter, and tighten it on the jacket, not the conductors.
2. Mount three WAGO blocks on the DIN rail: **HOT**, **NEUTRAL** and **GROUND** (use the green/yellow ground block for ground). Check that the blocks are rated for 12 AWG and at least 15 A, and have enough positions for every wire below.
3. **Supply cord:** black → HOT, white → NEUTRAL, green → GROUND.
4. **HOT → PREHEAT relay COM** and **HOT → CABIN relay COM.** **PREHEAT NO → preheat pigtail black. CABIN NO → cabin pigtail black.** Leave NC empty on both. Confirm the relays' screw terminals accept 12 AWG.
5. **Both pigtails' white → NEUTRAL. Both pigtails' green → GROUND.**
6. Label the female ends **"PREHEAT — SWITCHED"** and **"CABIN HEAT — SWITCHED."**
7. Mount the relay modules on standoffs with their AC terminals away from the controller and antenna. Keep AC and low-voltage wires on separate sides of the box.
8. Controller power: mount the DIN-rail supply. Wire its AC input from **HOT** and **NEUTRAL** (upstream of the relays, so it is always powered) and its ground terminal to **GROUND**. Before connecting the Muon, power the supply, turn its output down to about 11 V and measure it. Then wire DC+ to the Muon's VIN and DC– to GND.
9. With the lid closed and the cord plugged in, check each pigtail with the receptacle tester: dark when SAFE or commanded off, "correct wiring" when on.

### 6.6 Connect the temperature probes

1. Plug the Qwiic cable from the Muon's Qwiic connector to the DS2484 board.
2. **Check each probe on its own first.** Connect one probe at a time to the DS2484 terminals and confirm it is found and reads room temperature with the bring-up sketch (Section 7.1). Do this before splicing or adding connectors. One probe in Brian's 5-pack was dead out of the box.
3. Connect each probe's three wires to the DS2484 screw terminals: data, power and ground. On Brian's DROK probes, yellow = data, red = power, black = ground; confirm against your probe's documentation and Adafruit's DS2484 pinout page.
4. All three probes share the same three terminals. The firmware tells them apart by each probe's factory ID (Section 7).
5. Put a label flag on each probe: ENGINE, CABIN and HANGAR.

### 6.7 Extend the engine and cabin probes

1. Extend the ENGINE probe with the 3-conductor 22 AWG cable, long enough to follow the preheater cord from the enclosure to the cowl: one conductor each for data, power and ground. Solder and heat-shrink the splices.
2. Extend the CABIN probe the same way, long enough to follow the cabin-heater cord to where the heater sits.
3. **Test both at full length on the bench before installing** (Section 8, test 9).
4. Put a 3-pin **Deutsch connector** in the engine probe cable near the enclosure end of the preheater cord. The cord and its probe can then be unplugged and taken away together, leaving the box-side cable in the hangar. Use the crimp tool made for the contacts, and add the sealing plugs and wedge lock. If you remove the cabin cord for flight, give the cabin probe a connector too.
5. Tape or zip-tie the engine probe tip 4–6 inches back from the preheater cord's plug end, so it sits inside the cowl when the cord is plugged in but doesn't touch hot parts. Secure the cable along the cord every 1–2 feet so the two always move together.
6. Tape the cabin probe to the cabin-heater cord so it sits in the cabin air, not in the heater's outlet stream.

### 6.8 Final assembly

1. Mount the Muon, HAT and DS2484 at the end of the enclosure away from the relays and AC wiring. Mount the ARM/SAFE button where it's easy to reach.
2. Put the antenna on the top or side facing the hangar door, at least 1–2 inches from any steel once mounted.
3. Bring probe cables out through a gland.
4. Label the box: "Preheat controller — SAFE (button out) = both outlets off. Unplug the preheater before fueling or engine start."

## 7. Firmware and iPhone Shortcuts

You don't write code. You find your probes' IDs with a test sketch, copy them into Brian's firmware, flash it, and install the Shortcuts.

The firmware is in this repository: `firmware/src/hangar_mate.ino` (currently v0.2.0). A shared Web IDE link will be published **[link to be published]**. Until the planned `setprobe` function exists, probe IDs are set in the firmware itself, so you edit one table before flashing.

### 7.1 Find your probe IDs

1. In the Particle Web IDE, create a new app, paste `firmware/bringup/onewire_bringup.ino`, select your device and flash.
2. Open USB serial (115200), or read the `search` variable in the Console. The sketch lists every probe it finds by its 16-character factory ID and confirms the DS2484 at address 0x18. See `firmware/bringup/README.md`.
3. Connect the probes one at a time, or warm one in your hand and watch which reading rises, to tell ENGINE, CABIN and HANGAR apart. Write each ID on its label flag.

### 7.2 Set the probe IDs and flash the firmware

1. Create another app and paste `firmware/src/hangar_mate.ino`.
2. Find `PROBE_ROM` near the top. Replace the three rows with your IDs, two hex digits per byte (ID `2867612500000075` becomes `{0x28,0x67,0x61,0x25,0x00,0x00,0x00,0x75}`). Keep the order ENGINE, CABIN, HANGAR.
3. Flash. In the Console, confirm the device publishes an `hm/boot` event.
4. Read the `status` variable and confirm `eng`, `cab` and `hgr` all show plausible °F values. A probe that isn't answering shows `null`; recheck its wiring and ID.

The cabin heater's maximum run time is `CABIN_MAX_ON_MS` near the top of the firmware (120 min placeholder). Each new ON command starts a new window.

### 7.3 Set up push alerts

1. Install Pushover on your iPhone and create an account; note your user key and create an application token.
2. In the Particle Console → Integrations, add a webhook for event prefix `hm/` that posts to Pushover, using the template Brian publishes **[link]**. Your Pushover keys live in the webhook, not in the firmware.
3. Trigger a test (unlatch and re-latch ARM/SAFE with an outlet on) and confirm a push arrives.

### 7.4 Install the Shortcuts

1. Create a dedicated Particle access token for the Shortcuts only, so you can revoke it without affecting anything else. Keep it private.
2. Install the shared Shortcuts **[links]**. Each one asks for your device ID and token at install; they are never embedded in a shared link.
3. You get: **Preheat** (one button that toggles the preheater), **Cabin Heat** (the same for the cabin heater), **Hangar Status** and **All Off**. Preheat and Cabin Heat read the controller's reply to the command itself, so what they show is the state the controller actually set. "ON" means the relay is on; it does not yet prove current is flowing. That confirmation comes with the current sensor (PRD Section 9A, IF-003/IF-004).
4. Optional: add Siri phrases (e.g., "Preheat the airplane"). Because they toggle, always read the reply: ON or OFF.
5. **Never re-run Preheat or Cabin Heat just because it said "No response from hangar."** The command may have reached the controller even though the reply didn't reach you, and a second toggle would undo it. Run Hangar Status first (or check `tanisState` / `cabinState` in the Console), then decide.

| Shortcut | Cloud call | What you see |
| --- | --- | --- |
| Preheat | `tanis` = `toggle` | "Tanis ON — heat not yet confirmed", "Tanis OFF", "Relays NOT active" or "No response from hangar" |
| Cabin Heat | `cabin` = `toggle` | Same as Preheat, for the cabin heater |
| Hangar Status | read `status` | Both outlet states, temps, engine-minus-hangar, cabin heat minutes left, ARM/SAFE, power source, signal |
| All Off | `alloff` | "All outlets OFF" (`0`) or "Relays NOT active" (`-2`; outlets are off either way) |

**What the controller returns** (`return_value` in the reply to a function call):

| Value | Meaning | Phone shows |
| --- | --- | --- |
| `1` | Output is now ON | Tanis ON — heat not yet confirmed |
| `0` | Output is now OFF | Tanis OFF |
| `-2` | Refused: ARM/SAFE is SAFE (or not yet read after boot); outputs stay off | Relays NOT active — arm switch is off |
| `-1` | Bad argument | Treated as no valid response |
| missing | Controller offline, cloud timeout or token problem | No response from hangar, plus Particle's error text |

It never shows an old reading as current.

#### Building the Preheat Shortcut yourself

Until the shared Shortcuts are published, build it in the Shortcuts app:

1. **Get Contents of URL.** URL `https://api.particle.io/v1/devices/<device name>/tanis`. Tap *Show More*: Method **POST**; Header `Authorization` = `Bearer <token>`; Request Body **Form**, field `arg` = `toggle`. (This one action does both GET and POST; the Method setting decides which.)
2. **Get Dictionary Value**: *Value* for key `return_value` in *Contents of URL*. Function calls (POST) return `return_value`; variable reads (GET) return `result` instead, so don't read a variable here.
3. **If** *Dictionary Value* **is** `1` → **Show Notification** "Tanis ON — heat not yet confirmed". End If.
4. **If** is `0` → "Tanis OFF". End If.
5. **If** is `-2` → **Show Alert** "Relays NOT active — arm switch is off. Nothing changed." End If.
6. **If** (set to **All** conditions) *Dictionary Value* **is not** `1`, **is not** `0`, **is not** `-2` → **Show Alert** "No response from hangar: " followed by the *Contents of URL* variable. End If. This also catches an empty value, which is what an offline controller produces.

Don't add a retry or a Repeat around step 1: toggle is not idempotent. For **Cabin Heat**, duplicate it, change `tanis` to `cabin` in the URL, and change "Tanis" to "Cabin heat" in the messages. If your phone itself has no connection, Shortcuts stops at step 1 with its own error, which still tells you it failed.

## 8. Bench test checklist

These are the tests Brian's prototype passed. Plug a lamp into each pigtail. Pass every test before taking it to the hangar.

- [ ] **1. Outlet wiring.** Lid closed, cord plugged in, ARMED, each outlet commanded on: the 3-light tester in each pigtail shows correct wiring. Commanded off: pigtail dead. Each pigtail follows only its own command.
- [ ] **2. 3.3 V trigger.** ARMED. Run Preheat ten times and Cabin Heat ten times: each relay switches cleanly every time and the phone alternates ON / OFF, matching its lamp.
- [ ] **3. Power-up stays off.** Remove and restore controller power five times. Neither lamp ever lights.
- [ ] **4. Reset and update stay off.** Press the Muon's reset button, then reflash the firmware over cellular. Neither lamp lights.
- [ ] **5. SAFE blocks in hardware.** With both lamps on, release the button to SAFE: both lamps go out immediately, the button LED goes dark, and a SAFE alert arrives.
- [ ] **6. Commands rejected while SAFE.** Run Preheat and Cabin Heat: phone shows "Relays NOT active," lamps stay off. Read `armState` in the Console: SAFE. (If it reads ARMED with the button released, D4 isn't seeing the switch; fix before going on.)
- [ ] **7. Re-arm stays off.** Latch to ARMED: both lamps stay off until you send new commands.
- [ ] **8. Boot while SAFE.** With the button released, power-cycle the controller: lamps stay off and status reports SAFE. Then measure D4 with both relays energized: it should read at least 2.4 V.
- [ ] **9. Probes at full length.** With the engine and cabin probes on their full-length cables, Hangar Status shows all three temps. Warm each probe by hand; its reading rises within a minute. No missing readings over 30 minutes. Unplug the engine probe's Deutsch connector: `eng` shows `null` and the others keep reading.
- [ ] **10. Unreachable state.** Remove controller power and wait a minute. Preheat shows "No response from hangar" with Particle's error text; Hangar Status shows no current reading.
- [ ] **11. Cabin run-time limit.** Turn the cabin outlet on and leave it. At the maximum run time (120 min unless you changed it) the lamp goes off by itself and an `hm/cabin` alert arrives. To test faster, temporarily set `CABIN_MAX_ON_MS` to 2 minutes, then set it back and reflash.

## 9. Hangar installation and soak test

1. **Mount the enclosure** at the spot you chose in Section 3, off the floor, with the antenna at least 1–2 inches from steel. Brian mounts his beside the man door, next to the light switch. On a steel wall, magnets work; size them for sliding (shear) load, which is much lower than the rated pull. Add a short tether so the box can't fall.
2. **Plug the enclosure cord directly into the hangar receptacle.**
3. **Plug the preheater extension cord into the PREHEAT pigtail and the cabin-heater cord into the CABIN HEAT pigtail.** Double-check the labels.
4. **Cabin heater:** place it on a stable, flat spot, set to its lowest setting, with its placard on. Brian's sits in the baggage compartment.
5. **Strain relief:** loop the cords and probe cables through a hook or cleat below the box, so a pulled cord loads the hook, not the box.
6. **Hangar probe:** place it about chest height, away from doors, sunlight and the airplane's heat.
7. **Signal check in place:** with the door closed, confirm the device is online and run Hangar Status from home or the parking lot.
8. **Soak test, 24–48 hours:** leave it running with the door closed. Pass = no disconnects that last more than a few minutes, every Shortcut answers, no unexpected restarts in the Console event log.
9. **First real preheat:** run Preheat (confirm it says Tanis ON) with the cord plugged into the airplane. Note how fast engine-minus-hangar rises. After the first few cycles, set the "not warming" alert from what you saw: `EFFECT_CHECK_MS` (default 30 min) and `EFFECT_MIN_RISE_F` (default 5 °F) near the top of `hangar_mate.ino`, then reflash.

## 10. Daily use and preflight

**When you leave the airplane:** plug the preheater cord into the preheater plug (the engine probe goes into the cowl with it) and put the cabin heater in place. Leave the button ARMED if you want remote heat; SAFE if you don't.

**Before a flight:**

1. Run **Preheat** several hours ahead and confirm the reply is **Tanis ON**. It toggles, so if it says OFF it was already on; run it again. Tanis recommends about six hours or longer for full benefit; overnight is fine.
2. Run **Cabin Heat** within the cabin heater's maximum run time of your arrival (for example, as you leave home). It turns itself off at the limit.
3. Run **Hangar Status** after 30–60 minutes and look for engine temperature pulling away from hangar temperature. The probe reads cowl air, not metal; watch the trend.
4. At the airport: **unplug the preheater cord from the airplane before fueling or engine start** (the engine probe comes out with it), and remove the cabin heater.
5. Run **All Off**, or press the button to SAFE.

**If the flight is canceled:** run All Off. If you forget, the cabin heater turns itself off at its limit, and you'll get a long-run alert for the preheater at 24 hours. The firmware never turns the Tanis off by itself.

## 11. Troubleshooting

Change one thing at a time and recheck.

| Symptom | Likely cause | Check |
| --- | --- | --- |
| Muon won't power up | Wrong supply or setting | Measure the DIN-rail output: VIN needs 6–12 V. On the bench, USB-C needs a true PD supply and C-to-C cable |
| "No response from hangar" | No signal, no power, device offline, or token / device-name error | Read Particle's error text in the alert. Console shows online? Power at the supply? Re-run signal check at the box |
| A Shortcut shows "Relays NOT active" | Button released, or a loose switched-rail or sense wire | Latch the button (LED lit); check 5 V at each relay DC+; check D4 reads about 3 V; `armState` should read ARMED |
| Shortcut says ON/OFF but the relay doesn't follow | Old firmware, or Shortcut not reading `return_value` | Flash current firmware; Shortcut step 2 must read `return_value` from the POST reply |
| Relay doesn't click | Module DC+ on 3V3, jumper on active-low, or SAFE | DC+ must come from 5V through the switch; jumper active-high |
| Relay clicks but pigtail is dead | Load on NC, or loose NO or WAGO connection | Wiring per 6.5; receptacle tester |
| Outlet is on at power-up or when it should be off | Load wired through NC, or jumper on active-low | Fix immediately: COM → NO, active-high, 15 kΩ pull-down fitted |
| Wrong outlet switches | D2 / D3 or the pigtails swapped | Recheck 6.3 and 6.5 against the labels |
| Cabin heater turned itself off | Maximum run time reached (`hm/cabin` alert) | Normal. Run Cabin Heat again for a new window |
| A probe missing or reads nonsense | Loose terminal, wire order, or cable length | Recheck data / power / ground; test the probe without its extension |
| Engine temp doesn't rise | Cord not seated at the preheater plug, preheater fault, or probe outside the cowl | Check the preheater's own indicator; reseat; check probe position |
| No push alerts | Webhook or Pushover keys | Console → Integrations → webhook log for errors |

## 12. Expansion options

Each add-on uses spare controller pins and leaves the full build unchanged. Guides for each come later.

| Add-on | Hardware | Notes |
| --- | --- | --- |
| Current confirmation | Split-core current sensor (SCT-013-030) clamped on the switched hot lead inside the box, bias circuit, analog pin A6 (A2 for the cabin heater) | Gives "confirmed on" in seconds and catches a welded relay |
| Power-loss alert | Muon-compatible 3.7 V LiPo with temperature sensor (3-pin JST-PH) | 2-pin batteries without a thermistor are not compatible |
| Flood warning | Public river-gauge alerts + floor water sensor on D5 | River gauges give the lead time; the floor sensor is backup |
| Activity / lights | Motion sensor on D6, light sensor on the Qwiic bus | Later phase |

## 13. Minimum build

The minimum build is just the controller and one controllable outlet: remote on/off for the preheater, nothing else. It is the cheapest way to start, and everything in it is reused if you later go to the full build.

**What you give up:** no temperatures (so no proof the engine is warming and no "not warming" alert), no cabin heat, and **no ARM/SAFE switch** — the only local cutoff is unplugging the enclosure's cord from the hangar receptacle. Make that part of your fueling and engine-start routine.

**Parts (Section 4):** items 1 (Muon kit), 2 (relay module; one used), 7 (DIN-rail supply), 9 (WAGO blocks), 10 (screw-terminal HAT and resistors only: one 15 kΩ for the relay pull-down, plus one 10 kΩ and one 15 kΩ for D4 — no button), 11 (enclosure and supply cord), 12 (one pigtail) and 13 (preheater cord) if needed.

**Build:** follow Sections 6.1–6.5 for the PREHEAT relay and pigtail only, with these changes:

- Wire **Muon 5V directly to relay DC+**, with no switch.
- **Firmware:** a separate minimum-build firmware is planned as its own project and is not written yet. Until it exists, you can run the full firmware (v0.2.0): wire the 10 kΩ / 15 kΩ divider from 5V to D4 so it always reads ARMED, and set all three `PROBE_ROM` rows to zeros. Expect an "engine sensor not responding" alert 30 minutes after every preheat, and `null` temperatures in Hangar Status.
- Skip 6.6–6.8's probe steps. Label the box: "Preheat controller — unplug this cord to turn the outlet off."
- Bench tests 1–4 and 10 apply. Install per Section 9, steps 1–3 and 7–8.

All rules in Section 2 still apply except those about the cabin heater and ARM/SAFE.

## Appendix A. Pin map and open items

| Muon pin | Header pin | HAT label | Full build | Reserved for |
| --- | --- | --- | --- | --- |
| D0 / D1 (I2C) | 3 / 5 | SDA / SCL | DS2484 via Qwiic connector | Light sensor (same bus) |
| D2 | 11 | IO17 | Preheat relay IN (15 kΩ pull-down to relay GND) | — |
| D3 | 36 | IO16 | Cabin-heat relay IN (15 kΩ pull-down to relay GND) | — |
| D4 | 33 | IO13 | ARM/SAFE sense, 10k/15k divider from switched rail | — |
| D5 | 32 | — | — | Flood sensor |
| D6 | 12 | — | — | Motion sensor |
| A6 | 24 | — | — | Tanis current sensor |
| A2 | 26 | — | — | Cabin-heater current sensor |
| 5V | 2, 4 | 5V | ARM/SAFE switch → both relays' DC+ and LED | — |
| 3V3 | 1, 17 | 3V3 | Not used for the relays | Sensor supply |
| GND | 6, 9, 14, 20, 25, 30, 34, 39 | GND | Relay DC–, LED –, divider | — |

Avoid header pins 13, 15, 16 and 18 (boot-sensitive), 19, 21 and 23 (used internally for SPI), and 27–28 (not connected). All Muon GPIO is 3.3 V only and not 5 V tolerant.

**Open items before release** (owner: Brian)

- [x] DS2484 bridge reads all three probes on the M404 (Device OS 6.x), driven directly through `Wire` with no library (Nov 12, 2026)
- [ ] Verify firmware v0.2.0 on hardware: `status`, `alloff`, the "not warming" check, the long-run alert and the cabin run-time limit
- [ ] Confirm engine- and cabin-probe reliability at the longest expected cable length
- [ ] Set the cabin heater's maximum run time (PRD Section 25 item 38)
- [x] Controller power supply for builders: 8–12 V DIN-rail supply to VIN
- [x] DIN-rail supply: Mean Well 12 V, $14
- [x] Firmware source published in this repository (v0.2.0), with the alert threshold as a firmware setting
- [ ] Publish a shared firmware link, the `setprobe` command, the Pushover webhook template and the Shortcuts (including Cabin Heat)
- [ ] Minimum-build firmware (separate project); decide whether it reads D4 at all, which would remove the D4 divider from the minimum build
- [ ] Fill in bracketed prices; confirm the Muon kit contents
- [ ] Decide whether Android users get a front end

## Sources

- Tanis TPG0001 piston preheat guide
- Hangar Remote Monitoring & Preheat Control System PRD v0.21, including Section 9A (`docs/PRD.md`)
- [Muon datasheet](https://docs.particle.io/reference/datasheets/m-series/muon-datasheet/) · [Particle plans](https://docs.particle.io/notes/plans) · [Particle store](https://store.particle.io/products/particle-maker-kit)
- [Adafruit DS2484 (5976)](https://www.adafruit.com/product/5976)
