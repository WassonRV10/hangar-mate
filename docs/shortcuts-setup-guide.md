# iPhone Shortcuts Setup Guide — Hangar Mate

**Status:** Draft v0.2 (2026-11-14)
**Source:** The owner's working Shortcuts (2026-11-13/14), checked against PRD v0.22 Section 9A and firmware v0.2.0.
**Covers:** Engine Heat (Tanis toggle — "Preheat" in the PRD), Cabin Heat, Airplane Heat Status ("Hangar Status" in the PRD), and an optional All Off.

This guide builds the Shortcuts by hand. The builder's guide (Section 7.4) gives the short version; this is the detailed, screen-by-screen version.

---

## 1. What you need first

| Item | Where to get it | Notes |
|---|---|---|
| Device ID (or device name) | Particle Console → Devices | The API accepts either. |
| Access token | A **dedicated** token created only for these Shortcuts (PRD IF-SEC-01) | Treat it as a password: anyone with it can switch your heaters. Revoke and replace it if it leaks. Never post a screenshot that shows it. |
| iPhone with Shortcuts | Built in | Spoken replies need the volume up or headphones connected. |

Replace `<DEVICE_ID>` and `<TOKEN>` below with your values.

**Particle API calls used:**

| Purpose | Method | URL | Reply field |
|---|---|---|---|
| Call a function (`tanis`, `cabin`, `alloff`) | POST | `https://api.particle.io/v1/devices/<DEVICE_ID>/<function>` | `return_value` (integer) |
| Read a variable (`status`) | GET | `https://api.particle.io/v1/devices/<DEVICE_ID>/status` | `result` (JSON text) |

**Function return codes (PRD 9A.3):**

| Code | Meaning |
|---|---|
| `1` | Output is now ON (relay energized — heat **not** confirmed) |
| `0` | Output is now OFF |
| `-1` | Invalid argument |
| `-2` | Refused — ARM/SAFE switch is SAFE (or not yet read since boot). All outputs off. |
| missing | Controller offline, cloud timeout, or token problem — state unknown |

---

## 2. Common step: the "Get Contents of URL" action

Every Shortcut starts with this action.

1. Open **Shortcuts** → tap **+** → name the Shortcut.
2. Tap **Add Action**, search **Get Contents of URL**, and add it.
3. Tap the URL field and type the URL for that Shortcut (see each section).
4. Tap the **chevron (⌄)** next to the URL to show the options.
5. **Method:** POST for the control Shortcuts, GET for status.
6. **Headers:** tap **Add new header**.
   - Key: `Authorization`
   - Text: `Bearer <TOKEN>` (the word Bearer, one space, then the token)
   - Delete any blank header rows with the red ⊖.
7. **Request Body** (control Shortcuts only): choose **Form**, tap **Add new field** → **Text**.
   - Key: `arg`
   - Value: `toggle`

---

## 3. Engine Heat and Cabin Heat (control Shortcuts)

The two are identical except for the function name and the reply text. Build Cabin Heat, test it, then **duplicate** it (long-press → Duplicate) and edit the copy into Engine Heat.

| Shortcut | URL ends with | Cloud call |
|---|---|---|
| Cabin Heat | `/cabin` | `cabin` = `toggle` |
| Engine Heat | `/tanis` | `tanis` = `toggle` |

### 3.1 Actions in order

**Action 1 — Get Contents of URL**
Per Section 2: URL `https://api.particle.io/v1/devices/<DEVICE_ID>/cabin`, Method **POST**, Authorization header, Form body `arg` = `toggle`.

**Action 2 — Get Dictionary Value**
Get **Value** for **`return_value`** in **Contents of URL**. Its output, **Dictionary Value**, is what every If below tests.

**Action 3 — If Dictionary Value is 1 (now ON)**
1. Add **If**, tap the input, and choose **Dictionary Value**. Condition **is**, value **1**.
2. Inside it, add **Speak Text** (or **Show Notification** if you'd rather read it):
   - Cabin: `Cabin heat relay on. Heat not yet confirmed.`
   - Engine: `Preheat relay on. Heat not yet confirmed.`
3. Delete the **Otherwise** that Shortcuts inserts; keep **End If**.

"Heat not yet confirmed" is required (PRD IF-003). A `1` means the relay closed, not that current is flowing or the engine is warming.

**Action 4 — If Dictionary Value is 0 (now OFF)**
Same pattern. Reply: `Cabin heat is off.` / `Preheat is off.`

**Action 5 — If Dictionary Value is -2 (switch SAFE)**
Same pattern. Reply: `Relays not active. Arm switch is off. Nothing changed.`

**Action 6 — If All are true (anything else = state unknown)**
1. Add **If**, tap **Add Condition** until there are three, and set the selector at the top to **All**.
2. Conditions:
   - Dictionary Value **is not** 1
   - Dictionary Value **is not** 0
   - Dictionary Value **is not** -2
3. Inside: `No response from hangar. State unknown. Check status before trying again.`

This catches `-1`, an empty reply, and Particle's "timed out / device offline" reply. If the phone itself has no connection, Shortcuts stops at Action 1 with its own error, which still tells you it failed.

> **Do not add a retry or a Repeat.** `toggle` is not idempotent: a second toggle undoes the first (IF-010). After "state unknown," run Airplane Heat Status before sending another command.

### 3.2 Optional simplification

The four separate If blocks work. An equivalent, tidier version is one chain: **If 1 → Otherwise → If 0 → Otherwise → If -2 → Otherwise (unknown)**. It removes the three-condition block. Only worth doing if you're editing anyway.

---

## 4. Airplane Heat Status

Reads the controller's `status` variable and shows one summary screen.

### 4.1 What `status` contains (firmware v0.2.0)

```json
{"t":"2026-11-13T12:05Z",
 "tanis":{"st":"COMMANDED_ON","since":"11:30Z"},
 "cabin":{"st":"OFF","rem":0},
 "temp":{"eng":67.9,"cab":67.6,"hgr":69.0},
 "arm":"ARMED","pwr":"VIN","sig":63,"fw":"0.2.0"}
```

- `tanis.st` / `cabin.st` are `OFF` or `COMMANDED_ON` until the current sensor is installed. `COMMANDED_ON` means the relay is on but heat is not confirmed, so showing it as-is satisfies IF-003.
- A temperature is `null` when its probe isn't answering. That's normal for the engine probe when the Tanis cord is unplugged; the line will show blank.
- `cabin.rem` is minutes left on the cabin heater's maximum run time.

### 4.2 Actions in order

**Action 1 — Get Contents of URL**
URL `https://api.particle.io/v1/devices/<DEVICE_ID>/status`, Method **GET**, Authorization header. No request body.

**Action 2 — Get Dictionary Value**
Get **Value** for **`result`** in **Contents of URL**. Tap the output bubble → **Rename** → `StatusNow`.

**Action 3 — If StatusNow does not have any value**
1. Add **If**, input **StatusNow**, condition **does not have any value**.
2. Inside: **Show Alert** `No response from hangar.`, then **Stop This Shortcut**.
3. Keep the **Otherwise**; everything below goes inside it.

**Action 4 (inside Otherwise) — read each field**
For each row: add **Get Dictionary Value** (Get **Value** for *key* in **StatusNow**), then **Set Variable** *name* to **Dictionary Value**.

| Key | Variable | Shown as |
|---|---|---|
| `temp.eng` | `eng` | Engine: … °F |
| `temp.cab` | `cab` | Cabin: … °F |
| `temp.hgr` | `hgr` | Hangar: … °F |
| `tanis.st` | `tanis` | Tanis: OFF / COMMANDED_ON |
| `cabin.st` | `cabin` | Cabin heater: OFF / COMMANDED_ON |
| `arm` | `arm` | Switch: ARMED / SAFE |

The dot (`temp.eng`) reaches into the nested JSON. Shortcuts turns the `StatusNow` text into a dictionary on its own, so a separate **Get dictionary from StatusNow** action isn't needed; if you have one, it's harmless.

**Action 5 — Show Result**

```
Engine: [eng]°F
Cabin: [cab]°F
Hangar: [hgr]°F
Tanis: [tanis]
Cabin heater: [cabin]
Switch: [arm]
```

**Action 6 — End If** (closes Action 3).

### 4.3 Additions the PRD calls for (PRD 9A.5, item 41)

Add these the same way as Action 4, then add them to the Show Result text.

| Line | How |
|---|---|
| **Engine − Hangar** | **Calculate**: `eng` − `hgr`, round to one decimal. This is the quickest "is it warming" check until the current sensor is installed. If the engine probe is `null`, skip this line or expect a wrong number. |
| **Cabin heat left: … min** | Key `cabin.rem` |
| **Power** | Key `pwr` (`VIN` normal, `BATT` = on battery) |
| **Signal: … %** | Key `sig` (`-1` = unknown) |
| **Updated** | Key `t` (UTC) |

---

## 5. All Off (optional; build once `alloff` is verified on hardware)

1. Get Contents of URL: `https://api.particle.io/v1/devices/<DEVICE_ID>/alloff`, Method **POST**, Authorization header. Request Body **Form** with `arg` = `all` (the firmware ignores the value).
2. Get Dictionary Value: `return_value`.
3. If `0` → `All outlets off.` If `-2` → `Relays not active. Arm switch is off. Outlets are off.` Otherwise → `No response from hangar. State unknown.`

All Off is safe to re-run. It does not replace the physical ARM/SAFE switch.

---

## 6. Home screen and Siri

1. For each Shortcut: tap **⌄** next to its name → **Add to Home Screen**, then set the icon and color.
2. Drag one icon onto another to make a folder (e.g., "Airplane heat").
3. Siri uses the Shortcut's name as its phrase ("Hey Siri, Engine Heat"). Rename the Shortcut to change the phrase. Because the control Shortcuts toggle, always listen to the reply.

---

## 7. Verification checklist

Test with a lamp on each outlet, or with someone at the hangar.

| # | Setup | Action | Expected |
|---|---|---|---|
| T1 | ARMED, cabin off | Cabin Heat | "Cabin heat relay on. Heat not yet confirmed." Relay clicks. |
| T2 | After T1 | Cabin Heat | "Cabin heat is off." |
| T3 | SAFE | Cabin Heat | "Relays not active…" Nothing energizes. |
| T4 | SAFE → ARMED | Airplane Heat Status | Tanis OFF, Cabin heater OFF: nothing came back on by itself (IF-008). |
| T5 | Controller unplugged | Cabin Heat | "No response from hangar. State unknown." No retry. |
| T6 | Controller unplugged | Airplane Heat Status | "No response from hangar." alert, then stops. |
| T7 | Repeat T1–T3 and T5 | Engine Heat | Same results with the Preheat wording. |
| T8 | Tanis on 15+ min, probe in the cowl | Airplane Heat Status | Engine temperature rising faster than Hangar. |

---

## 8. Built Shortcuts vs. PRD (as of 2026-11-14)

| # | Finding | Action | Tracked |
|---|---|---|---|
| 1 | Cabin Heat says "Cabin heat is on" for `1`. IF-003 requires "not confirmed." | Change the reply (Section 3.1). | PRD item 41a |
| 2 | Airplane Heat Status lacks the delta, time remaining, power, and signal lines. Two lines are both labeled "Cabin." | Section 4.3; label the relay line "Cabin heater." | PRD item 41b |
| 3 | The Tanis Shortcut is "Engine Heat" on the phone and "Preheat" in the PRD. | Choose one name. | PRD item 41c |
| 4 | Airplane Heat Status has a blank header row. | Delete it. | — |
| 5 | All Off isn't built. | Section 5, after `alloff` is verified. | PRD item 41d |
