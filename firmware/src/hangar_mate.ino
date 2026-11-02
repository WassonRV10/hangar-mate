/*
 * Hangar_Mate — relay control firmware with toggle and arm-switch reporting
 * Particle Muon + M404, Device OS 6.x
 *
 * Changes from master_reset_bench_test.ino:
 *   - tanis / cabin accept "toggle" in addition to "on" / "off"
 *   - Return codes report the RESULTING state (see below)
 *   - ALL commands (on, off, toggle) return -2 while the arm switch is SAFE,
 *     so the phone always knows the relays have no power
 *   - Commands are refused until the arm switch has been read once after boot
 *   - Arm sense pin uses INPUT_PULLDOWN: a broken/missing sense wire reads SAFE
 *   - Tanis has NO timed auto-off (PRD TSR-001/002, Tanis guide)
 *   - Cabin heater keeps a firmware-enforced maximum run time
 *   - "test hwonly/normal" bench function removed (hardware path already verified)
 *   - Variable "overrideState" renamed "armState" (ARMED / SAFE / unknown)
 *   - Temperature code not included (DS2484 bridge not yet bench-tested)
 *
 * Wiring (unchanged):
 *   D2 (header pin 11, HAT IO17) -> Tanis relay IN  (active-high)
 *   D3 (header pin 36, HAT IO16) -> Cabin relay IN  (active-high)
 *   D4 (header pin 33, HAT IO13) <- switched 5V rail via 10k/15k divider
 *        HIGH (~3.0 V) = rail present = ARMED
 *        LOW            = rail absent  = SAFE (relays have no coil power)
 *
 * Cloud functions:
 *   tanis  on | off | toggle
 *   cabin  on | off | toggle
 *     Return codes:
 *        1 = output is now ON
 *        0 = output is now OFF
 *       -1 = bad argument
 *       -2 = refused: arm switch SAFE (or not yet read after boot); outputs OFF
 *
 * Cloud variables: tanisState, cabinState ("on"/"off"), armState
 * Cloud events:    hm/arm (switch changes), hm/cabin (timeout)
 */

#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

// ---------------------------------------------------------------- pins
const pin_t RELAY_TANIS = D2;
const pin_t RELAY_CABIN = D3;
const pin_t ARM_SENSE   = D4;

// ---------------------------------------------------------------- timing
const unsigned long DEBOUNCE_MS = 50UL;

// Cabin heater maximum run time. PLACEHOLDER: 120 min pending your decision
// (PRD Section 9A open item). Tanis deliberately has no equivalent.
const unsigned long CABIN_MAX_ON_MS = 120UL * 60UL * 1000UL;

// ---------------------------------------------------------------- return codes
const int RC_ON       = 1;
const int RC_OFF      = 0;
const int RC_BAD_ARG  = -1;
const int RC_DISARMED = -2;

// ---------------------------------------------------------------- state
bool tanisOn = false;
bool cabinOn = false;
unsigned long cabinSince = 0;

bool armed      = false;   // true only when the switched rail is confirmed present
bool armKnown   = false;   // false until the first debounced read after boot
int  lastRaw    = -1;
unsigned long rawChangedAt = 0;

String tanisState = "off";
String cabinState = "off";
String armState   = "unknown";

// ---------------------------------------------------------------- early init
// Both relays OFF as early as user firmware can run.
void earlyInit() {
    pinMode(RELAY_TANIS, OUTPUT);
    digitalWrite(RELAY_TANIS, LOW);
    pinMode(RELAY_CABIN, OUTPUT);
    digitalWrite(RELAY_CABIN, LOW);
    pinMode(ARM_SENSE, INPUT_PULLDOWN);   // floating/broken sense wire reads SAFE
}
STARTUP(earlyInit());

// ---------------------------------------------------------------- helpers
void publishEvent(const char* name, const char* msg) {
    Log.info("%s: %s", name, msg);
    if (Particle.connected()) {
        Particle.publish(name, msg);
    }
}

void setRelay(pin_t pin, bool on) {
    digitalWrite(pin, on ? HIGH : LOW);
    if (pin == RELAY_TANIS) {
        tanisOn = on;
        tanisState = on ? "on" : "off";
    } else {
        cabinOn = on;
        cabinSince = on ? millis() : 0;
        cabinState = on ? "on" : "off";
    }
    Log.info("%s relay %s", pin == RELAY_TANIS ? "Tanis" : "Cabin", on ? "ON" : "OFF");
}

// ---------------------------------------------------------------- commands
int handleCmd(pin_t pin, String arg) {
    arg.trim();
    arg.toLowerCase();

    bool isOn = (pin == RELAY_TANIS) ? tanisOn : cabinOn;

    bool want;
    if      (arg == "on")     want = true;
    else if (arg == "off")    want = false;
    else if (arg == "toggle") want = !isOn;
    else return RC_BAD_ARG;

    // Relays have no coil power when SAFE. Refuse every command so the
    // caller is told, and keep firmware state matching reality.
    if (!armKnown || !armed) {
        if (isOn) setRelay(pin, false);
        Log.warn("Rejected '%s' on %s: arm switch %s",
                 arg.c_str(),
                 pin == RELAY_TANIS ? "Tanis" : "Cabin",
                 armKnown ? "SAFE" : "not yet read");
        return RC_DISARMED;
    }

    setRelay(pin, want);
    return want ? RC_ON : RC_OFF;
}

int tanisCmd(String arg) { return handleCmd(RELAY_TANIS, arg); }
int cabinCmd(String arg) { return handleCmd(RELAY_CABIN, arg); }

// ---------------------------------------------------------------- arm switch
void checkArmSwitch() {
    int raw = digitalRead(ARM_SENSE);
    if (raw != lastRaw) {                 // start debounce window
        lastRaw = raw;
        rawChangedAt = millis();
        return;
    }
    if (millis() - rawChangedAt < DEBOUNCE_MS) return;

    bool nowArmed = (raw == HIGH);
    if (armKnown && nowArmed == armed) return;

    armKnown = true;
    armed = nowArmed;
    armState = armed ? "ARMED" : "SAFE";

    if (!armed) {
        // Hardware has already cut coil power; clear commanded state.
        if (tanisOn) setRelay(RELAY_TANIS, false);
        if (cabinOn) setRelay(RELAY_CABIN, false);
        publishEvent("hm/arm", "SAFE - all outlets off");
    } else {
        // Re-arming never restores the prior state.
        publishEvent("hm/arm", "ARMED - outlets stay off until commanded");
    }
}

// ---------------------------------------------------------------- setup / loop
void setup() {
    earlyInit();                          // belt and suspenders

    Particle.function("tanis", tanisCmd);
    Particle.function("cabin", cabinCmd);
    Particle.variable("tanisState", tanisState);
    Particle.variable("cabinState", cabinState);
    Particle.variable("armState", armState);

    Log.info("Hangar_Mate started - both relays OFF");
}

void loop() {
    checkArmSwitch();

    // Cabin heater safety timeout (controller-enforced). No Tanis timer.
    if (cabinOn && millis() - cabinSince >= CABIN_MAX_ON_MS) {
        setRelay(RELAY_CABIN, false);
        publishEvent("hm/cabin", "max run time reached - cabin heater off");
    }
}
