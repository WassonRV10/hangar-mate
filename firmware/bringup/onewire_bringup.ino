/*
 * Hangar Mate - 1-Wire bring-up test firmware
 * Target: Particle Muon + M404, Adafruit DS2484 on QWIIC, 3x DS18B20 (powered mode)
 *
 * Runs automatically on boot - no serial input needed. Open a serial monitor
 * (Workbench serial monitor or `particle serial monitor --follow`) and watch.
 *
 *   STEP 1  I2C scan                  -> expect 0x18 (DS2484)
 *   STEP 2  DS2484 reset/config       -> expect "DS2484 OK"
 *   STEP 3  1-Wire ROM search         -> expect 3 IDs starting 0x28
 *   STEP 4  Continuous readings       -> every 3 s, all probes + spread
 *           - hand-warm one probe at a time to map ID -> location
 *           - ice-water test: all ~0.0 C, spread <= ~1.0 C
 *   STEP 5  Disconnect test           -> unplugged probe shows DISCONNECTED,
 *                                        recovers when reconnected
 *   Re-search every 20 cycles so you can see probe count change.
 *
 * Once mapped, paste the printed ROM IDs into PROBE_MAP below and re-flash;
 * readings will then be labeled ENGINE / CABIN / HANGAR.
 *
 * The DS2484 is driven directly with Wire (no library dependency).
 *
 * CLOUD MODE (no USB needed) - console.particle.io -> Hangar_Mate:
 *   Events    hm/temps   all probe readings, once per minute
 *             hm/search  probe count + ROM IDs (on boot and on report)
 *   Variable  temps      latest readings (click GET)
 *   Variable  search     latest probe search result
 *   Function  report     any arg -> re-search, read, publish immediately
 */

#include "Particle.h"

SYSTEM_MODE(AUTOMATIC);
SerialLogHandler logHandler(LOG_LEVEL_WARN);

// ---------------- Probe map (fill in after hand-warm test) ----------------
struct Probe {
    const char *name;
    uint8_t rom[8];
};

// Leave rom all zeros for "not yet mapped". Example after mapping:
//   { "ENGINE", {0x28,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x12} },
// Confirmed 2026-11-12. 5817 (cabin) replaced a dead probe.
Probe PROBE_MAP[] = {
    { "ENGINE", {0x28,0x67,0x61,0x25,0x00,0x00,0x00,0x75} },  // 6761 - confirmed
    { "CABIN",  {0x28,0x58,0x17,0x24,0x00,0x00,0x00,0xC2} },  // 5817 - confirmed
    { "HANGAR", {0x28,0x0B,0x52,0x25,0x00,0x00,0x00,0x01} },  // 0B52 - confirmed
};
const int PROBE_MAP_COUNT = sizeof(PROBE_MAP) / sizeof(PROBE_MAP[0]);

// ---------------- DS2484 definitions ----------------
const uint8_t DS2484_ADDR = 0x18;

const uint8_t CMD_DEVICE_RESET = 0xF0;
const uint8_t CMD_SET_READ_PTR = 0xE1;
const uint8_t CMD_WRITE_CONFIG = 0xD2;
const uint8_t CMD_1W_RESET     = 0xB4;
const uint8_t CMD_1W_WRITE     = 0xA5;
const uint8_t CMD_1W_READ      = 0x96;
const uint8_t CMD_1W_TRIPLET   = 0x78;

const uint8_t PTR_STATUS = 0xF0;
const uint8_t PTR_DATA   = 0xE1;

const uint8_t ST_1WB = 0x01;  // 1-Wire busy
const uint8_t ST_PPD = 0x02;  // presence pulse detected
const uint8_t ST_SD  = 0x04;  // short detected
const uint8_t ST_RST = 0x10;  // device reset
const uint8_t ST_SBR = 0x20;  // single bit result
const uint8_t ST_TSB = 0x40;  // triplet second bit
const uint8_t ST_DIR = 0x80;  // branch direction taken

// DS18B20 commands
const uint8_t OW_SEARCH_ROM  = 0xF0;
const uint8_t OW_MATCH_ROM   = 0x55;
const uint8_t OW_SKIP_ROM    = 0xCC;
const uint8_t DS_CONVERT_T   = 0x44;
const uint8_t DS_READ_SCRPAD = 0xBE;

// ---------------- State ----------------
const int MAX_FOUND = 8;
uint8_t foundRom[MAX_FOUND][8];
int foundCount = 0;
bool ds2484Ok = false;
int cycle = 0;

// ---------------- Cloud mode ----------------
// Event names as they appear in console.particle.io -> device -> Events
const char *EVT_TEMPS  = "hm/temps";
const char *EVT_SEARCH = "hm/search";
const uint32_t PUBLISH_INTERVAL_MS = 60 * 1000;  // 1 per minute keeps data ops low

String tempsVar = "booting";        // Particle.variable "temps"
String lastSearchSummary = "none";
uint32_t lastPublishMs = 0;
volatile bool reportRequested = false;

void cloudPublish(const char *name, const String &data) {
    if (Particle.connected()) {
        Particle.publish(name, data, PRIVATE);
    }
}

// Particle.function "report": any argument. Sets a flag; work is done in loop()
// so 1-Wire traffic never runs inside the cloud callback.
int fnReport(String arg) {
    reportRequested = true;
    return 0;
}

// =====================================================================
// Low-level helpers
// =====================================================================
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

void printRom(const uint8_t *rom) {
    for (int i = 0; i < 8; i++) {
        Serial.printf("0x%02X%s", rom[i], i < 7 ? "," : "");
    }
}

bool romIsZero(const uint8_t *rom) {
    for (int i = 0; i < 8; i++) if (rom[i]) return false;
    return true;
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

// Poll status (read pointer is left on STATUS by 1-Wire commands)
bool waitIdle(uint8_t &status, uint32_t timeoutMs = 20) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        int s = i2cRead1();
        if (s < 0) return false;
        status = (uint8_t)s;
        if (!(status & ST_1WB)) return true;
    }
    return false;
}

// =====================================================================
// DS2484 / 1-Wire primitives
// =====================================================================
bool ds2484Init() {
    if (!i2cWrite1(CMD_DEVICE_RESET)) {
        Serial.println("  DS2484: no ACK on device reset");
        return false;
    }
    int s = i2cRead1();
    if (s < 0 || !(s & ST_RST)) {
        Serial.printf("  DS2484: reset bit not set (status=0x%02X)\n", s);
        return false;
    }
    // Config: APU=1 (active pull-up). Upper nibble = one's complement of lower.
    const uint8_t cfg = 0x01;
    if (!i2cWrite2(CMD_WRITE_CONFIG, (uint8_t)(cfg | ((~cfg & 0x0F) << 4)))) {
        Serial.println("  DS2484: config write failed");
        return false;
    }
    int rb = i2cRead1();  // read pointer now on config register
    if (rb != cfg) {
        Serial.printf("  DS2484: config readback 0x%02X (expected 0x%02X)\n", rb, cfg);
        return false;
    }
    return true;
}

// Returns true if presence pulse detected. Reports shorts.
bool owReset(bool verbose = false) {
    uint8_t st;
    if (!i2cWrite1(CMD_1W_RESET) || !waitIdle(st)) {
        if (verbose) Serial.println("  1-Wire reset: DS2484 not responding");
        return false;
    }
    if (st & ST_SD) {
        if (verbose) Serial.println("  1-Wire reset: SHORT DETECTED on bus (check DQ/GND wiring)");
        return false;
    }
    if (verbose && !(st & ST_PPD)) {
        Serial.println("  1-Wire reset: no presence pulse (no probes answering)");
    }
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

bool owTriplet(bool dir, uint8_t &status) {
    return i2cWrite2(CMD_1W_TRIPLET, dir ? 0x80 : 0x00) && waitIdle(status);
}

// =====================================================================
// ROM search (Maxim AN187 algorithm using DS2484 triplet command)
// =====================================================================
uint8_t srchRom[8];
int srchLastDiscrepancy = 0;
bool srchLastDevice = false;

void searchResetState() {
    srchLastDiscrepancy = 0;
    srchLastDevice = false;
    memset(srchRom, 0, 8);
}

int srchCrcFails = 0;
int srchAborts = 0;   // search ended early: no responders mid-search or I2C trouble

// Returns 1 = device found, 0 = search finished, -1 = CRC fail (caller may continue)
int owSearchNext(uint8_t *romOut) {
    if (srchLastDevice) return 0;
    if (!owReset()) { searchResetState(); return 0; }
    if (!owWriteByte(OW_SEARCH_ROM)) { searchResetState(); srchAborts++; return 0; }

    int lastZero = 0;
    for (int bitNum = 1; bitNum <= 64; bitNum++) {
        int idx = (bitNum - 1) / 8;
        uint8_t mask = 1 << ((bitNum - 1) % 8);
        bool dir;
        if (bitNum < srchLastDiscrepancy)      dir = srchRom[idx] & mask;
        else                                   dir = (bitNum == srchLastDiscrepancy);

        uint8_t st;
        if (!owTriplet(dir, st)) { searchResetState(); srchAborts++; return 0; }
        bool idBit  = st & ST_SBR;
        bool cmpBit = st & ST_TSB;
        bool taken  = st & ST_DIR;

        if (idBit && cmpBit) {  // nobody answered
            if (bitNum > 1) srchAborts++;  // dropped out mid-ROM = marginal wiring
            searchResetState();
            return 0;
        }
        if (!idBit && !cmpBit && !taken) lastZero = bitNum;          // discrepancy, took 0

        if (taken) srchRom[idx] |= mask; else srchRom[idx] &= ~mask;
    }

    srchLastDiscrepancy = lastZero;
    if (srchLastDiscrepancy == 0) srchLastDevice = true;

    if (crc8(srchRom, 7) != srchRom[7]) {
        Serial.print("  ROM CRC FAIL: ");
        printRom(srchRom);
        Serial.println("  (noise / marginal wiring)");
        srchCrcFails++;
        return -1;
    }
    memcpy(romOut, srchRom, 8);
    return 1;
}

void runSearch() {
    Serial.println("\n=== STEP 3: 1-Wire ROM search ===");
    owReset(true);
    searchResetState();
    srchCrcFails = 0;
    srchAborts = 0;
    foundCount = 0;
    uint8_t rom[8];
    int guard = 0;
    while (foundCount < MAX_FOUND && guard++ < 16) {
        int r = owSearchNext(rom);
        if (r == 0) break;
        if (r < 0) continue;  // bad CRC: skip this one, keep searching other branches
        memcpy(foundRom[foundCount], rom, 8);
        foundCount++;
    }
    if (srchCrcFails || srchAborts) {
        Serial.printf("  WARNING: %d CRC fail(s), %d aborted search pass(es)\n", srchCrcFails, srchAborts);
    }
    Serial.printf("  Found %d device(s)%s\n", foundCount,
                  foundCount == 3 ? "  -> PASS" : "  -> expected 3");
    for (int i = 0; i < foundCount; i++) {
        Serial.printf("  [%d] ", i);
        printRom(foundRom[i]);
        if (foundRom[i][0] != 0x28) Serial.print("   <-- not a DS18B20 family code!");
        Serial.println();
    }
    Serial.println("  Paste-ready for PROBE_MAP:");
    for (int i = 0; i < foundCount; i++) {
        Serial.print("    { \"?\", {");
        printRom(foundRom[i]);
        Serial.println("} },");
    }

    // Cloud copy: "n=3 | 28AABBCCDDEEFF12 | ..."
    lastSearchSummary = String::format("n=%d crcFail=%d abort=%d", foundCount, srchCrcFails, srchAborts);
    for (int i = 0; i < foundCount; i++) {
        lastSearchSummary += " | ";
        for (int b = 0; b < 8; b++) lastSearchSummary += String::format("%02X", foundRom[i][b]);
    }
}

// =====================================================================
// DS18B20
// =====================================================================
bool convertAll() {
    if (!owReset()) return false;
    if (!owWriteByte(OW_SKIP_ROM) || !owWriteByte(DS_CONVERT_T)) return false;
    delay(800);  // 12-bit conversion max 750 ms (powered mode)
    return true;
}

// Returns false if the probe didn't answer or CRC failed (treat as DISCONNECTED)
bool readTemp(const uint8_t *rom, float &tempC) {
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
    if (allFF) return false;                 // nobody home
    if (crc8(sp, 8) != sp[8]) return false;  // corrupt read

    int16_t raw = (int16_t)((sp[1] << 8) | sp[0]);
    tempC = raw / 16.0f;
    return true;
}

// =====================================================================
// Steps 1, 2, 4/5
// =====================================================================
void runI2CScan() {
    Serial.println("\n=== STEP 1: I2C scan ===");
    bool sawDs = false;
    int n = 0;
    for (uint8_t a = 1; a < 127; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  device at 0x%02X%s\n", a, a == DS2484_ADDR ? "  <-- DS2484" : "");
            if (a == DS2484_ADDR) sawDs = true;
            n++;
        }
    }
    if (n == 0) {
        Serial.println("  NOTHING found. Check QWIIC cable and that 3V3_AUX is enabled");
        Serial.println("  (setup.particle.io normally enables it; QWIIC is powered from 3V3_AUX).");
    }
    Serial.println(sawDs ? "  DS2484 present -> PASS" : "  DS2484 NOT found -> FAIL");
}

void printReadings() {
    if (!convertAll()) {
        Serial.println("convert failed: no presence on bus (all probes disconnected?)");
    }

    // Use the map if any entry is filled, else whatever the last search found
    bool useMap = false;
    for (int i = 0; i < PROBE_MAP_COUNT; i++) if (!romIsZero(PROBE_MAP[i].rom)) useMap = true;

    int n = useMap ? PROBE_MAP_COUNT : foundCount;
    float minC = 1000, maxC = -1000;
    int good = 0;
    String summary;

    Serial.printf("[%6lu s] ", millis() / 1000);
    for (int i = 0; i < n; i++) {
        const uint8_t *rom = useMap ? PROBE_MAP[i].rom : foundRom[i];
        char label[16];
        if (useMap) snprintf(label, sizeof(label), "%s", PROBE_MAP[i].name);
        else        snprintf(label, sizeof(label), "%02X%02X", rom[1], rom[2]);  // low serial bytes vary most

        float c;
        if (summary.length()) summary += " | ";
        if (useMap && romIsZero(rom)) {
            Serial.printf("%s: unmapped   ", label);
            summary += String::format("%s unmapped", label);
        } else if (readTemp(rom, c)) {
            Serial.printf("%s: %6.2f C / %6.1f F%s   ", label, c, c * 9.0f / 5.0f + 32.0f,
                          c == 85.0f ? " (85.0=power-on value?)" : "");
            summary += String::format("%s %.1fC/%.1fF", label, c, c * 9.0f / 5.0f + 32.0f);
            if (c < minC) minC = c;
            if (c > maxC) maxC = c;
            good++;
        } else {
            Serial.printf("%s: DISCONNECTED   ", label);
            summary += String::format("%s DISCONNECTED", label);
        }
    }
    if (good >= 2) {
        Serial.printf("| spread %.2f C", maxC - minC);
        summary += String::format(" | spread %.1fC", maxC - minC);
    }
    if (n == 0) summary = "no probes found";
    Serial.println();

    tempsVar = summary;  // always current for Particle.variable "temps"
}

// =====================================================================
void setup() {
    // Register cloud items first, before any long-running work
    Particle.variable("temps", tempsVar);
    Particle.variable("search", lastSearchSummary);
    Particle.function("report", fnReport);

    Serial.begin(115200);
    waitFor(Serial.isConnected, 15000);  // continues after 15 s if no USB (hangar use)
    delay(500);

    Serial.println("\n##### Hangar Mate 1-Wire bring-up #####");

    Wire.setSpeed(CLOCK_SPEED_100KHZ);
    Wire.begin();

    runI2CScan();

    Serial.println("\n=== STEP 2: DS2484 init ===");
    ds2484Ok = ds2484Init();
    Serial.println(ds2484Ok ? "  DS2484 OK (APU enabled)" : "  DS2484 init FAILED - stopping here");
    if (!ds2484Ok) {
        tempsVar = "DS2484 init FAILED - check QWIIC";
        return;
    }

    runSearch();

    Serial.println("\n=== STEP 4/5: continuous readings (every ~3 s) ===");
    Serial.println("  Hand-warm one probe at a time to identify it.");
    Serial.println("  Unplug a probe to confirm it shows DISCONNECTED, then reconnect.");
}

void loop() {
    if (!ds2484Ok) {
        delay(5000);
        Serial.println("DS2484 not initialized - fix STEP 1/2 and reset.");
        static bool sentFail = false;
        if (!sentFail && Particle.connected()) {
            cloudPublish(EVT_TEMPS, tempsVar);
            sentFail = true;
        }
        return;
    }

    // On-demand report from the Console / phone: re-search, read, publish now
    if (reportRequested) {
        reportRequested = false;
        runSearch();
        printReadings();
        cloudPublish(EVT_SEARCH, lastSearchSummary);
        cloudPublish(EVT_TEMPS, tempsVar);
        lastPublishMs = millis();
        delay(2200);
        return;
    }

    printReadings();
    if (++cycle % 20 == 0) runSearch();

    // Periodic cloud summary (first one goes out as soon as the cloud connects)
    if (Particle.connected() && (lastPublishMs == 0 || millis() - lastPublishMs >= PUBLISH_INTERVAL_MS)) {
        if (lastPublishMs == 0) cloudPublish(EVT_SEARCH, lastSearchSummary);
        cloudPublish(EVT_TEMPS, tempsVar);
        lastPublishMs = millis();
    }

    delay(2200);  // + ~800 ms conversion = ~3 s cycle
}
