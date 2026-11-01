# Hangar Remote Monitoring & Preheat Control System
## PRD v0.12 — Proposed Addition: Section 9A, Command & Notification Interface

**Status:** DRAFT. Not merged into the PRD. For owner review and approval.
**Base document:** PRD v0.11
**Date:** 2026-10-13

---

## Proposed revision note (v0.12)

Defines the remote command and notification interface that FR-013, FR-014, FR-015, CR-002, SR-001, and SR-002 require but did not specify. This resolves Section 3's undefined "remote notification and control software" item.

The architecture is:

- **Control:** iOS Shortcuts on the owner's iPhone call the Particle Cloud API directly.
- **Alerts:** the device publishes events, and a Particle webhook forwards them to Pushover.

There is no SMS channel and no custom mobile app.

All safety logic stays on the controller. The phone side is a thin, replaceable front end.

This revision adds one failure mode not previously captured: **Tanis current present while commanded OFF.** That indicates welded relay contacts, which is possible with the electromechanical relay selected in v0.10.

---

## 9A.1 Decisions (proposed)

| ID | Decision | Alternatives considered | Rationale | Reversibility |
|---|---|---|---|---|
| DEC-IF-01 | Control via iOS Shortcuts calling the Particle Cloud API | SMS; Particle Console/app; Blynk-type dashboard service; custom native app | Shortcuts give home-screen buttons and Siri with no app development and no added service. Reports indicate the Particle SIM does not support SMS. A native app is unjustified for a single user. | High. The front end can be replaced without firmware changes. |
| DEC-IF-02 | Alerts via a Particle webhook to Pushover | ntfy; SMS via a paid gateway; email | Reliable iOS push for a one-time app purchase. Its priority levels map onto Section 17's alert tiers, and its emergency priority repeats until acknowledged, which matches FR-008. | High |
| DEC-IF-03 | The controller is the sole authority for safety behavior | Logic split between phone and cloud | The cabin-heater timeout, the master reset, and fault detection must work even when the phone, the cellular link, or the cloud is unavailable (Section 2.2). | Low. This is architectural. |
| DEC-IF-04 | Single user with a single API token; multi-user deferred | Separate tokens or accounts per user | Single pilot. If a second user is added later, the Shortcut can be shared without changing the architecture (see IF-SEC-03). | High |

---

## 9A.2 Architecture

```
iPhone Shortcuts ──HTTPS──▶ Particle Cloud API ──cellular──▶ Muon/M404 (Hangar_Mate)
                                                                  │
iPhone Pushover ◀──HTTPS── Pushover API ◀── Particle Webhook ◀── events (hm/...)
```

- **Commands** are Particle cloud functions on the device.
- **Current state** is a single Particle cloud variable (`status`) that the device keeps updated.
- **Alerts** are private Particle events. One webhook matching the `hm/` prefix forwards them to Pushover. The Pushover credentials live in the webhook configuration, not in firmware.

---

## 9A.3 Command Set (Particle cloud functions)

### Correction to earlier discussion: no Tanis duration argument

TSR-001, TSR-002, and Section 26 prohibit timer control of Tanis. TSR-002 makes the long-run case a notification only, never an automatic de-energize. The Tanis command therefore takes **no duration and has no auto-off**. The cabin heater does have a timeout, as FR-006 requires.

| Function | Argument | Action | Requirement |
|---|---|---|---|
| `tanis` | `on` \| `off` | Energize or de-energize the Tanis relay. No timer. | FR-002, FR-015, TSR-001/002 |
| `cabin` | `on` \| `on,<min>` \| `off` | Energize the cabin-heater relay for `<min>` minutes, or the default duration if omitted. The duration is clamped to the configured maximum. | FR-006, FR-015 |
| `alloff` | (none) | De-energize all controlled outlets. This is the remote equivalent of the FR-015a end state, but it does not replace the physical button. | FR-015 |
| `ack` | `<alarm>` | *Reserved, Phase 3.* Acknowledge a latched alarm (FR-008). | FR-008 |

### Function return codes

The Particle API returns an integer from each function call.

| Code | Meaning |
|---|---|
| `1` | Command accepted and the output changed |
| `0` | Command accepted, but the output was already in the requested state |
| `-1` | Invalid argument |
| `-2` | Rejected: relay coil power is absent (master reset held, or rail lost; Appendix A.1) |

The return code only confirms that the device received the command. **It is never displayed as "ON."** See IF-003.

---

## 9A.4 State Reporting (`status` variable)

A compact JSON string that the device updates on every state change and on each sensor cycle. Illustrative content:

```json
{"t":"2026-11-02T06:14Z",
 "tanis":{"st":"CONFIRMED_ON","a":4.0,"since":"05:02Z"},
 "cabin":{"st":"OFF","rem":0},
 "temp":{"eng":71,"cab":38,"hgr":34},
 "rst":0,"pwr":"AC","batt":98,"sig":"good","fw":"0.1.0"}
```

### Output states (FR-003)

| State | Meaning |
|---|---|
| `OFF` | Commanded off, and current is below the off threshold |
| `COMMANDED_ON` | Relay commanded on, and the confirmation window has not yet elapsed |
| `CONFIRMED_ON` | Commanded on, and CT current is within the expected band |
| `FAULT_NO_CURRENT` | Commanded on, but current is still below threshold after the confirmation window |
| `FAULT_LOW_CURRENT` | Commanded on, but current is nonzero and below the expected band (possible partial element failure) |
| `FAULT_STUCK_ON` | Commanded off, but current is present (welded relay contacts). **Critical.** |
| `UNAVAILABLE` | Determined phone-side when the API call fails or times out. The device cannot report this state itself. |

The Tanis states use the CT on pin 24. The cabin-heater current states apply only if the optional second CT (pin 26) is installed. Otherwise the cabin heater reports command state plus the temperature trend.

---

## 9A.5 Phone Interface (iOS Shortcuts)

| Shortcut | Behavior |
|---|---|
| **Preheat On** | Calls `tanis on`, waits for the confirmation window, reads `status`, then displays the result. Example: "Tanis CONFIRMED ON — 4.0 A — engine 34°F / hangar 33°F". |
| **Preheat Off** | Calls `tanis off`, waits for the confirmation window, reads `status`, then displays the result. |
| **Cabin Heat On** | Prompts for minutes, defaulting to the configured default. Calls `cabin on,<min>`, waits for the confirmation window, reads `status`, then displays the result. |
| **All Off** | Calls `alloff`, waits for the confirmation window, reads `status`, then displays the result. |
| **Hangar Status** | Reads `status` and displays a one-screen summary: states, temperatures, engine-minus-hangar delta, heater time remaining, power source, signal. |

Each control Shortcut can be invoked by a Siri phrase (for example, "Preheat the airplane").

On API failure or timeout, every Shortcut displays **"Controller unreachable — state unknown."** It must never show a last-known state as if it were current.

---

## 9A.6 Requirements

| ID | Requirement | Traces to |
|---|---|---|
| IF-001 | The controller shall expose remote control solely through the cloud functions defined in 9A.3. | FR-015 |
| IF-002 | The controller shall maintain a `status` variable containing all fields in 9A.4. It shall update the variable within 2 s of any output state change, and at least every sensor-logging interval otherwise. | FR-003, FR-014 |
| IF-003 | No phone interface element shall display an output as ON based on a command result alone. ON shall be displayed only from a `status` read showing `CONFIRMED_ON`, or for the cabin heater without a CT, `COMMANDED_ON` labeled as unconfirmed. | CR-002, FR-015 |
| IF-004 | The controller shall evaluate Tanis CT current within a confirmation window after each Tanis command (default 10 s, configurable). It shall set the resulting state and publish the corresponding event. | FR-003, CR-002 |
| IF-005 | The controller shall detect `FAULT_STUCK_ON` continuously, not only after commands, and shall publish a Critical event. | TSR-004, ER-001 |
| IF-006 | The cabin-heater on-duration shall be enforced on the controller. It shall have a configurable default and a configurable hard maximum that no command argument can exceed. | FR-006 |
| IF-007 | The `tanis` command shall not accept a duration, and the controller shall never de-energize Tanis on a timer. | TSR-001, TSR-002, §26 |
| IF-008 | Commands shall return `-2` while relay coil power is absent. After coil power is restored, no output shall re-energize without a new command. | FR-015a |
| IF-009 | Every command shall be logged with time, function, argument, return code, and resulting state. | SR-002 |
| IF-NOT-01 | The controller shall publish the events in 9A.7 as private Particle events with the `hm/` prefix. | FR-013 |
| IF-NOT-02 | Each event shall carry its alert tier (Section 17), and the webhook shall map the tier to a Pushover priority as defined in 9A.7. | FR-013, §17 |
| IF-NOT-03 | Every notification shall include the event type, the time, and the relevant readings. | FR-013 |
| IF-SEC-01 | API access shall use a dedicated Particle access token created only for the Shortcuts. It shall be revocable without affecting other access. | SR-001 |
| IF-SEC-02 | All events shall be published private, never public. | SR-001 |
| IF-SEC-03 | If a Shortcut is ever shared, the token shall be supplied through the Shortcut's import question at install time, not embedded in the shared link. | SR-001 |

---

## 9A.7 Event Catalog

| Event | Tier | Pushover priority | Trigger | Phase |
|---|---|---|---|---|
| `hm/tanis/confirmed` | Info | 0 | Tanis reached `CONFIRMED_ON` | 1 |
| `hm/tanis/off` | Info | -1 (quiet) | Tanis confirmed off | 1 |
| `hm/tanis/fault` | Critical | 1 | `FAULT_NO_CURRENT` or `FAULT_LOW_CURRENT` | 1 |
| `hm/tanis/stuck` | Critical | 2 (repeats until acknowledged) | `FAULT_STUCK_ON` | 1 |
| `hm/tanis/longrun` | Warning | 1 | Tanis energized longer than the configured threshold (default 24 h). Notification only. | 1 |
| `hm/eng/norise` | Warning | 1 | Engine-minus-hangar delta below threshold after N minutes on. Threshold set empirically per FR-004. | 1 |
| `hm/reset` | Warning | 1 | Master-reset rail drop detected (FR-015a) | 1 |
| `hm/power/lost` | Critical | 1 | Controller on battery (ER-004) | 1 |
| `hm/power/restored` | Info | 0 | AC power restored | 1 |
| `hm/boot` | Info | -1 | Controller started or returned to service | 1 |
| `hm/cabin/on` / `hm/cabin/timeout` | Info | 0 | Heater on; heater reached its timeout | 2 |
| `hm/cabin/norise` | Warning | 1 | Cabin temperature not rising | 2 |
| `hm/flood`, `hm/river`, `hm/activity`, `hm/lights` | per §17 | per tier | Reserved | 3–5 |

Communications loss (CR-001) **cannot be published by the device**. See open item 32.

---

## 9A.8 Proposed changes to existing PRD sections

| Section | Change |
|---|---|
| §3 System Overview | Replace "remote notification and control software" with a reference to Section 9A. |
| FR-013 | Add "Tanis current present while commanded OFF" and "Master reset activated" to the notification list. |
| FR-014 | State that Phase 1 is satisfied by the Hangar Status Shortcut's text summary. A graphical dashboard remains under Future Enhancements (§24). |
| CR-002 | Cross-reference IF-003 and IF-004 as the implementation of the acknowledgment chain. |
| §18 Workflow | Steps 2–6 become "Run Preheat On; review the confirmed result." |

---

## 9A.9 Assumptions

- **A-1.** A confirmation window of about 10 s is enough time for relay actuation plus CT RMS sampling. To be validated during CT calibration (Appendix A.3).
- **A-2.** The phone side can read the `status` variable fast enough for interactive use over LTE-M. To be validated in the hangar signal test.
- **A-3.** The cabin-heater default is 120 min with a hard maximum of 240 min. These are placeholders until the owner sets them.

## 9A.10 New open items (continuing Section 25 numbering)

32. **Communications-loss detection (CR-001).** The device cannot report its own disconnection. Candidates are the Particle Cloud's device online/offline status events forwarded by a webhook, or a simple external check. Particle's offline-detection latency and webhook support for these events need to be verified.
33. **History storage (FR-016/017).** Particle does not retain sensor history. Options are a ring buffer on the device's flash, a webhook to a spreadsheet or log service, or deferral. Phase 1 only needs the current engine-minus-hangar trend, so this is not Phase 1 blocking.
34. **Cabin-heater timeout values (A-3).** Owner to set the default and the maximum.
35. **CT thresholds.** The ON band, the low-current fault threshold, and the OFF threshold. Derived from the Appendix A.3 calibration.
36. **Engine no-rise threshold and delay.** Set empirically after the first few preheat cycles (FR-004).
37. **Particle data-operation budget.** Confirm that the publish rate and the status polling stay within the plan's monthly allowance.
