/*
 * Hangar_Mate — relay control, ARM/SAFE interlock, temperatures, status
 * Particle Muon + M404, Device OS 6.x
 *
 * v0.2.0 adds (relay and ARM/SAFE behavior unchanged from v0.1):
 *   - DS18B20 temperatures through the DS2484 I2C-to-1-Wire bridge (QWIIC),
 *     read every 30 s without blocking the loop. No OneWire library.
 *   - `status` variable: JSON summary (PRD 9A.4, IF-002)
 *   - `alloff` function (PRD 9A.3)
 *   - FR-003a preheat effectiveness check: hm/eng/norise or hm/eng/ok
 *     30 min after Tanis is turned on. Notification only.
 *   - TSR-002 long-run notice: hm/tanis/longrun after 24 h ON. Notification only.
 *   - hm/boot event
 *
 * Wiring:
 *   D2 (header pin 11, HAT IO17) -> Tanis relay IN  (active-high)
 *   D3 (header pin 36, HAT IO16) -> Cabin relay IN  (active-high)
 *   D4 (header pin 33, HAT IO13) <- switched 5V rail via 10k/15k divider
 *        HIGH (~3.0 V) = rail present = ARMED
 *        LOW            = rail absent  = SAFE (relays have no coil power)
 *   QWIIC (I2C)                  -> Adafruit DS2484, address 0x18
 *        DS2484 1-Wire terminal  -> 3x DS18B20, powered mode (VDD, GND, DQ)
 *
 * Cloud functions:
 *   tanis  on | off | toggle
 *   cabin  on | off | toggle
 *   alloff (no argument)
 *     Return codes:
 *        1 = output is now ON
 *        0 = output is now OFF (alloff: all outputs now OFF)
 *       -1 = bad argument
 *       -2 = refused: arm switch SAFE (or not yet read after boot); outputs OFF
 *
 * Cloud variables: status (JSON), tanisState, cabinState, armState
 * Cloud events:    hm/arm, hm/cabin, hm/eng/norise, hm/eng/ok,
 *                  hm/tanis/longrun, hm/boot   (all private)
 *
 * Safety rules kept (see firmware/README.md):
 *   - Relay pins LOW in STARTUP() and again in setup()
 *   - ARM sense uses INPUT_PULLDOWN (broken wire reads SAFE)
 *   - Tanis is NEVER de-energized on a timer, by FR-003a, or by long-run
 */

#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

const char *FW_VERSION = "0.2.0";

// ---------------------------------------------------------------- pins
const pin_t RELAY_TANIS = D2;
const pin_t RELAY_CABIN = D3;
const pin_t ARM_SENSE   = D4;

// ---------------------------------------------------------------- timing
const unsigned long DEBOUNCE_MS = 50UL;

// Cabin heater maximum run time. PLACEHOLDER: 120 min pending your decision
// (PRD Section 25 item 38). Tanis deliberately has no equivalent.
const unsigned long CABIN_MAX_ON_MS = 120UL * 60UL * 1000UL;

// FR-003a defaults (PRD Section 25 item 35: tune from logged heat cycles)
const unsigned long EFFECT_CHECK_MS = 30UL * 60UL * 1000UL;  // 30 min after ON
const float         EFFECT_MIN_RISE_F = 5.0f;                // absolute rise, °F

// TSR-002 long-run notification threshold (notification only)
const unsigned long TANIS_LONGRUN_MS = 24UL * 60UL * 60UL * 1000UL;

// Temperature cycle
const unsigned long TEMP_INTERVAL_MS = 30UL * 1000UL;  // start a conversion every 30 s
const unsigned long TEMP_CONVERT_MS  = 800UL;          // DS18B20 12-bit max 750 ms

// ---------------------------------------------------------------- return codes
const int RC_ON       = 1;
const int RC_OFF      = 0;
const int RC_BAD_ARG  = -1;
const int RC_DISARMED = -2;

// ---------------------------------------------------------------- probe map
// ROM IDs from firmware/bringup/onewire_bringup.ino. Replace with your own.
enum ProbeRole { PROBE_ENGINE = 0, PROBE_CABIN = 1, PROBE_HANGAR = 2, PROBE_COUNT = 3 };

const uint8_t PROBE_ROM[PROBE_COUNT][8] = {
    {0x28,0x67,0x61,0x25,0x00,0x00,0x00,0x75},  // ENGINE (unplugs with the Tanis cord)
    {0x28,0x58,0x17,0x24,0x00,0x00,0x00,0xC2},  // CABIN
    {0x28,0x0B,0x52,0x25,0x00,0x00,0x00,0x01},  // HANGAR
};

// ---------------------------------------------------------------- relay state
bool tanisOn = false;
bool cabinOn = false;
unsigned long tanisSince = 0;
unsigned long cabinSince = 0;
String tanisSinceUtc = "";        // "HH:MMZ" when time is valid

bool armed      = false;   // true only when the switched rail is confirmed present
bool armKnown   = false;   // false until the first debounced read after boot
int  lastRaw    = -1;
unsigned long rawChangedAt = 0;

String tanisState = "off";
String cabinState = "off";
String armState   = "unknown";
String status     = "{}";

// ---------------------------------------------------------------- temperature state
bool   tempValid[PROBE_COUNT] = {false, false, false};
float  tempC[PROBE_COUNT]     = {0, 0, 0};
bool   ds2484Ok = false;
bool   converting = false;
unsigned long lastTempStart = 0;
unsigned long convertStartedAt = 0;

// ---------------------------------------------------------------- FR-003a / long-run state
bool  effectPending = false;
bool  effectBaseValid = false;
float effectBaseEngF = 0;
bool  effectBaseHgrValid = false;
float effectBaseHgrF = 0;
bool  longrunSent = false;

bool bootPublished = false;
bool statusDirty = true;

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

float cToF(float c) { return c * 9.0f / 5.0f + 32.0f; }

void cancelTanisChecks() {
    effectPending = false;
    longrunSent = false;
}

void startTanisChecks() {
    // FR-003a baseline from the most recent reading (at most ~30 s old)
    effectPending = true;
    effectBaseValid = tempValid[PROBE_ENGINE];
    effectBaseEngF = cToF(tempC[PROBE_ENGINE]);
    effectBaseHgrValid = tempValid[PROBE_HANGAR];
    effectBaseHgrF = cToF(tempC[PROBE_HANGAR]);
    longrunSent = false;
}

void setRelay(pin_t pin, bool on) {
    digitalWrite(pin, on ? HIGH : LOW);
    if (pin == RELAY_TANIS) {
        bool wasOn = tanisOn;
        tanisOn = on;
        tanisState = on ? "on" : "off";
        if (on && !wasOn) {
            tanisSince = millis();
            tanisSinceUtc = Time.isValid() ? Time.format(Time.now(), "%H:%MZ") : "";
            startTanisChecks();
        } else if (!on) {
            tanisSince = 0;
            tanisSinceUtc = "";
            cancelTanisChecks();   // FR-003a: cancelled silently when turned off / SAFE
        }
    } else {
        cabinOn = on;
        cabinSince = on ? millis() : 0;
        cabinState = on ? "on" : "off";
    }
    statusDirty = true;
    Log.info("%s relay %s", pin == RELAY_TANIS ? "Tanis" : "Cabin", on ? "ON" : "OFF");
}

// =====================================================================
// DS2484 I2C-to-1-Wire bridge (driven directly with Wire)
// =====================================================================
const uint8_t DS2484_ADDR = 0x18;

const uint8_t CMD_DEVICE_RESET = 0xF0;
const uint8_t CMD_SET_READ_PTR = 0xE1;
const uint8_t CMD_WRITE_CONFIG = 0xD2;
const uint8_t CMD_1W_RESET     = 0xB4;
const uint8_t CMD_1W_WRITE     = 0xA5;
const uint8_t CMD_1W_READ      = 0x96;

const uint8_t PTR_DATA = 0xE1;

const uint8_t ST_1WB = 0x01;  // 1-Wire busy
const uint8_t ST_PPD = 0x02;  // presence pulse detected
const uint8_t ST_SD  = 0x04;  // short detected
const uint8_t ST_RST = 0x10;  // device reset

const uint8_t OW_MATCH_ROM   = 0x55;
const uint8_t OW_SKIP_ROM    = 0xCC;
const uint8_t DS_CONVERT_T   = 0x44;
const uint8_t DS_READ_SCRPAD = 0xBE;

uint8_t crc8(const uint8_t *data, int len) {
    uint8_t crc = 0;
    for (int i = 0; i < len; i++) {
        uint8_t b = data[i];
        for (int j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ b) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            b >>= 1;
        }
    }
    return crc;
}

bool i2cWrite1(uint8_t a) {
    Wire.beginTransmission(DS2484_ADDR);
    Wire.write(a);
    return Wire.endTransmission() == 0;
}

bool i2cWrite2(uint8_t a, uint8_t b) {
    Wire.beginTransmission(DS2484_ADDR);
    Wire.write(a);
    Wire.write(b);
    return Wire.endTransmission() == 0;
}

int i2cRead1() {
    if (Wire.requestFrom(DS2484_ADDR, (uint8_t)1) != 1) return -1;
    return Wire.available() ? Wire.read() : -1;
}

bool waitIdle(uint8_t &st, unsigned long timeoutMs = 20) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        int s = i2cRead1();
        if (s < 0) return false;
        st = (uint8_t)s;
        if (!(st & ST_1WB)) return true;
    }
    return false;
}

bool ds2484Init() {
    if (!i2cWrite1(CMD_DEVICE_RESET)) return false;
    int s = i2cRead1();
    if (s < 0 || !(s & ST_RST)) return false;
    const uint8_t cfg = 0x01;  // APU (active pull-up)
    if (!i2cWrite2(CMD_WRITE_CONFIG, (uint8_t)(cfg | ((~cfg & 0x0F) << 4)))) return false;
    return i2cRead1() == cfg;
}

bool owReset() {
    uint8_t st;
    if (!i2cWrite1(CMD_1W_RESET) || !waitIdle(st)) return false;
    if (st & ST_SD) return false;
    return st & ST_PPD;
}

bool owWriteByte(uint8_t b) {
    uint8_t st;
    return i2cWrite2(CMD_1W_WRITE, b) && waitIdle(st);
}

int owReadByte() {
    uint8_t st;
    if (!i2cWrite1(CMD_1W_READ) || !waitIdle(st)) return -1;
    if (!i2cWrite2(CMD_SET_READ_PTR, PTR_DATA)) return -1;
    return i2cRead1();
}

// Start a conversion on every probe at once. Returns false if no probe answered.
bool tempStartConversion() {
    if (!owReset()) return false;
    return owWriteByte(OW_SKIP_ROM) && owWriteByte(DS_CONVERT_T);
}

// Read one probe by ROM. False = not answering or CRC failure.
bool tempReadProbe(const uint8_t *rom, float &c) {
    if (!owReset()) return false;
    if (!owWriteByte(OW_MATCH_ROM)) return false;
    for (int i = 0; i < 8; i++) if (!owWriteByte(rom[i])) return false;
    if (!owWriteByte(DS_READ_SCRPAD)) return false;

    uint8_t sp[9];
    bool allFF = true;
    for (int i = 0; i < 9; i++) {
        int b = owReadByte();
        if (b < 0) return false;
        sp[i] = (uint8_t)b;
        if (sp[i] != 0xFF) allFF = false;
    }
    if (allFF || crc8(sp, 8) != sp[8]) return false;

    int16_t raw = (int16_t)((sp[1] << 8) | sp[0]);
    c = raw / 16.0f;
    return true;
}

// Non-blocking temperature cycle, called from loop()
void serviceTemperatures() {
    unsigned long now = millis();

    if (!converting) {
        if (lastTempStart != 0 && now - lastTempStart < TEMP_INTERVAL_MS) return;
        lastTempStart = now;
        if (lastTempStart == 0) lastTempStart = 1;

        if (!ds2484Ok) {
            ds2484Ok = ds2484Init();
            if (!ds2484Ok) {
                Log.warn("DS2484 not responding on I2C");
                for (int i = 0; i < PROBE_COUNT; i++) tempValid[i] = false;
                statusDirty = true;
                return;
            }
        }
        if (tempStartConversion()) {
            converting = true;
            convertStartedAt = now;
        } else {
            // No presence pulse: every probe is unplugged, or the bus/bridge failed
            for (int i = 0; i < PROBE_COUNT; i++) tempValid[i] = false;
            ds2484Ok = ds2484Init();   // re-initialize in case the bridge reset
            statusDirty = true;
        }
        return;
    }

    if (now - convertStartedAt < TEMP_CONVERT_MS) return;
    converting = false;

    for (int i = 0; i < PROBE_COUNT; i++) {
        float c;
        bool ok = tempReadProbe(PROBE_ROM[i], c);
        if (!ok) ok = tempReadProbe(PROBE_ROM[i], c);   // one retry
        // 85.0 C is the DS18B20 power-on value: treat as not yet valid
        if (ok && c == 85.0f) ok = false;
        tempValid[i] = ok;
        if (ok) tempC[i] = c;
    }
    statusDirty = true;
}

// =====================================================================
// Commands
// =====================================================================
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

    if (want == isOn) {
        // Already in that state. For the cabin heater, "on" starts a new
        // run-time window (PRD 9A.3). Tanis keeps its original ON time and checks.
        if (want && pin == RELAY_CABIN) cabinSince = millis();
        return want ? RC_ON : RC_OFF;
    }

    setRelay(pin, want);
    return want ? RC_ON : RC_OFF;
}

int tanisCmd(String arg) { return handleCmd(RELAY_TANIS, arg); }
int cabinCmd(String arg) { return handleCmd(RELAY_CABIN, arg); }

int allOffCmd(String arg) {
    if (tanisOn) setRelay(RELAY_TANIS, false);
    if (cabinOn) setRelay(RELAY_CABIN, false);
    Log.info("alloff: all outputs off");
    if (!armKnown || !armed) return RC_DISARMED;   // IF-008: outputs off either way
    return RC_OFF;
}

// =====================================================================
// ARM/SAFE switch
// =====================================================================
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
    statusDirty = true;

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

// =====================================================================
// FR-003a effectiveness check and TSR-002 long-run notice
// Notification only: neither ever changes the Tanis relay.
// =====================================================================
void serviceTanisChecks() {
    if (!tanisOn) return;
    unsigned long onFor = millis() - tanisSince;
    char msg[256];

    if (effectPending && onFor >= EFFECT_CHECK_MS) {
        effectPending = false;

        char hgr[24];
        if (tempValid[PROBE_HANGAR]) snprintf(hgr, sizeof(hgr), "%.1fF", cToF(tempC[PROBE_HANGAR]));
        else                         snprintf(hgr, sizeof(hgr), "no reading");

        if (!tempValid[PROBE_ENGINE]) {
            snprintf(msg, sizeof(msg),
                     "Preheat check at %lu min: engine sensor not responding. Hangar %s. CT: not installed",
                     EFFECT_CHECK_MS / 60000UL, hgr);
            publishEvent("hm/eng/norise", msg);
        } else if (!effectBaseValid) {
            snprintf(msg, sizeof(msg),
                     "Preheat check at %lu min: no baseline (engine sensor not responding when Tanis turned on). Engine now %.1fF. Hangar %s. CT: not installed",
                     EFFECT_CHECK_MS / 60000UL, cToF(tempC[PROBE_ENGINE]), hgr);
            publishEvent("hm/eng/norise", msg);
        } else {
            float nowF = cToF(tempC[PROBE_ENGINE]);
            float rise = nowF - effectBaseEngF;
            snprintf(msg, sizeof(msg),
                     "Preheat check at %lu min: engine %.1fF -> %.1fF, rise %.1fF (min %.1fF). Hangar %s. CT: not installed",
                     EFFECT_CHECK_MS / 60000UL, effectBaseEngF, nowF, rise, EFFECT_MIN_RISE_F, hgr);
            publishEvent(rise < EFFECT_MIN_RISE_F ? "hm/eng/norise" : "hm/eng/ok", msg);
        }
    }

    if (!longrunSent && onFor >= TANIS_LONGRUN_MS) {
        longrunSent = true;
        snprintf(msg, sizeof(msg),
                 "Tanis has been ON for over %lu h. Still ON - no action taken.",
                 TANIS_LONGRUN_MS / 3600000UL);
        publishEvent("hm/tanis/longrun", msg);
    }
}

// =====================================================================
// status variable (PRD 9A.4, IF-002)
// =====================================================================
void appendTemp(char *buf, size_t len, const char *key, int idx, bool last) {
    size_t n = strlen(buf);
    if (tempValid[idx]) snprintf(buf + n, len - n, "\"%s\":%.1f%s", key, cToF(tempC[idx]), last ? "" : ",");
    else                snprintf(buf + n, len - n, "\"%s\":null%s", key, last ? "" : ",");
}

void updateStatus() {
    char buf[512];
    String t = Time.isValid() ? Time.format(Time.now(), "%Y-%m-%dT%H:%MZ") : "";

    // Phase 1: no CT yet, so states report command state (PRD 9A.5)
    long cabRemMin = 0;
    if (cabinOn) {
        unsigned long elapsed = millis() - cabinSince;
        cabRemMin = elapsed >= CABIN_MAX_ON_MS ? 0 : (long)((CABIN_MAX_ON_MS - elapsed + 59999UL) / 60000UL);
    }

    const char *pwr;
    switch (System.powerSource()) {
        case POWER_SOURCE_VIN:         pwr = "VIN";  break;
        case POWER_SOURCE_USB_HOST:
        case POWER_SOURCE_USB_ADAPTER:
        case POWER_SOURCE_USB_OTG:     pwr = "USB";  break;
        case POWER_SOURCE_BATTERY:     pwr = "BATT"; break;
        default:                       pwr = "unknown";
    }

    // Signal strength %, refreshed at most every 5 min (the modem query can take time)
    static int sig = -1;
    static unsigned long sigAt = 0;
    if (sigAt == 0 || millis() - sigAt >= 5UL * 60UL * 1000UL) {
        sigAt = millis();
        if (sigAt == 0) sigAt = 1;
        sig = -1;
        if (Cellular.ready()) {
            float s = Cellular.RSSI().getStrength();
            if (s >= 0) sig = (int)(s + 0.5f);
        }
    }

    snprintf(buf, sizeof(buf),
             "{\"t\":\"%s\","
             "\"tanis\":{\"st\":\"%s\",\"since\":\"%s\"},"
             "\"cabin\":{\"st\":\"%s\",\"rem\":%ld},"
             "\"temp\":{",
             t.c_str(),
             tanisOn ? "COMMANDED_ON" : "OFF", tanisSinceUtc.c_str(),
             cabinOn ? "COMMANDED_ON" : "OFF", cabRemMin);
    appendTemp(buf, sizeof(buf), "eng", PROBE_ENGINE, false);
    appendTemp(buf, sizeof(buf), "cab", PROBE_CABIN,  false);
    appendTemp(buf, sizeof(buf), "hgr", PROBE_HANGAR, true);
    size_t n = strlen(buf);
    snprintf(buf + n, sizeof(buf) - n,
             "},\"arm\":\"%s\",\"pwr\":\"%s\",\"sig\":%d,\"fw\":\"%s\"}",
             armState.c_str(), pwr, sig, FW_VERSION);

    status = buf;
    statusDirty = false;
}

// =====================================================================
// setup / loop
// =====================================================================
void setup() {
    earlyInit();                          // belt and suspenders

    Particle.function("tanis", tanisCmd);
    Particle.function("cabin", cabinCmd);
    Particle.function("alloff", allOffCmd);
    Particle.variable("tanisState", tanisState);
    Particle.variable("cabinState", cabinState);
    Particle.variable("armState", armState);
    Particle.variable("status", status);

    Wire.setSpeed(CLOCK_SPEED_100KHZ);
    Wire.begin();
    ds2484Ok = ds2484Init();
    if (!ds2484Ok) Log.warn("DS2484 not found at startup - will retry");

    updateStatus();
    Log.info("Hangar_Mate %s started - both relays OFF", FW_VERSION);
}

void loop() {
    checkArmSwitch();

    // Cabin heater safety timeout (controller-enforced). No Tanis timer.
    if (cabinOn && millis() - cabinSince >= CABIN_MAX_ON_MS) {
        setRelay(RELAY_CABIN, false);
        publishEvent("hm/cabin", "max run time reached - cabin heater off");
    }

    serviceTemperatures();
    serviceTanisChecks();

    if (!bootPublished && Particle.connected()) {
        bootPublished = true;
        publishEvent("hm/boot", String::format("Hangar_Mate %s started, all outlets off", FW_VERSION).c_str());
    }

    // Refresh status on change, and at least once a minute (cabin time remaining)
    static unsigned long lastStatus = 0;
    if (statusDirty || millis() - lastStatus >= 60000UL) {
        updateStatus();
        lastStatus = millis();
    }
}
