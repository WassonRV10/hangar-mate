# Hangar Remote Monitoring & Preheat Control System
## Project Requirements Document — Version 0.17

**Project status:** Initial requirements  
**Primary application:** Remote aircraft hangar monitoring and preheat control  
**Communications constraint:** No Wi-Fi available at hangar  
**Proposed controller platform:** Particle Muon carrier board + M404 M-SoM

**Revision notes (v0.17):** **Temperature-probe interface changed** (owner decision, 2026-10-27): the DS18B20 1-Wire bus moves from a GPIO pin to an **Adafruit DS2484 I²C-to-1-Wire bridge** on the QWIIC connector. Bench testing showed the probes answer presence pulses but every read returns 1s; Particle states RTL872x GPIO (the M404's MCU family) is too slow to bit-bang 1-Wire. Header pin 29 (A0/D19) is freed. Also corrects the pin name for header pin 29 (A0/D19, not D18/A0 as listed since v0.6). The CT bias/calibration work is paused pending a 10 µF capacitor. Parts to order: DS2484 breakout (Adafruit 5976), STEMMA QT/Qwiic cable, 10 µF capacitor, 10k resistor (ARM/SAFE divider). See Section 16, Appendix A.1, A.2, A.3, and Section 25 items 30 and 34.

**Revision notes (v0.16):** Corrects a design error in the v0.11 master-reset design and closes it on hardware. **Error:** v0.11 put a *momentary* normally-open pushbutton in series with the relay-coil 5V feed. That wiring keeps the relays unpowered except while the button is held — the opposite of the intent — and no momentary switch can hold the outlets off in hardware if the firmware is hung. **Change (owner decision, 2026-10-27):** the master reset is now a **latching (push-on/push-off) ARM switch** — a 19 mm 12 V illuminated latching button (switch rated 1 A, LED isolated from the switch). **Latched (LED lit) = ARMED**: 5V reaches both relay modules. **Unlatched (LED dark) = SAFE**: 5V to both relay modules is cut in hardware and stays cut until someone re-latches the switch. The Daiertek momentary button is no longer used. **Bench-verified 2026-10-27** with both relays and lamp loads: SAFE turns both outlets off immediately; "on" commands are rejected while SAFE; re-arming leaves outlets off until a new command; with the firmware deliberately ignoring the switch, SAFE still cut both relays (hardware path proven); booting while SAFE leaves both outlets off. The D4 sense divider was built as 15k/15k (~2.5 V) on the bench; 10k/15k (~3.0 V) is specified for final installation for more logic-HIGH margin. Affects FR-015a, Appendix A.1, A.2, A.3, and Section 25 item 31.

**Revision notes (v0.15):** The cabin-heater relay (Relay 2, D3 / header pin 36 / HAT "IO16") was bench-verified on 2026-10-27 in the same configuration as Relay 1 (5V VCC, active-high jumper, 15 kΩ IN pull-down, COM→NO). Test firmware drove D3 and held D2 LOW from `STARTUP()`. Results: reliable switching from 3.3 V (on/off and a 5-cycle test), and the relay stayed off through repeated power cycles, resets, and an OTA reflash. Both relay modules are now verified, which closes the remaining relay build task under Section 25 item 29 and Appendix A.3. See ER-001, Appendix A.1, A.2, and A.3.

**Revision notes (v0.14):** Closes Section 25 item 29 — the Tanis relay (Relay 1) was bench-verified on actual hardware on 2026-10-26 with bench-test firmware on D2. **Verified configuration:** hiBCTR module VCC on Muon **5V** (header pin 2 or 4), trigger jumper set to **active-high**, a **15 kΩ pull-down resistor from IN to the module's GND terminal**, firmware drives D2 LOW at startup (`STARTUP()`), and the load is wired COM→NO. **Results:** the Muon's 3.3 V GPIO triggers the relay reliably (on/off commands and a 5-cycle blink test), and the relay stays OFF through repeated power cycles, resets, and an OTA reflash. **Findings recorded:** (1) with the jumper in the active-low position, the outlet energized on power-up with an unconfigured controller, which violates 2.2 fail-safe behavior — active-high is required; (2) with module VCC on 3V3, a 3.3 V signal reached IN but the relay did not pull in — module VCC must be 5V. Also corrects the A.2 power row, which listed pins 1, 4, and 17 together as "3V3 / 5V": pins 1 and 17 are 3V3 and pins 2 and 4 are 5V. The cabin-heater relay (Relay 2, D3) shall be configured and tested the same way before installation. See ER-001, Appendix A.1, A.2, A.3, and Section 25 item 29.

**Revision notes (v0.13):** Records the on-site cellular signal test of the actual Muon + M404 (2026-10-13) and defines the controller's physical installation. **Signal (Section 25 items 15–16):** the M404's Particle SIM selects its carrier automatically and was observed on **both AT&T and T-Mobile** — the earlier single-carrier "T-Mobile" assumption (based on a phone test) is superseded. Measured LTE Cat-M1 signal inside the hangar ranged from RSRP −95 to −113 dBm; the best signal quality (RSRQ −12 dB) was near the hangar door, and the worst (−113 dBm / −20 dB, followed by a dropped session) was midway between the back wall and the door. All messages were delivered without retransmission at every position. **Installation (new ER-005):** the controller enclosure will be mounted on the fixed wall beside the fiberglass man door, next to the light switch and bi-fold door opener button. ER-005 defines the enclosure type (non-metallic, listed), no-drill magnetic mounting with independent cord strain relief and a fall tether, antenna placement, inlet rating, and an in-place 24–48 hour soak test before final installation. Enclosure material resolved as a listed polycarbonate enclosure, replacing the owner's original wooden-box plan (item 32). See ER-005, Section 15, and Section 25 items 15, 16, 32, and 33.

**Revision notes (v0.12):** Tightens the cabin heater's unattended/remote-controlled setting limit from **900 W to 700 W** (owner decision, 2026-10-13), superseding the v0.9 decision. At 700 W, combined Tanis + heater draw is **9.8 A** (vs. 11.5 A at 900 W), which leaves real margin under both the shared circuit's 16 A continuous capacity and the 12 A continuous rating (80% of 15 A) of the standard plug, cord, and power-strip hardware feeding the controlled outlets. Rationale: the RV-10 cabin is small, the heater's purpose is to take the chill off rather than fully heat the cabin, and the margin gain costs very little in useful heat. Affects FR-006 (decision text and placard content), Section 18 step 7, Section 20, and Section 25 item 21. The placard on the heater shall reflect 700 W. *Note: the separate draft "PRD v0.12 — Section 9A Command & Notification Interface (DRAFT)" is maintained as its own document and has not yet been merged into this file.*

**Revision notes (v0.11):** Resolves two more open items from on-hand hardware. **Controller power (Section 25 item 11b)** — the owner already has a USB-C PD supply and will plug it into the power-strip receptacle per ER-003's outlet allocation; this satisfies ER-004's requirement for a genuine USB-C PD-capable source (not a repurposed USB-A charger). **Master-reset switch (Section 25 item 31)** — the owner has a Daiertek 12V, 19mm momentary horn-style pushbutton on hand. This is a standard 2-terminal (SPST, Normally-Open-only) switch, not a DPST — switches of this generic "horn button" style are built with a single pole. This settles the pole-count question in Appendix A.1's hybrid master-reset design: with only one pole available, the GPIO logging/detection tap cannot come from a second pole or a diode off the switch itself. Instead, the detection tap is redesigned as a **voltage-divider sense input** reading the relay-coil-power rail directly, downstream of the switch — the same node the button interrupts — so the Muon can detect "rail present" vs. "rail cut" without needing a second pole. See Appendix A.1 and A.2 for the revised design and pin note, and Appendix A.3 for the one remaining item (confirming divider resistor values on the bench).

**Revision notes (v0.10):** Resolves Open Question 29 — relay hardware purchased: a **hiBCTR 2-pack, 5V-trigger, 30A single-channel relay module**, opto-isolated input, one unit for each of the Tanis and cabin-heater branches. This corrects a terminology assumption carried since v0.6: the purchased part is an **electromechanical relay module, not a solid-state relay (SSR)** as Appendix A had recommended — evident from its Normally-Open/Common/Normally-Closed terminal layout, which a true SSR doesn't have. This is evaluated as an acceptable substitution, not a defect: the 30A rating comfortably exceeds both loads (5.0A Tanis, 12.5A heater), and expected switching frequency (roughly once per flight day) is nowhere near the mechanical-cycle-life limits that make SSRs preferable in high-cycle applications. Two real open items remain before installation: confirming the board's opto-isolated input reliably triggers from the Muon's 3.3V GPIO (commonly fine for 5V-rated modules of this type, but not yet verified for this specific board), and confirming the trigger-polarity jumper setting (active-high vs. active-low) matches the firmware. See ER-001, Appendix A.1, and Section 25 item 29. *(Both items closed in v0.14.)*

**Also in v0.10:** two more purchases resolve open items. **Temperature sensors** — a DROK 5-pack of waterproof DS18B20 probes, matching Appendix A.1's recommendation exactly (3 needed for engine/cabin/hangar, 2 spares); no open issues. **Tanis electrical confirmation (FR-003 / Section 25 item 30)** — a Sazkjere 4-pack of SCT-013-030 30A split-core current-transformer (CT) sensors, not the opto-isolated AC-presence module Appendix A had recommended. This is an upgrade in capability (actual current magnitude instead of a simple present/absent signal) but changes the wiring and firmware: it needs an analog input pin instead of a digital one, a bias circuit to center its AC output within the Muon ADC's 0–3.3V range, and firmware-side RMS calculation rather than a simple digital read. See Appendix A.1 and A.2 for the revised pin assignment, and Section 25 item 30 for what's still open. With 2 spare CT sensors on hand, extending current sensing to the cabin-heater branch is now a low-cost option — see Appendix A.1's note on this.

**Revision notes (v0.9):** Confirms the v0.8 procedural-constraint recommendation (900 W max heater setting for unattended/remote-controlled use) as a **decision**, not just a recommendation — the owner will place a placard on the heater itself, legible to anyone operating it (not just the owner), stating the available settings and which one to avoid and why. See FR-006 for the placard's required content. *(Setting limit superseded by v0.12: 700 W.)*

**Revision notes (v0.8):** Records the actual cabin heater electrical rating: **120 V, 12.5 A max, with three manual power settings (700 W / 900 W / 1500 W)** — resolves Section 25 item 21. This surfaces a real conflict rather than just a data point: the heater's 1500 W setting draws 12.5 A, which combined with Tanis's 4.0 A (ER-002) totals **16.5 A**, exceeding the shared circuit's 16 A continuous-duty capacity (80% of the 20A breaker) by 0.5 A — and this isn't an edge case, since running both together for hours is the normal Phase 2 workflow. Because the heater's wattage dial is manual (the controller only switches the outlet, per FR-006) and adding current-sensing hardware to auto-detect this would be disproportionate to the risk, the resolution is a **procedural constraint** consistent with TSR-003's pattern: document that the heater shall be left on **900 W or lower** whenever it may run unattended/remote-controlled alongside Tanis, rather than adding hardware to enforce it in software. See FR-006 and Section 20 for details.

**Revision notes (v0.7):** Corrects the Tanis system's electrical rating: **460 W, 4.0 A at 115 V** (previously recorded as 240 W, 2.1 A — that figure is superseded). This changes two things downstream: ER-002's load-margin figures are recalculated, and the Phase 2 cabin-heater headroom analysis (Section 20) is revised — the available continuous-current headroom on the shared 20A circuit drops from ~13.9 A to **~12.0 A**, which narrows the acceptable cabin-heater wattage range if it's expected to run concurrently with Tanis. See ER-002, Section 20, and Section 25 items 3–5 for the specific changes.

**Revision notes (v0.6):** Resolves Open Question 11a — the Particle M404 (North American LTE Cat M1 with 2G fallback, built-in GNSS) has been selected and ordered separately from the already-ordered bare Muon carrier board. Adds **Appendix A**, a full GPIO pin assignment against the Muon's 40-pin expansion header, so wiring/enclosure prep can proceed while the M404 module is in transit. Appendix A also recommends specific component types for the relay and sensor interface: solid-state relays (SSR) for both switched branches (ER-001), waterproof DS18B20 1-Wire digital probes sharing a single bus for the engine/cabin/hangar temperature sensors, an I2C light sensor on the QWIIC connector, an AC-presence sensing module to satisfy FR-003's electrical-confirmation requirement, and a hybrid hardware+software master-reset design (physical button cuts relay-coil/SSR control power directly; a secondary GPIO tap is used only for event logging). Adds three new open items (29–31) covering the specific SSR module, AC-presence module, and master-reset switch pole count still to be selected.

**Revision notes (v0.5):** Switches the controller platform from the Particle Boron (BRN404X) to the Particle Muon carrier board with an M-SoM, due to Boron order-fulfillment/delivery problems. Updates all Boron-specific references (GPIO voltage, power architecture, antenna/enclosure notes, hardware architecture) to Muon equivalents pending confirmation of the specific M-SoM module ordered. Also resolves the Tanis shore-power plug location: accessed through the small oil-filler door on the cowling.

**Revision notes (v0.4):** Confirms the hangar circuit is a 20A breaker and does the headroom math for a future cabin heater; recommends a 14-gauge outdoor-rated extension cord; reframes flood detection around early-warning lead time (adds river-stage monitoring as a Phase 3 item, given the point sensor alone likely can't give enough warning to fly the aircraft out); defines the cabin heater's baggage-compartment installation and adds an unattended-operation safety timeout (FR-006); formally defines the local master-reset override (new FR-015a); adjusts TSR-002's notification threshold to account for legitimate overnight warm-up starts; and adds phone-location/geofencing as a candidate mechanism for occupancy detection and owner identification (FR-009, FR-012).

**Revision notes (v0.3):** Incorporates actual aircraft (Vans RV-10) and Tanis electrical values, documents the hangar's single-duplex-outlet constraint and the non-invasive (no panel/wiring alteration) installation requirement, confirms T-Mobile cellular signal is adequate, and resolves most of the Section 25 open questions.

**Revision notes (v0.2):** Incorporates BRN404X-specific datasheet, clarifies engine sensor mounting strategy, reframes Tanis operating profile as a pre-flight warm-up window rather than indefinite continuous operation, resolves the aircraft-operation interlock (TSR-003) as procedural/physical disconnect, and clarifies acceptable controller behavior in extreme cold.

---

## 1. Purpose

Develop a remotely accessible system that allows the aircraft owner to monitor and control selected electrical equipment at an aircraft hangar without requiring Wi-Fi at the hangar.

The primary purpose is to:

1. Remotely initiate aircraft engine preheating before traveling to the airport.
2. Monitor temperatures to confirm that the aircraft engine compartment and cabin are warming.
3. Optionally control a small electric cabin heater.
4. Provide environmental and security monitoring.
5. Provide notifications when potentially undesirable conditions occur.
6. Record activity at the hangar so the owner can determine when someone has entered or worked around the aircraft.

The system should operate independently of hangar Wi-Fi by using cellular communications.

---

# 2. Design Principles

### 2.1 Safety first

The system shall not interfere with normal aircraft operation, aircraft electrical systems, fueling procedures, or required preflight procedures.

The Tanis operating guide specifically states that the aircraft is not to be fueled or the engine operated while the preheat system is connected to shore power. The system must therefore provide an obvious and reliable means of disabling/disconnecting preheat before aircraft operation.

The Tanis system shall not be controlled by a timer or thermostat. The Tanis documentation specifies continuous operation and explicitly states that timers and thermostats are not to be used.

### 2.2 Fail-safe behavior

Loss of cellular communications shall not create an unsafe condition.

Loss of power to the controller shall not create an unexpected aircraft-preheat condition.

A failed controller shall not be capable of defeating the aircraft's normal electrical protection.

### 2.3 Incremental development

The system should be designed so that the core preheat functionality can be installed and validated before adding environmental and security features.

---

# 3. System Overview

The proposed system consists of:

- Cellular-connected controller
- Remote electrical switching/control
- Engine-compartment temperature sensor
- Cabin temperature sensor
- Hangar temperature sensor
- Flood/water sensor
- Hangar occupancy/activity sensor
- Hangar lighting-state sensor
- Local status indicators
- Backup power for the controller
- Remote notification and control software

**Controller platform change (v0.5):** The system was originally specified around the Particle Boron LTE (BRN404X). The owner encountered order-fulfillment/delivery problems with the Boron and is switching to the **Particle Muon carrier board** paired with a Particle M-SoM (M.2 System-on-Module).

The Muon is Particle's newer multi-radio carrier board, built around the Raspberry Pi form factor, that accepts an M-SoM module providing cellular, Wi-Fi, BLE, and GNSS, with the Muon carrier itself additionally providing LoRaWAN and Ethernet. Functionally it fills the same role the Boron did in this design — a cellular-connected controller with GPIO, analog inputs, I²C, SPI, and battery support — but the physical hardware, power architecture, and connector layout differ meaningfully from the Boron and are called out where relevant throughout this document (see ER-001, ER-004, Section 15).

**Open item carried into Section 25:** the specific M-SoM model to be paired with the Muon carrier (e.g., an LTE Cat M1 North American module comparable to the BRN404X) has not yet been confirmed. Electrical specifics in this document (GPIO voltage, current draw) are based on general Muon/M-SoM documentation and should be re-verified against the datasheet for the specific M-SoM module once selected, the same way the original BRN404X datasheet was used to correct assumptions made from the general Boron datasheet.

---

# 4. Functional Requirements

## FR-001 — Cellular communications

The system shall communicate with the owner remotely using cellular connectivity.

The system shall not require hangar Wi-Fi.

The system shall periodically verify cellular connectivity and report communication failures.

---

## FR-002 — Remote Tanis preheater control

The owner shall be able to remotely request that the Tanis engine preheater be energized.

The system shall provide a positive indication that the preheater command was received.

The system shall provide an indication of whether the system believes the Tanis heating circuit is actually energized.

The control system shall be designed around the Tanis requirement for continuous operation rather than periodically switching the heater on and off.

The Tanis documentation states that the system is activated by connecting it to appropriate shore power and deactivated by disconnecting it.

---

## FR-003 — Preheater operating status

The system shall provide remote indication of:

- Preheater OFF
- Preheater commanded ON
- Preheater confirmed ON
- Controller unavailable
- Possible preheater fault

Where practical, confirmation shall be based on an electrical measurement or other direct indication of the heating circuit rather than relying solely on the command sent to the controller.

**Implementation note (v0.10):** this will be implemented with a current-transformer (CT) sensor (see Appendix A.1), which reads actual current magnitude rather than a simple present/absent signal — a capability upgrade over this requirement's original intent, since it can also flag an abnormally low reading (partial element failure) rather than only a fully-off condition.

---

## FR-004 — Engine-compartment temperature

The system shall measure temperature in the engine compartment.

The temperature sensor shall be mounted to the Tanis shore-power extension cord itself, positioned near the plug end so that it sits inside the cowl opening when the cord is connected to the aircraft's Tanis shore-power plug. This approach is intentional:

- The sensor requires no permanent attachment point on the engine, cylinder, or crankcase, so it cannot interfere with engine components, maintenance, or existing engine-monitor probes.
- Because the sensor is physically part of the extension cord assembly, it is automatically withdrawn from the cowling whenever the cord is unplugged — the same action already required before flight. The sensor is never left in a location where it could be forgotten or interfere with flight.
- The sensor is not expected to read true engine-metal temperature; it reads the rise in ambient air temperature inside the cowling that results from the Tanis system heating the engine, which is sufficient to confirm the preheater is working.

**Entry point (v0.5):** On this RV-10, the Tanis shore-power plug is accessed through the small oil-filler door on the cowling (see Section 25, item 6). The extension cord and its attached sensor therefore route into the cowl through that door opening, not through a separate dedicated cutout — worth noting for anyone servicing the aircraft, since the oil door will have the cord passing through it whenever the system is energized.

The system shall periodically transmit the measured engine-compartment temperature.

The owner shall be able to view the current temperature remotely.

The system shall retain temperature history for later review.

The Tanis guide states that the installed preheat system produces an approximately 60°F ±10°F / 33.33°C ±5.56°C rise above ambient at thermal equilibrium. Because the sensor reads cowling ambient air rather than metal temperature directly, this figure is a useful qualitative reference for confirming a rising trend rather than an exact target value; the actual observed rise and equilibrium time should be characterized empirically once the sensor is installed.

---

## FR-005 — Cabin temperature

The system shall measure cabin temperature.

The cabin temperature shall be remotely viewable.

Cabin temperature history shall be retained.

The system shall be capable of detecting whether the cabin temperature is increasing after the cabin heater is enabled.

---

## FR-006 — Cabin heater control

The planned heater is a low-cost ceramic space heater that activates automatically whenever power is applied (no network/smart control of its own) and relies on its internal thermostat to cycle. This simplifies the control architecture: the system does not need to communicate with the heater directly — remotely enabling the cabin heater means remotely applying power to the outlet it's plugged into via a controller-driven relay, exactly like the Tanis outlet, just on its own independent circuit path.

The cabin heater shall be electrically independent from the Tanis preheater control.

The owner shall be able to remotely enable and disable power to the cabin heater outlet.

**Because this heater will run unattended in an enclosed aircraft compartment**, the following safety measures apply regardless of the heater's own built-in protections:

- The heater selected shall include tip-over shutoff and overheat/overtemperature protection as standard features (common on ceramic heaters, but shall be confirmed rather than assumed for a "cheap" unit) and shall be UL/ETL (or equivalent) listed.
- The system shall independently enforce a maximum on-duration safety timeout at the controller level — i.e., the controller shall automatically de-energize the cabin heater outlet after a configurable maximum run time even if no further command is received, rather than relying solely on the heater's own protections.
- This timeout is a cabin-heater-specific safety backstop and is unrelated to the Tanis no-cycling requirement (TSR-001), which does not apply to the cabin heater.

The cabin heater shall not be treated as part of the Tanis preheat system. The Tanis documentation explicitly states that interior heaters are outside the scope of its engine-preheat modification.

**Planned installation:** the heater will be placed in the baggage compartment, using the existing cord/opening already run into the compartment for the trickle charger. The trickle charger and cabin heater will need a shared power strip or outlet arrangement inside the compartment if both are to remain connected; this should be confirmed as a wiring detail rather than assumed.

**Heater rating and setting constraint (v0.8):** The selected heater is rated **120 V, 12.5 A**, with three manual power settings: **700 W, 900 W, and 1500 W**. The power-level dial is a manual physical control on the heater itself — the controller only switches the outlet on and off (per this requirement's own design above) and cannot select or limit which wattage setting is active.

At the 1500 W setting, the heater alone draws 12.5 A. Combined with the Tanis system's 4.0 A (ER-002), simultaneous operation draws **16.5 A**, which exceeds the shared circuit's 16 A continuous-duty capacity (see Section 20) by 0.5 A — a narrow overage, but a real one, since concurrent Tanis + cabin-heater operation for hours is the normal Phase 2 use case, not an edge case.

**Decision (v0.9, limit revised v0.12) — procedural constraint, not a hardware interlock:** consistent with how TSR-003 handles the Tanis disconnect requirement, this is resolved as an owner procedure rather than an automated circuit-load enforcement mechanism: **the heater's dial shall be left on the 700 W (lowest) setting whenever it may be left running unattended or under remote control**, particularly whenever it might run concurrently with Tanis. The 900 W and 1500 W settings are reserved for attended use only.

*v0.12 change:* the v0.9 decision allowed 900 W (11.5 A combined with Tanis). The owner tightened this to 700 W (9.8 A combined) on 2026-10-13. Both settings are within the circuit's 16 A continuous limit, but 900 W runs close to the 12 A continuous rating of the 15 A plug, cord, and power-strip hardware feeding the controlled outlets. The small RV-10 cabin needs little heat to make a meaningful difference, so 700 W provides real margin at negligible cost in useful heat.

This was evaluated against adding current-sensing hardware to detect and prevent the overage automatically; that was not adopted, since it adds meaningful hardware/firmware complexity to guard against an overage that a one-time dial-setting decision fully resolves.

**Placard requirement:** the owner will place a placard directly on the heater, since anyone who might operate it — not only the owner — needs to understand the constraint without having read this document. The placard shall state, at minimum:

- The three available settings (700 W / 900 W / 1500 W)
- That the heater must be left on **700 W** whenever it is not being actively attended (i.e., whenever it might be running under remote/automatic control)
- That the 900 W and 1500 W settings are for **attended use only** and must not be left on when leaving the aircraft/hangar

This is the same visual-cue philosophy already used for the Tanis extension cord (a bright jacket color so its connection state is obvious at a glance, ER-003) — a low-cost physical safeguard that doesn't depend on remembering a rule from this document.

---

# 5. Environmental Monitoring

## FR-007 — Hangar temperature

The system should measure the general hangar temperature.

The hangar temperature shall be remotely viewable.

Hangar temperature history should be retained.

The hangar temperature may be used as the environmental reference for evaluating engine and cabin temperature increases.

---

## FR-008 — Flood detection

**Context:** The hangar (KSET) sits near the confluence of the Missouri, Mississippi, and Illinois rivers. Flooding here arrives as a slow-rising river event from the north, not a sudden local event. The stated purpose of this requirement is not just to detect that water has arrived, but to give the owner enough **lead time** to drive to the airport, preflight, and fly the aircraft out before the field becomes unusable — a fundamentally different (and longer) warning window than a typical basement-flood point sensor is designed to provide.

This has a design implication worth flagging: a point-level sensor at the hangar's low point, by itself, only confirms that water has already reached the hangar floor. Given a ~45-minute drive plus preflight and taxi/departure time, water physically in the hangar may already be too late to act on. A more useful early-warning layer would be to monitor a public river-gauge feed for the nearest relevant gauge (e.g., USGS/NWS water-level services) against a configurable stage threshold, so the owner is alerted while the river is still rising toward flood stage — hours or days ahead of local water arrival — rather than only once water is on the hangar floor. This is called out explicitly as a **Phase 3 candidate**, not deferred to the general Future Enhancements list, given how directly it serves this requirement's actual purpose.

The physical point sensor remains valuable as a local backup/confirmation signal (it doesn't depend on an external data feed, and it independently verifies actual water intrusion), but should not be treated as the sole or primary early-warning mechanism given the fly-the-aircraft-out use case.

A water sensor shall be installed at a strategically selected low point in the hangar as the local/backup detection layer.

The system shall generate an immediate notification when water is detected.

The system shall continue reporting the alarm condition until water is no longer detected or the alarm is acknowledged.

The system should record:

- Date/time water was first detected
- Date/time water detection ended
- Duration of the event

Where a river-stage monitoring layer is implemented, the system should notify the owner when the monitored gauge crosses a configurable threshold, distinct from and in addition to the physical point-sensor alarm.

---

# 6. Hangar Activity Monitoring

## FR-009 — Hangar entry/activity detection

The system should detect activity indicating that someone has entered or is moving around the hangar.

The system shall record activity events with date and time.

The owner shall be able to review the activity history remotely.

The system should distinguish, where practical, between:

- No activity
- Activity detected
- Recent activity
- Known owner activity

The system is not initially required to identify a specific person.

**Candidate approach for "known owner activity":** the owner's phone (already proposed as a location/geofence signal for FR-012) could double as the "known owner" signal here — e.g., the phone being on-site or connecting via the companion app around the time activity is detected — rather than requiring camera-based or biometric identification. This is a simpler and more privacy-friendly way to distinguish "known owner activity" from generic/FBO activity, though it depends on the owner's phone being present and connected, and would not identify a person who enters without the phone (e.g., FBO staff). This should be evaluated as part of Phase 4 design rather than committed to now.

---

## FR-010 — FBO activity verification

The activity-monitoring system should allow the owner to determine whether the FBO entered the hangar after being requested to service/refuel the aircraft.

The system should record the approximate time of the activity.

The system should make it possible to correlate activity with an FBO service request.

The system does not need to positively identify the FBO unless a future requirement is added.

---

# 7. Hangar Lighting Monitoring

## FR-011 — Lights-left-on detection

The system shall detect when the hangar lights are left on.

The system shall be capable of determining whether the lights are on without requiring the lighting circuit itself to be controlled by the monitoring system.

The light switch is located at the man door used to access the hangar, which is a convenient, easily reachable location for a sensor to sit near (e.g., to correlate switch activation with door/entry activity), separate from the light fixtures themselves (see Section 16 — Lighting sensor).

---

## FR-012 — Occupancy-aware lighting alert

The system shall avoid sending a "lights left on" alert while the owner is actively working in the hangar.

The system shall determine that the hangar has become unoccupied before generating a lights-left-on notification.

**Owner-location-based trigger:** in addition to (or instead of) a fixed hangar-inactivity timer, the owner has proposed using phone location as the "unoccupied" signal — if the owner's phone is detected more than a configurable distance (e.g., 5 miles) from the hangar while the lights are still on, that is a strong, low-false-positive signal that the hangar is unoccupied and the lights were left on, since it doesn't depend on interpreting motion-sensor gaps. This requires a companion app or location-sharing mechanism and should be treated as a Phase 5 design option alongside, not necessarily instead of, the inactivity-timer approach, since the geofence method depends on the owner's phone being on, carried, and reporting location reliably.

Example behavior (inactivity-timer variant):

1. Owner enters hangar.
2. Lights are turned on.
3. System detects activity.
4. Owner works on aircraft.
5. Owner leaves.
6. System detects no activity for the configured exit period.
7. Lights remain on.
8. System sends a notification.

The exact inactivity period shall be configurable.

---

# 8. Notifications

## FR-013 — Remote notifications

The system shall be capable of sending notifications for important events.

Initial notification candidates:

- Tanis preheat successfully energized
- Tanis preheat failed to energize
- Engine temperature not increasing as expected
- Cabin heater enabled
- Cabin temperature not increasing as expected
- Flood detected
- Hangar activity detected
- Lights left on after hangar became unoccupied
- Controller lost cellular connectivity
- Controller power failure
- Controller returned to service

Notifications shall include the event type and time.

Where useful, notifications shall include relevant sensor readings.

---

# 9. Remote User Interface

## FR-014 — Remote status dashboard

The owner shall be able to remotely view:

- Tanis preheater status
- Engine-compartment temperature
- Cabin temperature
- Hangar temperature
- Cabin heater status
- Flood sensor status
- Hangar activity status
- Hangar lighting status
- Cellular/controller status

---

## FR-015 — Remote control

The owner shall be able to remotely:

- Turn Tanis preheat ON
- Turn Tanis preheat OFF, subject to the final safety architecture
- Turn cabin heater ON
- Turn cabin heater OFF

The interface shall clearly distinguish between a command being issued and confirmation that the commanded state has actually been achieved.

---

## FR-015a — Local manual override (master reset)

A physical **latching (maintained) ARM switch** shall be installed at the hangar, accessible without needing the remote interface, that forces **all** controlled outlets (Tanis relay, cabin heater relay, and any future controlled outlet) to their de-energized/off state — a "master reset" rather than a per-device toggle. *(Changed v0.16 from a momentary pushbutton — see revision notes.)*

The switch has two positions: **ARMED** (latched, indicator lit — outlets may be energized by remote command) and **SAFE** (unlatched, indicator dark).

Placing the switch in SAFE shall:

- Immediately de-energize all controlled outlets in hardware, regardless of their prior commanded state and regardless of the controller's software state
- Keep all outlets de-energized for as long as the switch remains in SAFE, with remote "on" commands rejected and reported as rejected
- Return the system to a known safe initial state
- Require a new, explicit remote command before any outlet is re-energized after the switch is returned to ARMED (i.e., re-arming does not "resume" prior state — it clears it)
- Be logged as an event (time, and that it was a local override rather than a remote command), consistent with FR-016/SR-002

This gives the owner (or anyone at the hangar) a simple, no-app-required way to guarantee everything is off — for example, before fueling, before engine start, or if something seems wrong — without needing to know or trust the current state of the remote system. Because the switch latches, SAFE also works as a lockout: nothing can be turned on remotely until someone at the hangar re-arms it.

---

# 10. Data Logging

## FR-016 — Event history

The system shall maintain a historical record of significant events.

Events should include:

- Timestamp
- Event type
- Sensor value where applicable
- Device state
- User command where applicable

---

## FR-017 — Temperature history

The system shall record temperature measurements at a configurable interval.

Temperature history shall be available for:

- Engine compartment
- Cabin
- Hangar

The history should make it possible to determine whether preheating is progressing normally before the owner leaves for the airport.

---

# 11. Electrical Requirements

## ER-001 — Electrical isolation

The low-voltage controller electronics shall be electrically isolated from aircraft and high-voltage circuits as appropriate.

High-voltage switching components shall be properly rated for the voltage, current, load type, and environment.

The controller's GPIO pins shall not directly switch high-voltage loads.

**Controller change note (v0.5):** Like the Boron, the Muon/M-SoM platform's GPIO is specified at 3.3 V and is not 5V tolerant, so this requirement is unchanged in substance — external interface circuitry (relay driver, opto-isolation, or solid-state relay module) shall be used between the controller and any high-voltage switching device. The specific interface circuitry may need to be re-selected depending on the Muon's GPIO drive characteristics versus the Boron's, but the isolation principle and 3.3V ceiling both carry over directly.

**Component recommendation (v0.6):** Appendix A originally recommended solid-state relay (SSR) modules for both switched branches, on the premise that they accept the Muon's 3.3V GPIO directly and provide inherent opto-isolation with no separate driver stage.

**Component selected (v0.10):** the part actually purchased — a hiBCTR 2-pack, 5V-trigger, 30A single-channel relay module — is an **electromechanical relay with an opto-isolated input**, not an SSR. This still satisfies this requirement: the opto-isolator plus the relay's physical coil/contact separation provide the same galvanic isolation between the Muon's GPIO and the switched AC an SSR would. See Appendix A.1 for the full evaluation of this substitution.

**Bench-verified (v0.14, v0.15):** both relay modules were verified on hardware — Tanis (D2) on 2026-10-26 and cabin heater (D3) on 2026-10-27. The Muon's 3.3V GPIO triggers each reliably with module VCC on 5V and the jumper set active-high, and each stays de-energized through power-up, reset, and OTA update. See Appendix A.1 for the verified configuration.

---

## ER-002 — Tanis load sizing

**Corrected (v0.7):** The installed Tanis system (Vans RV-10) is rated **115 V, 4.0 A, 460 W**. This corrects the previously recorded figure of 240 W / 2.1 A, which was in error.

Applying standard 125% continuous-load margin (the system is expected to run 6+ hours, which qualifies as a continuous load) yields a minimum design capacity of **5.0 A** — still a light load relative to any general-purpose relay, contactor, or SSR-style switching module, and well within the capacity of a standard NEMA 5-15 household receptacle (rated 15 A, 12 A continuous). Component selection for the switching device is not expected to be a significant constraint on its own; correct wiring, isolation (ER-001), and a normally-open fail state (TSR-003/2.2) matter more here than raw current capacity. This analysis is unaffected by the Boron-to-Muon controller change, since the Tanis load is switched externally to the controller in either case.

**Downstream effect:** this correction roughly doubles the current the Tanis branch draws from the shared 20A hangar circuit, which meaningfully tightens the Phase 2 cabin-heater headroom calculation — see Section 20.

---

## ER-003 — Non-invasive electrical installation

The hangar is a T-hangar with a single duplex (two-receptacle) 120 V outlet, standard NEMA 5-15, located on the pilot's side. The owner **cannot alter the hangar's electrical panel or existing wiring** — this is a hard constraint, not a preference.

All system electrical hardware shall therefore be installed as plug-in equipment that draws from the existing duplex outlet, rather than any hardwired connection to the hangar's branch circuit or panel. Additional outlets may be added only via a circuit-protected power strip plugged into the duplex outlet — this is an approved mechanism for expanding available outlets, not a wiring alteration.

A reasonable outlet allocation, subject to final design:

- **Duplex receptacle 1:** a relay/switching module (controlled by the controller) feeding the Tanis shore-power extension cord
- **Duplex receptacle 2:** a circuit-protected power strip, powering the controller's supply and any future low-draw hangar sensors/equipment (e.g., a Phase 2 cabin heater, subject to available headroom — see Section 20)

**Confirmed:** the duplex outlet is on a 20 A circuit breaker. The Tanis load (4.0 A continuous, ~5.0 A with margin per ER-002) uses a modest fraction of that. This still needs to be checked against the combined load once a cabin heater is added in Phase 2 (a typical 750–1500 W ceramic heater draws 6–12.5 A on its own) — see the revised headroom figure in Section 20.

**Extension cord recommendation:** per the Tanis guide's own cord-sizing table (Table 1), a **14-gauge (14/3), grounded, outdoor/garage-rated extension cord** (e.g., SJTW jacket) is the right choice here — it supports up to 15 A at 50 feet or 13 A at 100 feet, comfortably more than the corrected 4.0 A Tanis load requires, while still meeting the guide's minimum-gauge recommendation with margin to spare. This recommendation is unaffected by the ER-002 correction; the 14-gauge cord still has ample margin at 4.0 A. Choose a length that comfortably reaches from the relay-controlled outlet to the aircraft's Tanis plug (accessed through the cowling oil door, per Section 25 item 6) without excess slack pooling on the floor (measure the actual hangar run rather than guessing — 25 or 50 feet are common off-the-shelf lengths). A bright-colored jacket (orange/yellow) makes it easy to visually confirm at a glance whether it's connected, which is useful given TSR-003's reliance on the owner remembering to unplug it before flight.

---

## ER-004 — Controller power

The controller shall normally be powered from the hangar electrical system.

The controller should have battery backup sufficient to:

- Report a loss of primary power
- Continue communications for a useful period
- Detect and report restoration of power

**Controller change note (v0.5):** The Boron's onboard USB power and Li-Po charging circuit are being replaced by the Muon carrier board's equivalent power architecture. The Muon supports Li-Ion/Li-Po battery operation via a battery pack, and — unlike the Boron — Particle's guidance for the Muon/M-SoM power module notes it requires an actual USB-C port or USB-C power adapter to power the board and charge the battery; a USB-A-to-USB-C cable will not power the Muon or charge the battery, because the Muon uses USB-C Power Delivery to negotiate the supply voltage. This is a concrete installation detail: whatever powers the Muon in the hangar (e.g., a wall-wart plugged into the power-strip receptacle from ER-003) must be a genuine USB-C PD-capable supply/cable, not a repurposed USB-A charger with an adapter tip.

The Boron's rated operating range (-20°C to +60°C) was previously used to justify accepting a controller/charging fault below -20°C as a non-issue, since flying isn't desirable in that range regardless (see prior revision). The equivalent Muon/M-SoM operating range has not yet been confirmed and should be checked once the specific M-SoM module is selected (see Section 25); the same reasoning is expected to apply, but should not be assumed without checking the actual datasheet figures, the same way the BRN404X datasheet correction applied to the Boron.

---

## ER-005 — Controller enclosure and installation (new in v0.13)

**Location (decision):** the controller enclosure — housing the Muon/M404, both relay modules, the CT sensor(s), and the controlled outlets — shall be mounted on the **fixed hangar wall beside the man door**, next to the hangar light switch and the bi-fold door opener button.

Rationale:

- **RF:** the bi-fold hangar door is a large metal door, but the man door set into it appears to be fiberglass and is likely the hangar's main RF path for cellular signal. The 2026-10-13 signal test found the best signal quality near the door (Section 25, item 16).
- **Function:** this location puts the controller, the ARM/SAFE master-reset switch (FR-015a), and the lighting/occupancy sensing (FR-011, Section 16) together at the point where people enter and switch on the lights.
- The enclosure shall **not** be mounted on the man door or the bi-fold door itself. Door movement would flex the connected cords and could pull the enclosure loose.

**Enclosure (decision, see item 32):** the enclosure shall be a **listed polycarbonate electrical enclosure** (NEMA-rated), with listed receptacles mounted in it.

- Non-metallic material is required so the enclosure does not block the cellular antenna.
- A listed enclosure is recommended over a homemade wooden box because it is designed to contain a 120 V wiring or relay fault. Wood is combustible, and this enclosure sits in a hangar with fuel vapors and an aircraft nearby.
- Heat is not a significant factor. The selected relays are electromechanical (Appendix A.1), so internal heat is limited to relay coils and the controller.

**Mounting:** the enclosure may be attached with **magnets** to the steel wall. This satisfies ER-003's no-alteration constraint, since nothing is drilled or fastened to the hangar structure.

- Magnets shall be sized for **shear (sliding) load**, not their rated straight-pull force. Shear holding on painted or corrugated steel is commonly a fraction of the rated pull; as a design assumption, assume about 25% and size the magnets with margin above that.
- **Cord strain relief shall be independent of the enclosure** (e.g., a magnetic hook or cleat mounted below the box that the outgoing cords loop through), so a pulled or tripped-over cord loads the strain relief rather than the enclosure.
- A **short fall tether** shall connect the enclosure to a fixed point, so the box cannot fall to the floor if it is dislodged.

**Antenna placement:**

- The cellular antenna shall be mounted on the top or side of the enclosure facing the man door, with **at least 1–2 inches of standoff from the steel wall** or any other metal surface. An antenna mounted flat against steel is detuned and partially shadowed.
- Inside or on the enclosure, the antenna shall be placed at the **opposite end from the relays and AC wiring**.

**Inlet rating:** the enclosure's supply plug, cord, and internal AC wiring shall be rated for at least **15 A**. With the cabin heater at its 700 W limit (FR-006), maximum combined continuous load is about **9.8 A** (Tanis 4.0 A + heater 5.8 A + negligible controller draw), within the 12 A continuous rating of 15 A hardware.

**Verification before final installation (see open item 33):**

1. With the controller inside the finished enclosure and the antenna in its final position, confirm the signal at the mounting location (vitals refresh).
2. Run a **24–48 hour soak test** at that location with the hangar door closed. Pass criteria: no cloud disconnects, no unacknowledged messages, RSRQ remaining at −15 dB or better, and any carrier changes (AT&T ↔ T-Mobile) noted. No panic resets shall occur during the soak test.

---

# 12. Tanis Safety Requirements

## TSR-001 — No timer-based cycling

The control system shall not implement periodic cycling of the Tanis heater.

The Tanis operating guide explicitly states:

> "the preheat system is self-regulating by design; it is not to be cycled on and off, timers and thermostats are not to be used."

---

## TSR-002 — Pre-flight warm-up window (not indefinite continuous operation)

The system's intended operating profile is a **pre-flight warm-up window**: the owner remotely energizes the Tanis system several hours before departure, then flies shortly after arriving at the hangar, rather than leaving the system energized indefinitely while the aircraft sits idle.

This distinction matters for engine health, not just convenience. Extended energization without a subsequent flight — whether continuous over many days or repeatedly cycled — can allow moisture to collect in the crankcase and cylinders rather than being driven off by an actual flight. The intended workflow (energize, then fly) is specifically designed so that the heat generated is followed by operation that boils off any accumulated moisture, rather than leaving a warm, humid engine sitting unused.

The system shall still support leaving the Tanis system energized continuously for the duration of the warm-up window (per the Tanis guide's six-hour-or-longer recommendation) without cycling it on and off, consistent with TSR-001. What the system shall avoid is *encouraging or defaulting to* indefinite multi-day energization.

**Typical usage pattern:** the owner is normally about 45 minutes from the airport and generally starts preheating a few hours before departure. For early-morning departures, the owner may instead start the system the night before, going to bed with it running — meaning a normal, expected warm-up window can legitimately run overnight (roughly 8–12 hours), not just "a few hours."

The system shall notify the owner (see FR-013) if the Tanis system has remained continuously energized for longer than a configurable duration without an indication that the aircraft has flown. Given the legitimate overnight-start case above, this threshold shall default to on the order of 24 hours (configurable) so that normal overnight preheating does not generate a false warning, while still catching the case of a canceled/delayed flight left running for a day or more. This is a notification only — it shall not cause the system to automatically de-energize or cycle the heater, which would violate TSR-001.

---

## TSR-003 — Aircraft operation interlock

The system shall prevent normal aircraft operation while the Tanis system remains connected to shore power.

The Tanis documentation requires the preheat system to be disconnected before aircraft operation.

**This is resolved as procedural control:** the owner/pilot is responsible for physically disconnecting the shore-power extension cord from the aircraft's Tanis plug (accessed through the cowling oil door — see Section 25, item 6) as part of normal preflight procedures, consistent with the Tanis guide's own control model (Section C.2/C.3 — the system is activated and deactivated solely by connecting/disconnecting shore power) and consistent with existing preflight checklist practice. No electrical interlock component is required for this requirement to be satisfied.

The remote-control system's role is limited to remotely energizing/de-energizing shore power *to* the extension cord and reporting status — it does not attempt to enforce the physical disconnect itself. The system's engine-compartment sensor placement (FR-004, Section 16) provides an incidental secondary benefit here: because the sensor is mounted on the extension cord, its absence after disconnection is a further (non-safety-critical) indication that the cord has been removed, in addition to the required visual/procedural check.

---

## TSR-004 — Fault handling

If the system detects an abnormal condition such as loss of heat, electrical fault, smoke, or other evidence of malfunction, it shall provide an appropriate alert.

The Tanis guide instructs that a malfunction such as tripped circuit protection, smoke, or lack of heat requires disconnecting the system from power and inspection before flight.

The remote monitoring system shall not attempt to automatically reset a potentially failed Tanis system without a separately defined safety requirement.

---

# 13. Communications Reliability

## CR-001 — Cellular signal monitoring

The controller shall monitor cellular connectivity.

The system shall report when communications have been lost for longer than a configurable period.

---

## CR-002 — Remote command acknowledgment

Every remote command shall have an acknowledgment state.

Example:

**Command sent → Command received → Output changed → Output confirmed**

The system shall not report "preheater ON" merely because the user pressed an ON button.

---

# 14. Security

## SR-001 — Authorized control

Only authorized users shall be able to control the Tanis preheater or cabin heater.

## SR-002 — Command logging

All remote control commands shall be logged with:

- Time
- Command
- Result
- User/device where available

## SR-003 — Physical security

The controller and switching electronics should be installed in a secured enclosure.

The Boron documentation recommends using an enclosure to protect the module and baseboard rather than relying on conformal coating; the Muon's M-SoM module is subject to the same general guidance (moisture-sensitive components, buttons that cannot be usefully coated), so an enclosure remains the right approach rather than conformal coating regardless of which controller board is used.

---

# 15. Initial Hardware Architecture

The following is a proposed architecture, not yet a final component selection.

### Controller

**Particle Muon carrier board with M-SoM module (LTE Cat M1 or equivalent North American cellular module)**

Responsibilities:

- Cellular communications
- Sensor acquisition
- Control outputs
- Event logging
- Remote commands
- Notifications
- Local status

**v0.5 change:** This replaces the previously specified Particle Boron LTE (BRN404X), due to Boron order-fulfillment/delivery problems. The Muon is a carrier board (Raspberry Pi form factor) that accepts a separate M-SoM module providing the actual cellular/Wi-Fi/BLE/GNSS radio; the specific M-SoM model has not yet been finalized (see Section 25). Functionally, the controller's role in this architecture — cellular link, GPIO-driven relay control, sensor acquisition, and cloud connectivity for the remote dashboard — is unchanged. The Muon carrier additionally exposes LoRaWAN and Ethernet, and a 40-pin expansion (HAT-style) connector plus a QWIIC/I²C connector, which are not required by this design but may simplify sensor wiring compared to the Boron's more limited GPIO breakout.

The previously noted carrier support (AT&T, T-Mobile, and Verizon in the United States, with a note concerning Verizon enterprise plans) was specific to the BRN404X module and needs to be re-confirmed once the actual M-SoM model is selected, since carrier compatibility is a property of the cellular module, not the Muon carrier board itself.

**Observed carrier behavior (v0.13):** the M404 with its Particle SIM selects the carrier automatically and was observed on both **AT&T and T-Mobile** during testing, including a switch between them within one hangar session. The design shall not assume a single carrier. See Section 25, items 15–16.

**Installation:** see ER-005 for enclosure, mounting location, and antenna placement.

**Pin assignment (v0.6):** a full GPIO pin assignment against the Muon's 40-pin expansion header, along with specific component recommendations for the relay/sensor interface, is now defined in **Appendix A**.

### Inputs

Potential inputs:

1. Engine-compartment temperature sensor
2. Cabin temperature sensor
3. Hangar temperature sensor
4. Flood/water sensor
5. Motion/activity sensor
6. Lighting-state sensor
7. Tanis electrical/current-state sensor
8. Controller power-failure signal

### Outputs

Potential outputs:

1. Tanis preheater control
2. Cabin heater control
3. Local status indicator
4. Audible/local alarm, if desired

---

# 16. Sensor Placement Requirements

Sensor placement shall be treated as part of the system design rather than an afterthought.

### Engine sensor

The engine-compartment sensor shall be mounted to the Tanis shore-power extension cord near the plug end, so that it sits inside the cowl opening whenever the cord is connected and is automatically withdrawn when the cord is disconnected. This placement:

- Avoids any permanent attachment to engine components, so it cannot interfere with engine hardware or maintenance
- Avoids conflict with the cylinder-head temperature (CHT) probe boss, which may already be used both by an engine monitor and by the Tanis heating element itself (current Tanis TSP-series systems use a heated bolt that substitutes for a rocker cover screw or intake manifold bolt at that location)
- Cannot be forgotten in the cowling, since removing the cord removes the sensor
- Should be secured to the cord (e.g., taped or strain-relieved near the plug) so it does not become detached during routine plug/unplug handling
- Should be shielded or positioned so it is not resting directly against a hot element or exhaust component, since it is reading cowling ambient air temperature rather than a specific component's temperature

The cord (and sensor) enters the cowl through the small oil-filler door, which is also where the Tanis shore-power plug itself is accessed on this RV-10 (see Section 25, item 6).

**Sensor type recommendation (v0.6, interface updated v0.17):** see Appendix A — a waterproof DS18B20 1-Wire digital probe is recommended for this sensor, sharing a single 1-Wire bus with the cabin and hangar sensors. The bus is driven by a DS2484 I²C bridge on the QWIIC connector, not a GPIO pin.

### Cabin sensor

Now that the cabin heater's planned location is known (baggage compartment), the cabin sensor shall be positioned away from the baggage compartment specifically, in the main cabin area (e.g., near the panel or a front seat), so it represents general cabin temperature rather than reading warm air discharged directly from the baggage compartment heater.

### Hangar sensor

The hangar sensor should be located away from direct heater output, sunlight, doors, and other localized temperature sources.

### Flood sensor

The flood sensor should be positioned at the lowest practical location where water is expected to appear first. Per FR-008, this sensor serves as a local backup/confirmation signal rather than the primary early-warning mechanism, given the lead time required to fly the aircraft out ahead of river flooding.

### Activity sensor

The activity sensor should cover normal access paths and working areas while minimizing false triggers.

### Lighting sensor

The hangar lights were previously accessed with a 24-foot extension ladder, meaning they're mounted well out of easy reach. Rather than co-locating the light-level sensor at the fixture itself (which would require a ladder for any future maintenance, battery change, or adjustment), the sensor should be mounted at a lower, easily reachable point (e.g., wall- or post-mounted, angled to reliably pick up the change in ambient light when the hangar lights switch on/off) so routine upkeep doesn't require the extension ladder.

---

# 17. Alert Philosophy

The system shall distinguish between information and actionable alarms.

### Informational

Examples:

- Hangar temperature changed
- Preheater turned on
- Cabin heater turned on
- Person/activity detected

### Warning

Examples:

- Engine temperature has not increased sufficiently
- Cabin heater has been running longer than expected
- Cellular connection has been lost
- Hangar lights remain on after inactivity

### Critical

Examples:

- Flood detected
- Unexpected electrical condition
- Potential preheater malfunction
- Controller power failure combined with loss of monitoring capability

---

# 18. Suggested User Workflow

## Preflight heating

1. User decides to fly.
2. User opens the remote hangar-control interface.
3. User views current hangar and engine temperatures.
4. User commands Tanis preheat ON.
5. System confirms command receipt.
6. System confirms electrical/heating operation.
7. User optionally enables cabin heater (dial left at 700 W per FR-006's operating constraint).
8. System monitors engine and cabin temperature.
9. User receives notification when heating is progressing normally.
10. User travels to airport.
11. Before aircraft operation, the Tanis system is disconnected in accordance with aircraft/Tanis procedures (extension cord unplugged from the oil-door shore-power plug).

The Tanis guide recommends approximately six hours or longer for maximum benefit and says to verify operation after approximately 30 minutes by checking for warmth at the cylinder assemblies and crankcase.

---

# 19. Phase 1 — Minimum Viable System

The first implementation should include only the functionality necessary to prove the basic concept.

### Required Phase 1 features

- Cellular communications
- Muon/M-SoM controller
- Remote Tanis control
- Tanis electrical-state confirmation
- Engine temperature sensor
- Hangar temperature sensor
- Remote temperature display
- Basic notifications
- Controller power monitoring
- Local manual override

### Phase 1 success criteria

The owner can:

1. Leave the hangar.
2. Use a phone from outside the hangar.
3. Connect to the system through cellular communications.
4. Turn the Tanis system on remotely.
5. Confirm that it actually energized.
6. Observe engine temperature increasing.
7. Observe hangar temperature.
8. Receive an alert if heating does not behave as expected.

---

# 20. Phase 2 — Cabin Preheat

Add:

- Cabin temperature sensor
- Electric cabin heater control
- Cabin heater status
- Cabin temperature monitoring
- Heater safety timeout/interlock as appropriate
- Cabin-heating notifications

**Constraint carried over from Section 11 (ER-003), corrected in v0.7, finalized in v0.8:** the hangar's single duplex outlet is on a confirmed 20 A circuit, shared between the Tanis system, the controller, and the cabin heater. At 80% continuous-load derating, that circuit supports about 16 A continuous. The Tanis system draws 4.0 A (ER-002), leaving roughly **12.0 A of headroom** for the cabin heater and controller combined.

**Selected heater (v0.8):** 120 V, 12.5 A max, with three manual settings — 700 W (5.8 A), 900 W (7.5 A), and 1500 W (12.5 A). Checked against the 12.0 A headroom:

| Setting | Current | Combined with Tanis (4.0 A) | Within 16 A circuit limit? | Unattended use (v0.12) |
|---|---|---|---|---|
| 700 W | 5.8 A | 9.8 A | Yes — comfortable margin | **Allowed** |
| 900 W | 7.5 A | 11.5 A | Yes — but close to the 12 A continuous rating of 15 A plug/cord/power-strip hardware | Attended only |
| 1500 W | 12.5 A | **16.5 A** | **No — exceeds by 0.5 A** | Attended only |

**Decision (revised v0.12):** per FR-006, the heater shall be operated at 700 W whenever it may run unattended/remote-controlled, particularly when it may overlap with Tanis operation. The 900 W and 1500 W settings are reserved for attended use only. This is enforced by owner procedure — a dial setting plus a placard on the heater itself — not by the control system, for the reasons discussed in FR-006.

---

# 21. Phase 3 — Hangar Environmental Monitoring

Add:

- Flood sensor
- Flood alarm
- River-stage threshold monitoring (public gauge data) as the primary early-warning layer, per FR-008
- Historical environmental data
- More detailed temperature reporting

---

# 22. Phase 4 — Security & Activity

Add:

- Hangar activity sensor
- Entry/activity history
- FBO activity verification
- Occupancy state
- User/owner activity recognition

---

# 23. Phase 5 — Lighting Monitoring

Add:

- Hangar lighting-state detection
- Occupancy-aware lights-left-on detection
- Configurable exit/inactivity delay
- Lights-left-on notifications

---

# 24. Future Enhancements

Potential future features include:

- Graphical temperature history
- Preflight readiness status
- Estimated heating progress
- Multiple user accounts
- FBO/service activity annotations
- Remote camera integration
- Door-open monitoring
- Aircraft shore-power status
- AC power consumption monitoring
- Automatic maintenance/service logs
- Local touchscreen/status display
- Additional environmental sensors

These are intentionally not requirements for the initial system.

---

# 25. Open Questions

Items marked **✅ Resolved** are now settled. Remaining items should be obtained before further detailed design proceeds.

### Aircraft/Tanis

1. ✅ **Resolved** — Aircraft: Vans RV-10 (owner-built).
2. What exact Tanis system/kit model is installed? *(still open — electrical values are known, but not the specific model designation; low priority, not blocking)*
3. ✅ **Resolved (corrected v0.7)** — Voltage: 115 V.
4. ✅ **Resolved (corrected v0.7)** — Wattage: **460 W** (supersedes the previously recorded 240 W).
5. ✅ **Resolved (corrected v0.7)** — Current draw: **4.0 A** (supersedes the previously recorded 2.1 A).
6. ✅ **Resolved** — The Tanis shore-power plug is accessed through the small oil-filler door on the cowling. The extension cord (with the FR-004 sensor near its plug end) routes into the cowl through that same door opening.
7. ✅ **Resolved** — A Tanis power indicator light is already installed.
8. ✅ **Resolved** — Circuit protection is already installed as part of the Tanis system.
9. ✅ **Resolved** — Not a dedicated circuit; the Tanis system shares the hangar's single duplex outlet, on a confirmed 20 A breaker (see Hangar section below).
10. ✅ **Resolved** — A 14-gauge (14/3), grounded, outdoor-rated extension cord is recommended (see ER-003); owner will source and use one. Still adequate at the corrected 4.0 A draw.

### Controller (new in v0.5)

11a. What specific M-SoM module will be paired with the Muon carrier board (e.g., an LTE Cat M1 North American module)? ✅ **Resolved (v0.6)** — Particle M404 (North American LTE Cat M1 with 2G fallback, built-in GNSS) has been selected and ordered separately from the already-ordered bare Muon carrier board. See Appendix A for the pin assignment built around this module's Cortex-M33 GPIO map; final electrical re-verification against the M404 datasheet (GPIO drive current, operating temperature range) remains open until the module is in hand.

11b. ✅ **Resolved (v0.11)** — Owner already has a USB-C PD supply on hand and will power the Muon from it, plugged into the power-strip receptacle (ER-003). Satisfies ER-004's requirement for a genuine USB-C PD-capable source. Battery-pack operation remains available per ER-004 as backup, not primary power.

### Hangar

11. ✅ **Resolved** — Standard 120 V, NEMA 5-15 (ordinary 3-prong US household outlet).
12. ✅ **Resolved** — A single duplex (two-receptacle) outlet on a 20 A breaker, located on the pilot's side of the T-hangar. The hangar electrical panel/wiring **cannot be altered**; additional outlets may only be added via a circuit-protected power strip plugged into the existing duplex.
13. ✅ **Resolved** — 20 A circuit breaker.
14. ~~Electrical panel arrangement / location relative to aircraft~~ — **No longer relevant**, since panel/wiring alteration is out of scope by constraint.
15. ✅ **Resolved (updated v0.13)** — Cellular service is available. The original check was a T-Mobile phone showing good signal inside the hangar with the door shut. Confirmed with the actual Muon + M404 on 2026-10-13: the Particle SIM selects the carrier automatically and connected on **both AT&T and T-Mobile** inside the hangar. The single-carrier T-Mobile assumption is superseded.
16. ✅ **Resolved (v0.13)** — Measured with the Muon + M404 (Particle device vitals, LTE Cat-M1) on 2026-10-13:

| Position | Carrier | RSRP (strength) | RSRQ (quality) |
|---|---|---|---|
| Back of hangar, door open | AT&T | −100 dBm | −19 dB |
| Back of hangar, door closed | AT&T | −95 dBm | −16 dB |
| Halfway to door | AT&T (different cell) | −113 dBm | −20 dB |
| By the door | T-Mobile | −103 dBm | −12 dB |

For reference, readings at the owner's home were about −82 to −90 dBm / −10 to −14 dB. Observations: the link delivered every message without retransmission at all positions. The halfway position was worst and was followed by a dropped session and re-registration on T-Mobile. The door position had clearly the best signal quality, although it was the only position measured on T-Mobile, so location and carrier effects cannot be fully separated from this test. The door location was selected (ER-005), subject to the in-place soak test in item 33.
17. ✅ **Resolved (reframed)** — Airport is KSET, near the confluence of the Missouri, Mississippi, and Illinois rivers; flooding arrives from the north and would overwhelm the whole airport. The real requirement is early warning with enough lead time to retrieve and fly the aircraft out, not just point-level flood detection — see the updated FR-008 and the new Phase 3 river-stage monitoring item. The exact low point for the physical backup sensor is still to be selected on-site.
18. ✅ **Resolved** — Exact height unknown, but lights were previously accessed with a 24-foot extension ladder — informs sensor mounting choice (see Section 16, Lighting sensor).
19. ✅ **Resolved** — Light switch is located at the man door used to access the hangar.
20. ✅ **Resolved** — Not yet purchased; plan is a low-cost ceramic heater that activates automatically when power is applied (own thermostat, no smart control needed) — see updated FR-006.
21. ✅ **Resolved (v0.9, revised v0.12)** — Heater is rated 120 V, 12.5 A max, with three manual settings (700 W / 900 W / 1500 W). Checked against Phase 2 circuit headroom (Section 20): the 1500 W setting exceeds the shared circuit's continuous-duty capacity when combined with Tanis; resolved as a procedural constraint enforced via a placard on the heater rather than a hardware interlock. Limit for unattended/remote-controlled operation tightened from 900 W to **700 W** in v0.12 — see FR-006 and Section 20.
22. ✅ **Resolved** — Baggage compartment, using the existing cord/opening already run in for the trickle charger.

### Operations

23. ✅ **Resolved** — Typically a few hours before departure; for early-morning departures, the owner may start it the night before and leave it running overnight (~8–12 hours). House is ~45 minutes from the airport. See updated TSR-002 for how this shapes the "energized too long" notification threshold.
24. ✅ **Resolved (reframed)** — Not indefinite continuous operation; the intent is a pre-flight warm-up window (energize hours before departure, then fly), per TSR-002 in Section 12.
25. ✅ **Resolved** — The system notifies the owner if Tanis has been energized unusually long without a flight; it does not automatically de-energize or cycle (see TSR-002).
26. ✅ **Resolved** — A physical "master reset" at the hangar forces all controlled outlets off and requires a fresh remote command to re-energize anything; see FR-015a. *(v0.16: implemented as a latching ARM/SAFE switch rather than a momentary button.)*
27. ✅ **Resolved** — Owner proposes using phone location/geofencing (e.g., a >5-mile threshold) as the "hangar unoccupied" signal for the lights-left-on alert, rather than relying solely on an inactivity timer; see updated FR-012. Exact distance threshold and companion-app mechanism are implementation details for Phase 5.
28. ✅ **Resolved (candidate approach)** — Owner's phone presence (via the same location/companion-app mechanism as item 27) is proposed as the "known owner" signal, rather than camera or biometric identification; see updated FR-009. Won't identify people who enter without the phone (e.g., FBO staff) — acceptable, since FR-010 doesn't require positively identifying the FBO either.

### Wiring/component selection (new in v0.6)

29. ✅ **Closed (v0.14)** — hiBCTR 2-pack, 5V-trigger, 30A single-channel relay modules (electromechanical, opto-isolated input) purchased (v0.10), one per branch, as an accepted deviation from Appendix A's original SSR recommendation. Both open sub-items were verified on hardware on 2026-10-26 with the Tanis module on D2: **(a) 3.3V trigger** — the Muon's 3.3V GPIO reliably switches the relay when module VCC is on 5V; **(b) trigger polarity** — jumper set to **active-high**, matching firmware (HIGH = on). With a 15 kΩ pull-down on IN, the relay stays off through power-up, reset, and OTA. See Appendix A.1 for the verified configuration. The cabin-heater module (D3) was configured identically and passed the same tests on 2026-10-27 (v0.15); no relay items remain open.
30. ✅ **Resolved (v0.10), with build tasks remaining** — SCT-013-030 split-core current transformers (Sazkjere 4-pack) purchased for Tanis electrical confirmation (FR-003), replacing the originally-planned AC-presence module. See Appendix A.1 for the rationale and A.2 for the analog pin assignment. Remaining before this can be wired in: design/verify the bias circuit centering the CT's output within the Muon ADC's 0–3.3V range, and calibrate the ADC reading against the Tanis branch's known ~4.0A draw (Appendix A.3).
31. ✅ **Closed (v0.16)** — Master-reset switch. *v0.11 history:* a Daiertek 12V 19mm momentary SPST-NO pushbutton was selected and wired in series with the relay-coil 5V rail; this was a design error (a normally-open momentary in series leaves the relays unpowered except while held, and no momentary switch can hold outlets off if firmware is hung). *v0.16 resolution:* replaced with an on-hand **19 mm 12 V illuminated latching push-on/push-off button** (switch rated 1 A; LED isolated from the switch). The switch contacts (pigtail leads labeled "NO" and "NC", verified by continuity to close when latched) are in series with the relay-coil 5V rail; the LED is powered from the switched rail so it lights only when ARMED. D4 senses the switched rail through a divider. Bench-verified 2026-10-27 (see Appendix A.1). Remaining build task: fit a 10k top resistor in the divider before installation (Appendix A.3).

34. ✅ **Resolved (v0.17), with bench test remaining** — Temperature-probe bus interface. Direct GPIO 1-Wire does not work on the M404: probes answered presence but all reads returned 1s, consistent with Particle's statement that RTL872x GPIO is too slow to bit-bang 1-Wire. Owner approved (2026-10-27) an **Adafruit DS2484 I²C-to-1-Wire bridge** on the QWIIC connector. Header pin 29 is freed. Remaining: bench-test once the part arrives (Appendix A.3).

### Installation (new in v0.13)

32. ✅ **Resolved (v0.13)** — enclosure material: **listed polycarbonate enclosure** (owner decision, 2026-10-13), replacing the original plan for a homemade wooden box. Specific enclosure model still to be selected.
33. **Open — in-place signal soak test.** Run the 24–48 hour soak test defined in ER-005 with the controller in its final enclosure, location, and antenna position before final installation.

---

# 26. Initial Design Constraint

The most important constraint established by the supplied documentation is:

**The remote-control system is controlling the electrical connection to the Tanis system; it must not attempt to turn the Tanis heater itself into a thermostatically or timer-controlled heater.**

The Tanis system is designed to be self-regulating and continuously powered while connected to appropriate ground power.

This distinction should remain explicit throughout the electrical, software, and user-interface design.

---

# 27. Project Goal

The completed system should make it possible for the aircraft owner to leave home, remotely initiate aircraft preheating at the hangar, verify through temperature and electrical feedback that the aircraft is actually warming, optionally warm the cabin, and receive useful environmental/security notifications — all without requiring Wi-Fi at the hangar.

The system should provide **confidence rather than merely remote control**: the owner should be able to determine not only that a command was sent, but that the intended equipment responded and that the aircraft environment is actually warming.

---

# Appendix A. GPIO Pin Assignment — Muon 40-Pin Header (v0.6)

This appendix defines a specific pin-by-pin wiring plan against the Muon carrier board's fixed 40-pin expansion header. **The header pinout itself is a property of the Muon carrier board, not the M-SoM** — so this plan does not have to wait for the M404 to arrive, and can be used to bench-wire and test relays/sensors ahead of time (see the "what can proceed while waiting for the M404" discussion earlier in this project's history).

Facts below (pin numbers, net names, reserved functions) come directly from Particle's published Muon datasheet. Component choices (DS18B20, relay modules, etc.) reflect what's been selected or purchased so far — see the open items in Section 25 (items 29–31) for what's still unverified or unselected.

**Note on the screw-terminal breakout HAT:** the HAT's terminal labels use Raspberry Pi BCM GPIO numbering, not Particle pin names. Examples: the HAT terminal "IO17" is header pin 11, which is Muon **D2** (verified v0.14); "IO16" is header pin 36, which is Muon **D3** (verified v0.15); "IO13" is header pin 33, which is Muon **D4** (verified v0.16). Always map HAT labels to header pin numbers, then to the Muon pin names in A.2.

## A.1 Recommended component choices and rationale

**Temperature sensors — selected (v0.10): DROK 5-pack waterproof DS18B20 probes, all on one shared 1-Wire bus.**
Rather than three separate analog thermistor circuits (one GPIO/ADC channel each), a single digital 1-Wire bus hosts the engine, cabin, and hangar temperature sensors together, each identified by its own factory-programmed 64-bit ROM ID. This is a standard choice for exactly this situation — a sensor at the end of a long cable run (the engine sensor rides on the Tanis extension cord) — because the 1-Wire digital signal tolerates cable length and electrical noise far better than a raw analog voltage-divider thermistor would. The purchased 5-pack covers the 3 needed (engine, cabin, hangar) with 2 spares.

**1-Wire bus interface — changed (v0.17): Adafruit DS2484 I²C-to-1-Wire bridge (product 5976) on the Muon's QWIIC connector, replacing direct GPIO 1-Wire.**

- **Why:** bench test on 2026-10-27 with three probes on header pin 29 (A0/D19), a 4.7k pull-up, and the OneWire library (built for Device OS 5.9.0, because the library does not compile on 6.x): the probes answered the reset pulse (presence detected), but every bit read returned 1 (READ ROM = FFFFFFFFFFFFFFFF) and the address search found no devices. Particle has stated that RTL872x-based modules (the M404's MCU family) cannot bit-bang 1-Wire because their GPIO is too slow for the protocol's timing. This is a hardware limit, not a wiring or library-setting problem.
- **What the bridge does:** the DS2484 generates the 1-Wire timing in hardware; the Muon talks to it over I²C. It connects to the QWIIC port with a STEMMA QT/Qwiic cable (no soldering, no GPIO pin), has screw terminals for the probe leads, and shares the I²C bus with the planned light sensor at a different address. It also drives the bus more strongly than a GPIO pin, which helps the long cable run to the engine probe.
- **Evidence it works on this MCU family:** in the Particle community, a DS18B20 was reported working through a DS2484 on a P2 (RTL872x), and Particle stated its DS2482-RK library works on RTL872x devices. Neither has yet been verified on the M404 (Appendix A.3).
- **Bench-verified so far:** the three probes are wired correctly (yellow = data, red = VCC, black = GND) and respond with presence pulses. Probe leads move from the DROK adapter to the DS2484's terminals; the DROK adapter and its 4.7k pull-up are no longer used.
- **Production firmware** shall not depend on the unmaintained OneWire library or on targeting Device OS 5.9.0.

**Relay/switching modules — selected (v0.10): hiBCTR 2-pack, 5V-trigger, 30A single-channel relay module (electromechanical, opto-isolated input), one per branch.**

This section originally recommended solid-state relays (SSRs) over electromechanical relays, on the reasoning that SSRs commonly accept a 3–32VDC control signal directly from the Muon's 3.3V GPIO with no separate driver stage, provide inherent isolation, and have no mechanical contacts to wear out over repeated 8–12 hour duty cycles across a flying season.

The part actually purchased is an electromechanical relay, not an SSR — its Normally-Open/Common/Normally-Closed terminal layout is the tell, since a true SSR has no NC terminal to offer. This is accepted as a reasonable substitution rather than reworked, for three reasons:

- **Load margin is generous either way.** 30A against a 5.0A (Tanis, ER-002) or 12.5A (heater) load leaves large headroom regardless of relay technology.
- **Mechanical wear is not a practical concern at this duty cycle.** Relays of this class are typically rated for 100,000+ mechanical cycles; at roughly one switching cycle per flight day, the SSR's no-moving-parts advantage doesn't materially change the system's expected service life.
- **Isolation (ER-001) is preserved.** The opto-isolated input plus the relay's physical coil/contact separation provide the same galvanic isolation an SSR would.

**Bench verification (v0.14, 2026-10-26) — both pre-installation items closed for the Tanis module (D2).** Tested with bench-test firmware on the actual Muon + M404, with a lamp as the load.

Verified configuration — this is the required configuration for **both** relay modules:

| Item | Required setting | Why |
|---|---|---|
| Module VCC (DC+) | Muon **5V** (header pin 2 or 4) — through the ARM/SAFE switch (FR-015a) | The relay coil does not pull in at 3V3. Observed: with VCC on 3V3, 3.3 V reached IN but the relay did not switch. |
| Module GND (DC−) | Muon GND | Common reference for the trigger signal |
| Trigger jumper | **Active-high** | Observed: in the active-low position, the outlet energized on power-up with an unconfigured controller, which violates 2.2. Active-high means a LOW, floating, or dead controller output = relay off. |
| IN pull-down | **15 kΩ from IN to the module's GND terminal** (any value from about 4.7–47 kΩ is acceptable) | Holds IN low before firmware runs and if the signal wire comes loose. GPIO load is about 0.22 mA. |
| Firmware | Drive the relay pin LOW in a `STARTUP()` function and again in `setup()`; HIGH = on | Earliest point user firmware can set a known state |
| Load wiring | COM → NO (hot conductor) | See wiring requirement below |

Test results (all passed):

1. **3.3V trigger:** `on`/`off` commands and a 5-cycle on/off test switched the relay cleanly from the Muon's 3.3 V GPIO.
2. **Power-up/reset:** repeated power cycles and resets — the relay never energized.
3. **OTA:** a firmware reflash over cellular — the relay stayed off throughout.

**Cabin-heater module (D3) — verified v0.15, 2026-10-27:** configured per the table above and passed the same three tests. The test firmware held the Tanis relay (D2) LOW throughout, and only the cabin outlet switched.

The bench-test firmware includes a 10-minute auto-off as a guard for an unattended bench lamp only. Production Tanis firmware shall not have a timed auto-off (TSR-001, TSR-002, Section 26).

**Wiring requirement — use NO, not NC:** the module exposes Normally-Open (NO), Common (COM), and Normally-Closed (NC) load terminals. The AC load **must be wired through NO and COM**, never NC. Wiring into NC would invert the fail-safe behavior required by TSR-003/2.2 — the relay would default to energized (load ON) on loss of control power or a dead Muon, rather than de-energized (load OFF) as required.

**Tanis electrical-state confirmation (FR-003) — selected (v0.10): SCT-013-030 split-core current transformer, clamped around the Tanis branch's hot conductor downstream of the relay.**

This replaces Appendix A's original recommendation of a simple opto-isolated AC-presence module. The part purchased (Sazkjere 4-pack, SCT-013-030, 30A) is a non-contact clamp-on CT: it reads current magnitude by induction, with no electrical connection to the AC conductor at all — which is arguably a *better* fit for ER-001's isolation principle than a conductively-tapped presence module would have been, since there's no galvanic connection to isolate in the first place.

It's also a capability upgrade over the original plan: instead of a binary "AC present / not present" signal, it gives an actual current reading, so the controller can distinguish a fully-working Tanis branch (current near the expected 4.0 A) from a stuck-open relay (near 0 A) or a partially-failed heating element (anomalously low but nonzero current) — all of which a simple presence module would have reported identically as "something is or isn't on."

The trade-off is wiring and firmware complexity: this needs an **analog** input pin (not digital), a **bias circuit** to center the CT's AC output voltage within the Muon ADC's 0–3.3V range (the SCT-013-030's 30A/1V variant has a built-in burden resistor, but its output swings around 0V and needs a resistor-divider bias network, e.g. two resistors from 3.3V to ground with the CT signal summed in, to sit entirely within the ADC's positive input range), and firmware that samples across an AC cycle and computes an RMS or peak-to-peak current value rather than just reading a digital pin. This is real added work relative to the original plan, but it was already purchased, and the capability gain is worth it for a part doing double duty as both FR-003's confirmation sensor and, if extended to the cabin-heater branch, a path toward the "AC power consumption monitoring" item already listed under Future Enhancements (Section 24).

Two spare CT sensors are on hand. Extending current sensing to the cabin-heater branch (same bias-circuit and firmware pattern, one more analog pin) is a low-cost option now rather than a future purchase — this is offered, not required; it isn't needed to satisfy any current requirement, since FR-003's confirmation duty applies to Tanis specifically.

See Section 25, item 30, for what's still open before this is ready to wire in.

**Light-level sensor — I2C digital lux sensor (e.g., BH1750) on the QWIIC connector, not a raw photoresistor.**
The Muon's QWIIC connector is wired directly to the I2C bus (pins 3 and 5 on the header). A digital I2C lux sensor plugs in without needing a separate GPIO pin or an analog voltage-divider circuit, and gives an actual light-level reading rather than an uncalibrated voltage.

**Master reset (FR-015a) — selected (v0.16): 19 mm 12 V illuminated latching push-on/push-off button, used as an ARM/SAFE switch. Hybrid hardware + software design.**

*v0.16 correction:* v0.11 specified a Daiertek momentary SPST-NO pushbutton in series with the relay 5V feed. That was wrong: a normally-open momentary in series keeps the relays unpowered except while held, and no momentary switch can keep outlets off in hardware if the firmware is hung. Replaced with a latching switch (owner decision, 2026-10-27).

To satisfy FR-015a's requirement that the override work "regardless of the controller's software state," the switch is not *purely* a GPIO input that firmware interprets. Instead:

- **Hardware path (the safety mechanism):** the switch contacts are wired **in series with the 5V supply feeding both hiBCTR relay modules' VCC**. Latched = closed = ARMED; unlatched = open = SAFE. In SAFE, neither relay coil can be powered no matter what the GPIO outputs are doing or whether the Muon is running.
- **Indicator:** the button's LED is isolated from its switch. LED + is connected to the switched rail and LED − to GND, so the LED lights only when ARMED (the manufacturer's "LED on with switch" wiring). The 12 V LED is dimmer at 5 V but clearly visible.
- **Sense path (logging/lockout only):** a resistor divider from the switched rail to GND feeds D4 (header pin 33, HAT "IO13"). D4 HIGH = ARMED, LOW = SAFE. Specified values: **10 kΩ top, 15 kΩ bottom (~3.0 V)** — under the Muon's 3.3 V GPIO ceiling with margin above a typical ~2.3 V logic-HIGH threshold (0.7 × 3.3 V; the M404's exact figure is not yet confirmed from its datasheet). The bench build used 15k/15k (~2.5 V); it worked, but has little margin, so fit a 10k top resistor before installation. A misread at low margin fails safe (firmware treats it as SAFE).
- **Firmware behavior:** on a transition to SAFE (50 ms debounce), drive both relay pins LOW, clear the commanded state, log and publish the event, and reject "on" commands while SAFE. On return to ARMED, publish the event and leave both outlets off until a new remote command.

Wiring:

```
Muon 5V (pin 2/4) ── switch lead "NO"
switch lead "NC" ──┬── Tanis relay VCC
                   ├── Cabin relay VCC
                   ├── LED +        (LED − to GND)
                   └── 10k ──┬── D4 / IO13 (pin 33)
                             └── 15k ── GND
```

(The pigtail labels "NO" and "NC" are the two switch leads; continuity confirmed they close when latched and open when unlatched.)

**Bench verification (v0.16, 2026-10-27) — all passed**, with lamp loads on both outlets:

1. ARMED: both outlets switch on by command.
2. Unlatch to SAFE: both outlets off immediately; SAFE event published.
3. While SAFE: "on" commands rejected (return −2); outlets stay off.
4. Re-latch to ARMED: outlets stay off until commanded; ARMED event published.
5. Hardware proof: with firmware set to ignore the switch, SAFE still cut both outlets; re-arming restored them (showing why the firmware lockout is also needed).
6. Power-cycle while SAFE: both outlets stay off; state reports SAFE.

## A.2 Pin assignment table

| Header pin | Muon pin name | Function used | Assignment |
|---|---|---|---|
| 3 | D0 (SDA) | I2C data | QWIIC bus — DS2484 1-Wire bridge (temperature probes, v0.17) and light-level sensor (BH1750 or equivalent) |
| 5 | D1 (SCL) | I2C clock | QWIIC bus — DS2484 1-Wire bridge and light-level sensor |
| 11 | D2 | Digital out | **Relay 1 control** — Tanis relay (hiBCTR module, via NO/COM). Active-high, 15 kΩ pull-down IN→GND. HAT label "IO17". **Bench-verified v0.14.** |
| 36 | D3 | Digital out | **Relay 2 control** — cabin-heater relay (hiBCTR module, via NO/COM). Active-high, 15 kΩ pull-down IN→GND. HAT label "IO16". **Bench-verified v0.15.** |
| 33 | D4 | Digital in (divided sense) | Master-reset ARM/SAFE sense — divider (10k/15k) from the switched relay 5V rail. HIGH = ARMED, LOW = SAFE. HAT label "IO13". Logging/lockout tap only, not the safety path. **Bench-verified v0.16.** |
| 32 | D5 | Digital in | Flood/water sensor |
| 12 | D6 | Digital in | Activity/motion (PIR) sensor |
| 29 | A0/D19 | — | **Free (v0.17).** Previously the GPIO 1-Wire bus; the temperature probes moved to the DS2484 I²C bridge because RTL872x GPIO cannot bit-bang 1-Wire (A.1). HAT label "IO5". *Pin name corrected 2026-10-27: earlier versions listed "D18/A0"; per the Muon datasheet, header pin 29 is A0/D19 (D18 is A1, header pin 31).* |
| 24 | D29/A6 | Analog in | Tanis current transformer (SCT-013-030, via bias circuit) — FR-003 |
| 26 | D17/A2 | Analog in | *(optional)* Cabin-heater current transformer, if extended — see Appendix A.1 |
| 2, 4 | 5V | Power | **hiBCTR relay module VCC** (both modules, through the ARM/SAFE switch) — required; the relay will not pull in from 3V3 (verified v0.14) |
| 1, 17 | 3V3 | Power | Sensor supply and pull-ups as needed (check each sensor's supply voltage separately); **not** for relay module VCC |
| 6, 9, 14, 20, 25, 30, 34, 39 | GND | Ground | Common ground for all sensors/relays |

*v0.14 correction:* earlier versions listed the power pins as a single row ("1, 4, 17 — 3V3 / 5V"). Pins 1 and 17 are 3V3; pins 2 and 4 are 5V.

**Pins deliberately avoided**, and why:

- **D9/TX, D10/RX (pins 8, 10):** left free in case a debug console or a UART-based module is useful later.
- **D11/MISO, D12/MOSI, D13/SCK (pins 21, 19, 23):** reserved internally for SPI (the Muon's onboard Ethernet controller uses this bus) — not available as general GPIO.
- **Pin 40 (D20):** previously reserved for a digital AC-presence sense signal (superseded in v0.10 by the analog CT sensors above) — now free for future use.
- **D29/A6, D17/A2 (pins 24, 26) — reassigned in v0.10:** these were originally left free as a margin against the Muon's optional internal SPI2 function, in case a future need arose. The v0.10 CT sensors now use them as analog inputs, since this design has no SPI2 need. If a future addition requires SPI2, these two analog assignments would need to move to different ADC-capable pins.
- **D14/A5, D27, D24, D25 (pins 13, 15, 16, 18):** boot-mode-sensitive pins (pull-up/pull-down states or boot-triggered functions per the datasheet) — avoided entirely to prevent interfering with the module's boot process.
- **Pins 27–28 (GPIO0/GPIO1, "ID_SD"/"ID_SC"):** not connected on the Muon — reserved by the Raspberry Pi HAT ID-EEPROM convention, unusable regardless.

## A.3 Still open before this can be finalized

- ✅ ~~Bench-test the hiBCTR relay modules' trigger input against actual Muon GPIO output (3.3V)~~ — **done for the Tanis module (v0.14)**; reliable with module VCC on 5V (Section 25, item 29).
- ✅ ~~Confirm the hiBCTR modules' trigger-polarity jumper setting~~ — **done (v0.14)**: active-high, firmware HIGH = on (Section 25, item 29).
- ✅ ~~Configure and test the cabin-heater relay module (D3) identically to the Tanis module~~ — **done (v0.15)**: same configuration, all tests passed.
- **Bench-test the DS2484 1-Wire bridge (v0.17):** connect via QWIIC, move the three probe leads to its terminals, confirm all three probes are found and read plausible temperatures, record each probe's ROM address and label it ENGINE / CABIN / HANGAR, and confirm the library used works on Device OS 6.x (Section 25, item 34).
- **Design and verify the CT bias circuit** *(paused 2026-10-27 pending a 10 µF capacitor; bench sketch ready)* so the SCT-013-030's AC output sits entirely within the Muon ADC's 0–3.3V input range, and confirm the Muon's analog input doesn't exceed 3.3V at the Tanis branch's expected peak current (Section 25, item 30).
- **Calibrate the CT reading against known current** — bench-test against the Tanis branch's expected ~4.0A RMS (and the heater's settings, if extended) to establish the ADC-reading-to-amperage conversion the firmware will use (Section 25, item 30).
- ✅ ~~Master-reset switch wiring and divider~~ — **done (v0.16)**: latching ARM/SAFE switch bench-verified (Section 25, item 31). **Remaining before installation:** replace the bench divider's top 15k with a **10k** (~3.0 V at D4), and confirm D4 reads ≥2.4 V with both relay coils energized. Confirm the M404's GPIO input-HIGH threshold from its datasheet when convenient.
- Once the M404 module is in hand, re-verify this pin table against its actual GPIO map — the header pinout shown here is fixed by the Muon carrier board, but Particle's documentation ties specific header pins back to specific M.2 SoM pins (shown in the "M.2 Pin" column of the underlying datasheet), so it is worth a final cross-check rather than assuming no change. *(Partially confirmed: header pin 11 = D2 (v0.14), header pin 36 = D3 (v0.15), and header pin 33 = D4 (v0.16) work correctly.)*
