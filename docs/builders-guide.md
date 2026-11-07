# Hangar Preheat Controller — Builder's Guide (Minimum Build)

Oct 31, 2026 · Brian

**Status: PRELIMINARY — do not distribute yet.** This guide describes the minimum build of Brian's prototype: one opto-isolated relay module, one switched receptacle in a non-metallic electrical enclosure (listed polycarbonate recommended), an engine probe and a hangar probe, and a latching ARM/SAFE switch that cuts relay power in hardware. The relay and ARM/SAFE design are bench-verified on Brian's unit.

Release this guide only after these are closed on Brian's bench:

- [ ] DS2484 I2C-to-1-Wire bridge received and reading the probes on the M404 (part on order)
- [ ] Engine probe reads reliably at full cable length (controller to cowl)
- [ ] Production firmware published as a shared app, with a version number
- [ ] iPhone Shortcuts published with a token prompt at install
- [x] Controller power supply choice confirmed for builders: 8–12 V DIN-rail supply to VIN (Section 4, item 8)
- [ ] Fill in the bracketed prices and links in Section 4

## 1. What you're building

You'll build a cellular controller that turns your Tanis (or similar plug-in preheater) on and off from an iPhone and shows engine and hangar temperatures, so you can confirm the engine is warming before you leave home. It needs no hangar Wi-Fi and no monthly fee.

**How it works:** an iPhone Shortcut calls the Particle Cloud, which reaches the controller over LTE-M cellular. The controller drives a relay that switches a receptacle powering your preheater extension cord. Two waterproof probes report back: one taped to the extension cord so it sits inside the cowl, one for hangar air. The engine-minus-hangar difference is your proof of heat.

**What the minimum build does:**

- Turns the preheater outlet on and off from your phone, with a confirmation that the controller received the command
- Reports engine-compartment and hangar temperature on demand
- Sends a push alert if the engine isn't warming, or if the preheater has been on longer than a set time (default 24 h)
- Provides a latching ARM/SAFE button at the box that blocks the outlet in hardware

**What it does not do (yet):** cabin heat, confirm current flow, alert on hangar power loss, or flood and activity alerts. See Section 12.

## 2. Safety rules

You own your installation. This device switches 120 V in a hangar with fuel and an aircraft nearby; follow these rules without exception.

1. **Read your preheater's manual.** Tanis systems are self-regulating; the Tanis guide says not to cycle them and not to use timers or thermostats. The firmware never turns the Tanis off on a timer. Other preheaters may have different rules.
2. **Unplug the preheater before fueling or starting the engine.** The Tanis guide requires it. The remote system does not replace this step — put it on your checklist.
3. **Load through COM and NO only, never NC.** Wired through NC, the outlet is live whenever the controller is off or dead, which inverts every fail-safe in this design.
4. **Relay trigger jumper on active-high, with the 15 kΩ pull-down fitted.** In active-low, Brian's outlet energized at power-up before the firmware ran.
5. **Electrical enclosure, listed receptacle, 15 A rated cord and wiring.** A listed polycarbonate enclosure is recommended, because it is tested to contain a 120 V fault near fuel. If you use an unlisted plastic box (see the alternate in Section 4), mount it away from fuel and the aircraft. No homemade wooden boxes.
6. **One plug, one circuit.** Plug the enclosure's cord directly into the hangar receptacle, not into another power strip, and stay within your circuit's capacity.
7. **Use a grounded 14-gauge outdoor-rated extension cord** for the preheater. Tanis's cord table supports this gauge with margin for typical runs.
8. **Mount everything off the floor** and away from where water collects.
9. **Check your hangar lease and airport rules** on unattended electrical devices before installing.
10. **Don't add an unattended cabin heater** to the minimum build. Cabin heat needs its own relay, a timeout and a low setting (Section 12).

## 3. Before you buy: hangar check

Confirm your hangar can support this before ordering parts. Ten minutes on site answers all of it.

1. **Cellular signal.** Ask Brian to bring his controller, or use any LTE phone as a rough first check. The controller uses LTE-M and its SIM picks the carrier automatically (Brian's has used both AT&T and T-Mobile). Check at the spot you plan to mount it, with the hangar door closed. In Brian's metal T-hangar, the best spot was beside the man door.
2. **Outlet.** Find the receptacle you'll use and its breaker size. You need one free receptacle for the enclosure's cord.
3. **Cord route.** Measure the run from the outlet to your preheater plug on the airplane. That sets your extension-cord length and the engine-probe cable length.
4. **Mounting spot.** Pick a dry spot off the floor near the outlet, with room for the controller enclosure, and a place to anchor a short fall tether.
5. **Preheater.** Note its make, model and current draw from its label or manual.
6. **Phone.** The interface is built on iPhone Shortcuts. There is no Android version yet.

## 4. Bill of materials

Estimated total is roughly $200–250 with no monthly fee; firm it up once the bracketed items are priced. Prices checked Oct 2026 where linked.

| # | Part | Qty | Approx. price | Notes |
| --- | --- | --- | --- | --- |
| 1 | [Particle Muon kit, North America (M404 LTE-M/2G)](https://store.particle.io/products/particle-maker-kit) | 1 | $79.95 | Controller + cellular module. Confirm the kit includes the LTE antenna |
| 2 | 5 V-trigger, 30 A single-channel relay module, opto-isolated input, NO/COM/NC terminals, trigger-polarity jumper | 1 | [$__] | Brian used a hiBCTR 2-pack; the spare covers cabin heat later |
| 3 | [Adafruit DS2484 I2C-to-1-Wire bridge (5976)](https://www.adafruit.com/product/5976) | 1 | $7.50 | Probes connect here; plugs into the Qwiic port |
| 4 | STEMMA QT / Qwiic cable, 100 mm or longer | 1 | [$__] | Muon Qwiic port to DS2484 |
| 5 | Waterproof DS18B20 temperature probes | 2 (+1 spare) | [$__] | Brian used a DROK 5-pack |
| 6 | Twisted-pair cable (Cat5e or similar) | cord length | [$__] | Extends the engine probe to the cowl |
| 7 | 19 mm illuminated latching (push-on/push-off) button, switch rated 1 A+, LED isolated from switch | 1 | [$__] | ARM/SAFE switch. A 12 V LED is dimmer at 5 V but visible |
| 8 | DIN-rail AC-DC power supply, adjustable 8–12 V DC output | 1 | [$__] | Feeds the Muon's VIN (6–12 V). For bench work before the box is built, a USB-C PD supply and C-to-C cable also work; a USB-A charger will not |
| 9 | 40-pin screw-terminal breakout HAT | 1 | [$__] | No soldering on the header |
| 10 | Resistors, 1/4 W: 15 kΩ (×2), 10 kΩ (×1) | — | <$1 | Relay input pull-down; ARM/SAFE sense divider |
| 11 | **Recommended:** listed (UL/ETL) polycarbonate electrical enclosure, NEMA-rated, with cord grips | 1 | [$__] | Non-metallic so the antenna works. **Alternate (what Brian used):** Joinfworld lockable NEMA 4 plastic enclosure, 11.4 × 7.5 × 5.5 in, clear ABS cover, grid plate, DIN rail, 1/2" NPT glands, about $38. It is not listed and states no flame rating; Brian accepted that because his box is at the man door, 10+ ft from fuel. Decide for your own hangar. |
| 12 | Listed 15 A duplex receptacle (tamper-resistant fine) | 1 | [$__] | Hot-side tab broken: one half switched, one always on |
| 13 | Supply cord, 14/3 grounded, 15 A plug | 1 | [$__] | Enclosure inlet |
| 14 | 14 AWG wire, listed lever or wire-nut connectors | — | [$__] | Internal AC wiring |
| 15 | Magnets rated for the box weight (in shear), magnetic hook, short tether | 1 set | [$__] | No-drill mounting on steel walls |
| 16 | 14-ga grounded outdoor extension cord | 1 | [$__] | Skip if you already have one |
| 17 | 22–24 AWG hookup wire, zip ties, electrical tape, labels | — | [$__] | |

**Accounts (free):** Particle (Free Plan: up to 100 devices and 100K data operations per month, cellular data included — [plan details](https://docs.particle.io/notes/plans)) and Pushover for push alerts (one-time app purchase; check current price).

## 5. Tools

- Small flat and Phillips screwdrivers (screw terminals)
- Wire strippers and flush cutters
- Digital multimeter (continuity and DC volts)
- Plug-in receptacle tester (3-light)
- Drill and step bit for the enclosure (button, receptacle, cord grips)
- Computer with Chrome or Edge (device setup and firmware flashing)
- iPhone with the Shortcuts and Pushover apps
- A lamp or other small 120 V load for bench testing

## 6. Step-by-step build

Build and test the low-voltage side first, then the AC side. Keep the enclosure cord unplugged whenever the lid is open.

### 6.1 Set up the controller (30 min)

1. Attach the M404 module and the LTE antenna to the Muon as shown in Particle's quick-start.
2. For bench setup, power the Muon from a USB-C PD supply and C-to-C cable (a USB-A-to-USB-C cable will not work; the Muon negotiates its voltage over USB-C PD), or from the DIN-rail supply (BOM item 8) to VIN once its output is set. VIN accepts 6–12 V.
3. On a computer, go to setup.particle.io, create or sign in to your Particle account, and follow the prompts. Particle recommends this route because it updates Device OS, activates the SIM and turns on the 3V3_AUX supply the header needs.
4. Name the device (e.g., `Hangar_<tail number>`). Confirm it shows online in the Particle Console.

### 6.2 Fit the screw-terminal HAT

1. Power off. Press the HAT onto the Muon's 40-pin header, aligned pin 1 to pin 1.
2. **The HAT's labels use Raspberry Pi numbering, not Particle names.** Use this map:

| Function | Muon pin | Header pin | HAT label |
| --- | --- | --- | --- |
| Relay control out | D2 | 11 | IO17 |
| ARM/SAFE sense in | D4 | 33 | IO13 |
| 5 V (relay power, via ARM/SAFE) | 5V | 2 or 4 | 5V |
| Ground | GND | 6, 9, 14, 20, 25 | GND |

Pins 1 and 17 are 3V3. **Do not power the relay module from 3V3** — it will not pull in.

### 6.3 Configure and wire the relay module (low-voltage side)

1. Set the trigger jumper to **active-high** (high = on).
2. Fit a **15 kΩ resistor from the module's IN terminal to its GND (DC–) terminal.** It holds the relay off before firmware runs or if the control wire comes loose.
3. Wire **D2 (IO17) → IN**.
4. Wire **Muon GND → module DC–**.
5. Module **DC+** connects to the ARM/SAFE switched rail in 6.4, not directly to 5V.

### 6.4 Wire the ARM/SAFE switch

The latching button sits in the relay module's 5 V feed. Latched (LED lit) = ARMED; released (LED dark) = SAFE, and the relay coil has no power no matter what the firmware does.

1. With the meter, find the two switch leads that close when latched and open when released. Call them A and B. Identify the LED + and – leads.
2. Wire **Muon 5V → switch lead A**.
3. From **switch lead B** (the switched rail), run three wires: to **relay DC+**, to **LED +**, and to a **10 kΩ resistor**.
4. Join the other end of the 10 kΩ to **D4 (IO13)** and to a **15 kΩ resistor to GND**. This divider puts about 3.0 V on D4 when ARMED.
5. Wire **LED – → GND**.

```
Muon 5V (pin 2/4) ── switch A
switch B ──┬── relay DC+
           ├── LED +         (LED – to GND)
           └── 10k ──┬── D4 / IO13 (pin 33)
                     └── 15k ── GND
D2 / IO17 (pin 11) ──┬── relay IN
                     └── 15k ── relay DC–
Muon GND ─────────────── relay DC–
```

The firmware reads D4 continuously: HIGH = ARMED, LOW = SAFE. The pin uses an internal pull-down, so a missing or broken sense wire reads SAFE, never ARMED. On SAFE it drives D2 low, clears the commanded state and sends an alert. While SAFE it refuses **every** relay command (on, off or toggle) and returns `-2`, so the phone reports "relays not active" instead of pretending the command worked. Re-arming leaves the outlet off until a new command.

### 6.5 Wire the AC side

One duplex receptacle: the top half is switched by the relay (preheater), the bottom half is always on.

1. Mount the receptacle in the enclosure lid or wall. **Break off the brass (hot-side) tab only**; leave the silver (neutral) tab in place.
2. Bring the 14/3 supply cord in through a cord grip.
3. **Green (ground) → receptacle ground screw.**
4. **White (neutral) → a silver screw.** The intact tab feeds both halves.
5. **Black (hot) →** split with a listed connector into two 14 AWG leads: one to **relay COM**, one to the **always-on half's brass screw**.
6. **Relay NO → the switched half's brass screw.** Leave NC empty.
7. Label the switched half **"PREHEAT — SWITCHED"** and the other **"ALWAYS ON."**
8. Mount the relay module on standoffs with its AC terminals away from the controller and antenna. Keep AC and low-voltage wires on separate sides of the box.
9. Controller power: mount the 8–12 V DIN-rail supply inside the box. Connect its AC input to the incoming line and neutral **upstream of the relay**, so it is always powered, and its ground terminal (if it has one) to the box's equipment ground. Before connecting the Muon, power the supply and set and measure its output within 6–12 V. Then wire DC+ to the Muon's VIN and DC– to GND.

### 6.6 Connect the temperature probes

1. Plug the Qwiic cable from the Muon's Qwiic connector to the DS2484 board.
2. Connect each probe's three wires to the DS2484 screw terminals: data, power and ground. On Brian's DROK probes, yellow = data, red = power, black = ground; confirm against your probe's documentation and Adafruit's DS2484 pinout page.
3. Both probes share the same three terminals. The firmware tells them apart by each probe's factory ID (Section 7).
4. Put a label flag on each probe: ENGINE and HANGAR.

### 6.7 Extend the engine probe and attach it to the cord

1. Extend the ENGINE probe with twisted-pair cable long enough to follow the extension cord from the enclosure to the cowl. Use one pair for data + ground and a second pair for power. Solder and heat-shrink the splices.
2. **Test at full length on the bench before installing** (Section 8, test 9).
3. Tape or zip-tie the probe tip 4–6 inches back from the extension cord's plug end, so it sits inside the cowl when the cord is plugged in but doesn't touch hot parts. Secure the cable along the cord every 1–2 feet so the two always move together.

### 6.8 Final assembly

1. Mount the Muon, HAT and DS2484 at the end of the enclosure away from the relay and AC wiring. Mount the ARM/SAFE button where it's easy to reach.
2. Put the antenna on the top or side facing the hangar door, at least 1–2 inches from any steel once mounted.
3. Bring probe cables out through a cord grip.
4. Label the box: "Preheat controller — SAFE (button out) = outlet off. Unplug the preheater before fueling or engine start."

## 7. Firmware and iPhone Shortcuts

You don't write code. You flash Brian's shared firmware, point it at your probes, and install the Shortcuts.

### 7.1 Flash the firmware

1. Open the shared firmware link: **[link to be published]**, version **[v__]**.
2. In the Particle Web IDE, copy the app to your account, select your device, and flash.
3. In the Console, confirm the device restarts and publishes an `hm/boot` event.

### 7.2 Assign the probes

1. In the Console, read the device's `status` variable. It lists each probe found with its factory ID.
2. Warm the ENGINE probe in your hand and see which reading rises. Call the `setprobe` function to assign it: **[exact command to be published with the firmware]**.
3. Repeat for HANGAR. Read `status` again and confirm `eng` and `hgr` both show plausible values.

### 7.3 Set up push alerts

1. Install Pushover on your iPhone and create an account; note your user key and create an application token.
2. In the Particle Console → Integrations, add a webhook for event prefix `hm/` that posts to Pushover, using the template Brian publishes **[link]**. Your Pushover keys live in the webhook, not in the firmware.
3. Trigger a test (unlatch and re-latch ARM/SAFE with the outlet on) and confirm a push arrives.

### 7.4 Install the Shortcuts

1. Create a dedicated Particle access token for the Shortcuts only, so you can revoke it without affecting anything else. Keep it private.
2. Install the shared Shortcuts **[links]**. Each one asks for your device ID and token at install; they are never embedded in a shared link.
3. You get: **Preheat** (one button that toggles the preheater) and **Hangar Status**. **All Off** is added when the production firmware publishes the `alloff` function. Preheat reads the controller's reply to the command itself, so what it shows is the state the controller actually set. "ON" means the relay is on; it does not yet prove current is flowing. That confirmation comes with the current sensor (PRD Section 9A, IF-003/IF-004).
4. Optional: add a Siri phrase to Preheat (e.g., "Preheat the airplane"). Because it toggles, always read the reply: ON or OFF.
5. **Never re-run Preheat just because it said "No response from hangar."** The command may have reached the controller even though the reply didn't reach you, and a second toggle would undo it. Run Hangar Status first (or check `tanisState` in the Console), then decide.

| Shortcut | Cloud call | What you see |
| --- | --- | --- |
| Preheat | `tanis` = `toggle` | "Tanis ON — heat not yet confirmed", "Tanis OFF", "Relays NOT active" or "No response from hangar" |
| Hangar Status | read `tanisState`, `armState` (and `status` once released) | States, temps, engine-minus-hangar, signal |
| All Off (later) | `alloff` | Command result |

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

Don't add a retry or a Repeat around step 1: toggle is not idempotent. Copy it and change `tanis` to `cabin` for a cabin-heat button. If your phone itself has no connection, Shortcuts stops at step 1 with its own error, which still tells you it failed.

## 8. Bench test checklist

These are the tests Brian's prototype passed. Plug a lamp into the switched half. Pass every test before taking it to the hangar.

- [ ] **1. Receptacle wiring.** Lid closed, cord plugged in, ARMED, outlet commanded on: the 3-light tester shows correct wiring on both halves. Commanded off: switched half dead, always-on half live.
- [ ] **2. 3.3 V trigger.** ARMED. Run Preheat ten times: the relay switches cleanly every time and the phone alternates Tanis ON / Tanis OFF, matching the lamp.
- [ ] **3. Power-up stays off.** Remove and restore controller power five times. The lamp never lights.
- [ ] **4. Reset and update stay off.** Press the Muon's reset button, then reflash the firmware over cellular. The lamp never lights.
- [ ] **5. SAFE blocks in hardware.** With the lamp on, release the button to SAFE: lamp goes out immediately, the button LED goes dark, and a SAFE alert arrives.
- [ ] **6. Commands rejected while SAFE.** Run Preheat: phone shows "Relays NOT active," lamp stays off. Read `armState` in the Console: SAFE. (If it reads ARMED with the button released, D4 isn't seeing the switch; fix before going on.)
- [ ] **7. Re-arm stays off.** Latch to ARMED: lamp stays off until you run Preheat again, which shows Tanis ON.
- [ ] **8. Boot while SAFE.** With the button released, power-cycle the controller: lamp stays off and status reports SAFE. Then measure D4 with the relay energized: it should read at least 2.4 V.
- [ ] **9. Probes at full length.** With the engine probe on its full-length extension cable, Hangar Status shows both temps. Warm the engine probe by hand; its reading rises within a minute. No missing readings over 30 minutes.
- [ ] **10. Unreachable state.** Remove controller power and wait a minute. Preheat shows "No response from hangar" with Particle's error text; Hangar Status shows no current reading.

## 9. Hangar installation and soak test

1. **Mount the enclosure** at the spot you chose in Section 3, off the floor, with the antenna at least 1–2 inches from steel. Brian mounts his beside the man door, next to the light switch. On a steel wall, magnets work; size them for sliding (shear) load, which is much lower than the rated pull. Add a short tether so the box can't fall.
2. **Plug the enclosure cord directly into the hangar receptacle.**
3. **Plug the preheater extension cord into the switched half.** Double-check the label.
4. **Strain relief:** loop the extension cord and probe cable through a hook or cleat below the box, so a pulled cord loads the hook, not the box.
5. **Hangar probe:** place it about chest height, away from doors, sunlight and the airplane's heat.
6. **Signal check in place:** with the door closed, confirm the device is online and run Hangar Status from home or the parking lot.
7. **Soak test, 24–48 hours:** leave it running with the door closed. Pass = no disconnects that last more than a few minutes, every Shortcut answers, no unexpected restarts in the Console event log.
8. **First real preheat:** run Preheat (confirm it says Tanis ON) with the cord plugged into the airplane. Note how fast engine-minus-hangar rises. After the first few cycles, set the "not warming" alert threshold from what you saw **[firmware setting to be published]**.

## 10. Daily use and preflight

**When you leave the airplane:** plug the extension cord into the preheater plug (probe goes into the cowl with it). Leave the button ARMED if you want remote preheat; SAFE if you don't.

**Before a flight:**

1. Run **Preheat** several hours ahead and confirm the reply is **Tanis ON**. It toggles, so if it says OFF it was already on; run it again. Tanis recommends about six hours or longer for full benefit; overnight is fine.
2. Run **Hangar Status** after 30–60 minutes and look for engine temperature pulling away from hangar temperature. The probe reads cowl air, not metal; watch the trend.
3. At the airport: **unplug the extension cord from the airplane before fueling or engine start.** The probe comes out with it.
4. Run **Preheat** (reply: Tanis OFF) or press the button to SAFE.

**If the flight is canceled:** turn it off. If you forget, you'll get a long-run alert at 24 hours. The firmware never turns the Tanis off by itself.

## 11. Troubleshooting

Change one thing at a time and recheck.

| Symptom | Likely cause | Check |
| --- | --- | --- |
| Muon won't power up | Wrong supply or setting | Measure the DIN-rail output: VIN needs 6–12 V. On the bench, USB-C needs a true PD supply and C-to-C cable |
| "No response from hangar" | No signal, no power, device offline, or token / device-name error | Read Particle's error text in the alert. Console shows online? Power at the supply? Re-run signal check at the box |
| Preheat shows "Relays NOT active" | Button released, or a loose switched-rail or sense wire | Latch the button (LED lit); check 5 V at relay DC+; check D4 reads about 3 V; `armState` should read ARMED |
| Shortcut says ON/OFF but the relay doesn't follow | Old firmware, or Shortcut not reading `return_value` | Flash current firmware; Shortcut step 2 must read `return_value` from the POST reply |
| Relay doesn't click | Module DC+ on 3V3, jumper on active-low, or SAFE | DC+ must come from 5V through the switch; jumper active-high |
| Relay clicks but switched half is dead | Load on NC, or loose NO / brass-screw wire | Wiring per 6.5; receptacle tester |
| Outlet is on at power-up or when it should be off | Load wired through NC, or jumper on active-low | Fix immediately: COM → NO, active-high, 15 kΩ pull-down fitted |
| Both halves switch together | Brass tab not broken | Unplug; break the brass tab |
| A probe missing or reads nonsense | Loose terminal, wire order, or cable length | Recheck data / power / ground; test the engine probe without its extension |
| Engine temp doesn't rise | Cord not seated at the preheater plug, preheater fault, or probe outside the cowl | Check the preheater's own indicator; reseat; check probe position |
| No push alerts | Webhook or Pushover keys | Console → Integrations → webhook log for errors |

## 12. Expansion options

Each add-on uses spare controller pins and leaves the minimum build unchanged. Guides for each come later.

| Add-on | Hardware | Notes |
| --- | --- | --- |
| Cabin heat | Second relay module on D3 (header 36, HAT IO16), same configuration, DC+ on the same ARM/SAFE rail; second switched receptacle; third probe; ceramic heater | Heater must have tip-over and overheat shutoff and be listed. Run it on its lowest setting (Brian uses 700 W, about 9.8 A combined with the Tanis) and placard it. Firmware enforces a run-time limit |
| Current confirmation | Split-core current sensor (SCT-013-030) clamped on the switched hot lead inside the box, bias circuit, analog pin A6 | Gives "confirmed on" in seconds and catches a welded relay |
| Power-loss alert | Muon-compatible 3.7 V LiPo with temperature sensor (3-pin JST-PH) | 2-pin batteries without a thermistor are not compatible |
| Flood warning | Public river-gauge alerts + floor water sensor on D5 | River gauges give the lead time; the floor sensor is backup |
| Activity / lights | Motion sensor on D6, light sensor on the Qwiic bus | Later phase |

## Appendix A. Pin map and open items

| Muon pin | Header pin | HAT label | Minimum build | Reserved for |
| --- | --- | --- | --- | --- |
| D0 / D1 (I2C) | 3 / 5 | SDA / SCL | DS2484 via Qwiic connector | Light sensor (same bus) |
| D2 | 11 | IO17 | Relay IN (15 kΩ pull-down to relay GND) | — |
| D3 | 36 | IO16 | — | Cabin-heater relay |
| D4 | 33 | IO13 | ARM/SAFE sense, 10k/15k divider from switched rail | — |
| D5 | 32 | — | — | Flood sensor |
| D6 | 12 | — | — | Motion sensor |
| A6 | 24 | — | — | Tanis current sensor |
| 5V | 2, 4 | 5V | ARM/SAFE switch → relay DC+ and LED | — |
| 3V3 | 1, 17 | 3V3 | Not used for the relay | Sensor supply |
| GND | 6, 9, 14, 20, 25, 30, 34, 39 | GND | Relay DC–, LED –, divider | — |

Avoid header pins 13, 15, 16 and 18 (boot-sensitive), 19, 21 and 23 (used internally for SPI), and 27–28 (not connected). All Muon GPIO is 3.3 V only and not 5 V tolerant.

**Open items before release** (owner: Brian)

- [ ] Receive the DS2484 bridge; confirm it and its library read the probes on the M404 (Device OS 6.x)
- [ ] Confirm engine-probe reliability at the longest expected cable length
- [x] Controller power supply for builders: 8–12 V DIN-rail supply to VIN
- [ ] Name a specific DIN-rail supply model and price for the BOM
- [ ] Publish firmware, the `setprobe` command, the alert threshold setting, the Pushover webhook template and the Shortcuts
- [ ] Fill in bracketed prices; confirm the Muon kit contents
- [ ] Decide whether Android users get a front end

## Sources

- Tanis TPG0001 piston preheat guide
- Hangar Remote Monitoring & Preheat Control System PRD v0.19, including Section 9A (`docs/PRD.md`)
- [Muon datasheet](https://docs.particle.io/reference/datasheets/m-series/muon-datasheet/) · [Particle plans](https://docs.particle.io/notes/plans) · [Particle store](https://store.particle.io/products/particle-maker-kit)
- [Adafruit DS2484 (5976)](https://www.adafruit.com/product/5976)
