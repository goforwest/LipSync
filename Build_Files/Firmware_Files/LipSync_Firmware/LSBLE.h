/* 
* File: LSBLE.h
* Firmware: LipSync
* Developed by: MakersMakingChange
* Version: v4.1 (28 March 2025)
  License: GPL v3.0 or later

  Copyright (C) 2024 - 2025 Neil Squire Society
  This program is free software: you can redistribute it and/or modify it under the terms of
  the GNU General Public License as published by the Free Software Foundation,
  either version 3 of the License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with this program.
  If not, see <http://www.gnu.org/licenses/>
*/
// 4.6.6: include guard. This header defines external-linkage objects
// (bledis, blehid, needsInitialization) and the free initializeBluefruit()
// — safe only because the Arduino build merges every tab into ONE
// translation unit and this file is included exactly once. A second
// inclusion (or any future .cpp consumer, e.g. the host-test pattern used
// for LSTimer) would be an immediate multiple-definition break; every other
// LS*.h already carries one.
#ifndef LSBLE_H
#define LSBLE_H
#include <bluefruit.h>
#include <nrf_sdm.h>

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)

BLEDis bledis;
BLEHidAdafruit blehid;
volatile bool needsInitialization = true;  // Written by the BLE init task, read by the loop task

class LSBLEMouse {

  public:
    inline LSBLEMouse(void);
    inline void begin(const char* s = "LipSync");
    inline void end(void);
    inline void move(signed char x, signed char y);
    inline void moveAll(signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
    inline void scroll(signed char wheel = 0);
    inline void pan(signed char pan = 0);
    inline void service(void);
    inline void click(uint8_t b = MOUSE_LEFT);
    inline void press(uint8_t b = MOUSE_LEFT);   // press LEFT by default
    inline void release(uint8_t b = MOUSE_LEFT); // release LEFT by default
    inline bool isPressed(uint8_t b = MOUSE_LEFT); // check LEFT by default
    inline bool isConnected(void);
  protected:
    uint8_t _buttons;
    void buttons(uint8_t b);
  private:
    bool mouseReport(signed char b, signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
    bool _buttonStatePending = false;
    bool _clickPending = false;
    bool _clickPressedSent = false;
    uint8_t _clickButton = 0;
};

// LSBLEKeyboard removed (4.5.7 review): never instantiated anywhere — a
// v4.0-era leftover whose keyboardReport() carried a delay(2) per report,
// the same copy-paste-invitation class that motivated 4.5.4's LSUSBKeyboard
// removal. (The btKeyReport struct went with it.)

// 4.5.18: the begin()-mirror probe is RETIRED — it completed its diagnostic
// mission (bench round 12: pstep=11, the whole SoftDevice sequence passed,
// 0x1001 dead) and the double-enable dance it performed (enable ->
// cfg_sets -> sd_ble_enable -> sd_softdevice_disable -> begin()'s second
// enable) is a nonstandard sequence no working recipe uses — the prime
// suspect for the st=2 DIS failure and the SD fault-resets that followed.
// begin() runs its OWN pre/post_enable hooks internally (USE_TINYUSB is
// defined for the lib), so this file no longer declares or calls them.

// 4.5.20/4.5.21: reset-proof verdict persistence — the task RAISES A FLAG
// (g_bleVerdictDirty, declared in LSApiDeclarations.h); the LOOP performs the
// filesystem write. The 4.5.20 shape wrote from this task using LSMEMORY's
// shared global File handle and corrupted the settings file (bench round 14
// — the re-seed signature: IN=0.00|0.00, CM reverted to default).
void initializeBluefruit(const char* s) {
  g_bleVerdictDirty = true;  // entry record: stage 0 = "died during begin" if this is the last write

  // 4.5.9 bench round 3: every begin()/start() is CHECKED and the failure is
  // printed. A failed SoftDevice enable previously cascaded into the
  // GATT/DIS/HID/advertising SVC calls on a dead stack — an SVC call with
  // the SD down HardFaults, the Seeed core's HardFault handler is an
  // INSTANT reset, and the BLE boot looped forever (bench 2026-10-25: the
  // first real CM=2 boot on this firmware line — every earlier CM=2 boot was
  // a safe-mode CDC-only boot that never touched Bluefruit). On failure we
  // bail with needsInitialization still true: every BLE call site is
  // guarded (isConnected, btFeedbackLoop), no SVC call is made, and the
  // boot trace names the failure instead of faulting.
  // 4.5.13 bench round 7: the 2026-10-26 06:45 session captured ble=3 st=1
  // sd=0 — the SoftDevice was DOWN at boot (the bootloader-leftover theory
  // is dead) and the witness was SD-legal, so the failure is INSIDE
  // AdafruitBluefruit::begin(), which swallows every return code
  // (VERIFY_STATUS(err, false)) and reports nothing but false. "st=1" has
  // therefore been over-claiming: begin() is enable + 8 cfg_sets +
  // sd_ble_enable (the classic APP-RAM-ceiling check) + opt/semaphore/task
  // calls, and any of them can be the one dying.
  // 4.5.14 bench round 8: the probe that MIRRORED begin()'s SoftDevice-level
  // sequence call-for-call. It did its job: bench round 12's log read
  // pstep=11 pcode=0x0 need=0x20002D80 — enable, all 8 cfg_sets and
  // sd_ble_enable ALL passed, the 0x1001 hunt was over, and begin() itself
  // succeeded for the first time on this line. The failure moved to st=2
  // (DIS begin) with an SD fault-reset right behind it — and the probe's
  // own double-enable dance (enable -> full BLE bring-up -> disable ->
  // begin()'s second enable: a double LFCLK cycle and a disable-while-BLE-
  // enabled no working recipe performs) became the prime suspect.
  // 4.5.18: the dance is RETIRED — begin() runs directly, the stock single
  // enable. pstep semantics on the wire: 0 = not a BLE boot; 11 = the
  // historical probe-passed build shape; 12 = the direct-begin shape this
  // firmware now ships. pcode=/need= stay 0 in this shape.
  uint8_t sdAlreadyEnabled = 0;
  (void) sd_softdevice_is_enabled(&sdAlreadyEnabled);
  g_bleSdWasEnabled = (sdAlreadyEnabled != 0) ? 1 : 0;
  if (sdAlreadyEnabled) {
    (void) sd_softdevice_disable();  // MBR-serviced SVC: legal whether or not the SD is up
    // 4.6.6: the SD is DOWN from this instant — drop the flag (setup seeded
    // it true for this exact leftover case) so no retention write takes the
    // SVC path into a disabled SoftDevice on the way to begin()'s re-enable.
    g_softDeviceEnabled = false;
  }
  // 4.5.17 bench round 11: THE PRIORITY MODEL, CORRECTED BY THE BENCH. The
  // classic "app must use 4-7 with the SoftDevice" rule is WRONG for this
  // S140 6.1.1 — the actual SD-reserved set is {0, 1, 4}, stated verbatim
  // by the core's own TinyUSB port ("Priorities 0, 1, 4 (nRF52) are
  // reserved for SoftDevice"), and proven on this bench: the stock example
  // passes sd_softdevice_enable's check with USBD enabled at priority 2,
  // while EVERY failing BLE boot in the hunt carried an enabled interrupt
  // at exactly 4 — the 4.5.13 witness (TIMER3 at 4 has blocked every enable
  // since 4.5.10, when the BLE init moved after the witness arms), the
  // 4.5.15 Wire "fix", and the 4.5.15/16 sweep's own USBD demotions. The
  // source fixes this round put everything we own at priority 7 (legal
  // under BOTH models, never masked by FreeRTOS's BASEPRI=2 sections);
  // this sweep is the belt-and-braces: any enabled IRQ found at 0, 1 or 4
  // is recorded (bad1=/bad2= bit masks, IRQ n = bit n — the BOOT/HB lines
  // carry them) and demoted to 7 (dem= counts) BEFORE the probe runs, so
  // the enable sees a legal NVIC whatever any future boot path enables.
  {
    for (int irq = 0; irq < 48; irq++) {  // nRF52840: 48 external IRQs (0-47)
      const uint32_t bit = 1UL << (irq & 31);
      volatile uint32_t* iser = (irq < 32) ? &NVIC->ISER[0] : &NVIC->ISER[1];
      const uint32_t prio = NVIC_GetPriority((IRQn_Type)irq);
      if ((*iser & bit) && (prio == 0 || prio == 1 || prio == 4)) {
        if (irq < 32) {
          g_bleIrqBad1 |= bit;
        } else {
          g_bleIrqBad2 |= bit;
        }
        NVIC_SetPriority((IRQn_Type)irq, 7);
        g_bleProbeDemoted++;
      }
    }
  }
  // 4.5.18: the probe's double-enable dance is retired (see the header
  // comment above). begin() runs directly below — the stock single enable,
  // with its own internal USE_TINYUSB pre/post_enable hooks, on the 16 KB
  // init task, with the stage checks and every guard this line has built.
  g_bleProbeStep = 12;  // the wire's build-shape marker: 12 = direct begin
  // 4.6.6: begin() ENABLES the SoftDevice inside this call. The retention
  // writers in the loop task (LED/serial trail stamps, the witness marker,
  // the streak clear) must not touch the POWER registers until the SD-up
  // flag can be raised — so the init window opens here and closes the
  // moment begin() returns, on every path. (Raising the flag itself early
  // is NOT an option: with the SD down the SVC path would hand SD-function
  // SVCs to the core's FreeRTOS SVC trampoline.)
  g_bleInitWindow = true;
  if (!Bluefruit.begin()) {
    g_bleInitStage = 1;
    // begin() can fail at a LATER internal stage with the SoftDevice already
    // enabled (the enable is only its first step). Re-probe instead of
    // assuming: a raw retention write with the SD actually up is an
    // APP_MEMACC kill. Order matters — the flag goes up FIRST, the window
    // closes second, so no intermediate state lets a stamp take the raw
    // path while the SD is up (flag true + window open → stamps skip;
    // flag true + window closed → SVC path; both safe).
    uint8_t sdUpAfterFail = 0;
    (void) sd_softdevice_is_enabled(&sdUpAfterFail);
    g_softDeviceEnabled = (sdUpAfterFail != 0);
    g_bleInitWindow = false;
    if (USB_DEBUG) { Serial.println("BOOT: BLE FAILED at SoftDevice enable"); }
    g_bleVerdictDirty = true;
    return;
  }
  // 4.5.27: THE SoftDevice is UP from this instant until the next reset —
  // every POWER-register access from here on MUST go through the SD SVCs
  // (see lipsyncRetentionUpdate in LSApiDeclarations.h; the Fable 5.1
  // consult pinned the entire BLE death saga on raw GPREGRET2 writes).
  // 4.6.6 ordering: flag FIRST, window closed SECOND — see the failure path.
  g_softDeviceEnabled = true;
  g_bleInitWindow = false;
  if (USB_DEBUG) { Serial.println("BOOT: BLE stack up"); }
  Bluefruit.Periph.setConnInterval(9, 16);  // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(4);                  // Check bluefruit.h for supported values
  Bluefruit.setName(s);
  bledis.setManufacturer("MakersMakingChange");
  bledis.setModel("LipSync Mouse");
  // 4.5.19 (expert consult, 2026-10-26): BLEDis::begin() and
  // BLEHidAdafruit::begin() return err_t — the RAW SoftDevice error code
  // (sd_ble_gatts_service_add's own value on the DIS failure path, via
  // VERIFY_STATUS). The stage checks were discarding it into a bool.
  // The code now rides the BOOT/HB lines as rc= so the next log answers
  // the expert's decisive question directly: 0x4 = NRF_ERROR_NO_MEM
  // (attr-table exhaustion — check the active table size), 0x7 = INVALID_PARAM,
  // 0xF = FORBIDDEN, 0x10 = INVALID_ADDR (pointers/corruption).
  {
    err_t disRc = bledis.begin();
    if (disRc != NRF_SUCCESS) {
      g_bleInitStage = 2;
      g_bleInitRc = disRc;
      if (USB_DEBUG) { Serial.println("BOOT: BLE FAILED at DIS begin"); }
      g_bleVerdictDirty = true;
      return;
    }
  }
  {
    err_t hidRc = blehid.begin();
    if (hidRc != NRF_SUCCESS) {
      g_bleInitStage = 3;
      g_bleInitRc = hidRc;
      if (USB_DEBUG) { Serial.println("BOOT: BLE FAILED at HID begin"); }
      g_bleVerdictDirty = true;
      return;
    }
  }
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  // 4.5.12 bench fix: ONE appearance, exactly like the core's stock
  // blehid_mouse example. The 4.1.0-era sequence added BOTH keyboard and
  // mouse appearances, and the arithmetic sits exactly at the 31-byte
  // legacy advertising limit: flags(3) + txpower(3) + appearance(4)x2 +
  // service UUID(4) + name "LS_xxxxxxxx"(13) = 31 with ZERO margin — any
  // device whose name runs one byte longer makes addName/start fail. The
  // advertised appearance is cosmetic; the HID service still carries the
  // full keyboard+mouse report descriptors untouched (the wire HID
  // contract is unchanged).
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // 20 ms - in unit of 0.625 ms (Interval:  fast mode = 20 ms, slow mode = 152.5 ms)
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  if (!Bluefruit.Advertising.start(0)) {         // 0 = Don't stop advertising after n seconds
    g_bleInitStage = 4;
    if (USB_DEBUG) { Serial.println("BOOT: BLE FAILED at advertising start"); }
    g_bleVerdictDirty = true;
    return;
  }
  if (USB_DEBUG) { Serial.println("BOOT: BLE advertising"); }
  g_bleInitStage = 5;
  needsInitialization = false;  // FULL SUCCESS ONLY — every guarded call site keys on this
  g_bleVerdictDirty = true;  // st=5: if the boot STILL dies after this, the fault is past the init — advertising/runtime
}


/*****************************
     MOUSE SECTION
 *****************************/

LSBLEMouse::LSBLEMouse(void)
{

}

void LSBLEMouse::begin(const char* s)
{
  _buttons = 0;
  if (needsInitialization) {
    initializeBluefruit(s);
    // needsInitialization is now cleared by initializeBluefruit itself, ONLY
    // on full success (4.5.9) — a failed begin leaves every guarded call site
    // safely reporting "not connected" instead of touching a dead stack.
  }
}


void LSBLEMouse::end(void)
{
}

bool LSBLEMouse::mouseReport(int8_t b, int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
  // 4.5.10: the BLE stack comes up on its own task now (see the 16 KB
  // init task in LipSync_Firmware.ino) — an SVC call on a never-begun or
  // still-enabling SoftDevice HardFaults, which is a whole boot class
  // worse than a dropped report. Skip until needsInitialization clears;
  // the buttons state still mutates, so the first post-init movement
  // report carries it.
  if (needsInitialization || !Bluefruit.connected()) return false;
  bool sent = blehid.mouseReport(b, x, y, wheel, pan);
  if (sent) _buttonStatePending = false;
  return sent;
}

void LSBLEMouse::move(int8_t x, int8_t y)
{
  mouseReport(_buttons, x, y, 0, 0);
}
void LSBLEMouse::moveAll(int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
  mouseReport(_buttons, x, y, wheel, pan);
}

void LSBLEMouse::scroll(int8_t wheel)
{
  mouseReport(_buttons, 0, 0, wheel, 0);
}

void LSBLEMouse::pan(int8_t pan)
{
  mouseReport(_buttons, 0, 0, 0, pan);
}

void LSBLEMouse::service(void)
{
  if (!_clickPending && !_buttonStatePending && _buttons == 0) return;
  if (needsInitialization || !Bluefruit.connected()) {
    // A disconnect releases HID state at the host. Do not replay a stale
    // pre-disconnect click OR a settled drag when a different host reconnects
    // later. This check must precede the "nothing pending" fast return: a
    // successfully-sent drag has _buttons set but no pending flag.
    _buttons = 0;
    _buttonStatePending = false;
    _clickPending = false;
    _clickPressedSent = false;
    _clickButton = 0;
    return;
  }
  if (!_clickPending && !_buttonStatePending) return;  // Settled nonzero button state; nothing to transmit while connected

  if (_clickPending) {
    if (!_clickPressedSent) {
      _buttons |= _clickButton;
      if (mouseReport(_buttons, 0, 0, 0, 0)) _clickPressedSent = true;
    } else {
      _buttons &= (uint8_t)~_clickButton;
      if (mouseReport(_buttons, 0, 0, 0, 0)) {
        _clickPending = false;
        _clickPressedSent = false;
        _clickButton = 0;
      }
    }
    return;
  }

  if (_buttonStatePending) mouseReport(_buttons, 0, 0, 0, 0);
}

void LSBLEMouse::click(uint8_t b)
{
  if (_clickPending || needsInitialization || !Bluefruit.connected()) return;
  _clickButton = b & MOUSE_ALL;
  if (_clickButton == 0) return;
  _clickPending = true;
  _clickPressedSent = false;
  service();
}

void LSBLEMouse::buttons(uint8_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
    if (!mouseReport(_buttons, 0, 0, 0, 0)) _buttonStatePending = true;
  }
}

void LSBLEMouse::press(uint8_t b)
{
  buttons(_buttons | b);
}

void LSBLEMouse::release(uint8_t b)
{
  buttons(_buttons & ~b);
}

bool LSBLEMouse::isPressed(uint8_t b)
{
  if ((b & _buttons) > 0)
    return true;
  return false;
}

bool LSBLEMouse::isConnected(void) {
  // 4.5.7 bench fix: Bluefruit.connected() on a SoftDevice that was never
  // begun (needsInitialization true) HANGS the firmware — the bench caught
  // it live: CM,1:2 on a USB session reached setLedDefault() BEFORE the
  // hand-off's btmouse.begin(), isConnected() never returned, no ack ever
  // printed, and the 30 s watchdog killed the device. Every caller that can
  // run pre-begin (setLedDefault's BLE branch, btFeedbackLoop) now gets an
  // honest "not connected" instead of a dead loop.
  if (needsInitialization) return false;
  return Bluefruit.connected();
}



// (KEYBOARD SECTION removed with LSBLEKeyboard — 4.5.7 review:
//  never instantiated, and its keyboardReport carried delay(2) per
//  report — the copy-paste-invitation class 4.5.4's LSUSBKeyboard
//  removal set the precedent for)

/*****************************
     MULTI-HOST BLUETOOTH SWITCHING (upstream #16, firmware 4.8.0+)
 *****************************/

// The host table IS the vendored lib's bond directory: one LittleFS file per
// bonded peer under /adafruit/bond_prph/<12-hex-MAC>, holding the 80-byte
// keyset AND the peer name captured at pair time (bonding.cpp bond_save_keys_
// dfr). This section enumerates those files, disconnects/re-advertises for a
// switch, removes single bonds, and pushes MANUAL,0:BT,4/BT,5 state lines to
// the app with the CH,5 contract (MANUAL + seq 0 matches no command waiter).
// No settings file, no settings keys — see docs/design/16-multi-host-ble.md.

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include "utility/bonding.h"  // BOND_DIR_PRPH + bond_remove_key (vendored lib, public header)

using namespace Adafruit_LittleFS_Namespace;

// Push change-detect state. The CH,5 doctrine (LSScreen's
// _lastReportedMenuState): the FIRST enumeration after boot PRIMES this
// state silently — no unsolicited MANUAL lines on a healthy boot (the
// 4.5.29 wire contract); the app's connect load reads BT,0 for the current
// truth. Every real change after the prime pushes BT,4/BT,5.
uint8_t g_btPushedHostCount = 0xFF;
int g_btPushedActiveRow = -1;
bool g_btPushedPrimed = false;

bool lsBleStackReady() {
  return !needsInitialization;
}

// 12-hex bond address of the currently connected host into out13 (13 bytes).
// A connected but NOT-bonded peer has no row (no bond file exists yet) — the
// app sees BT,5:0 and the card shows "connected, not paired". False when the
// stack is down, nothing is connected, or the peer is unbonded.
bool lsHostActiveMac(char* out13) {
  out13[0] = '\0';
  if (needsInitialization) return false;
  if (Bluefruit.connected() == 0) return false;
  uint16_t hdl[1];
  if (Bluefruit.getConnectedHandles(hdl, 1) == 0) return false;
  BLEConnection* conn = Bluefruit.Connection(hdl[0]);
  if (conn == NULL || !conn->bonded()) return false;
  // getPeerAddr returns the BOND identity address for bonded connections
  // (BLEConnection.cpp) — the same bytes the bond file is named by.
  ble_gap_addr_t peer = conn->getPeerAddr();
  sprintf(out13, "%02X%02X%02X%02X%02X%02X",
          peer.addr[0], peer.addr[1], peer.addr[2],
          peer.addr[3], peer.addr[4], peer.addr[5]);
  return true;
}

// Enumerate the bond directory into MAC-sorted rows (the deterministic row
// order the wire and the Hub page both use). Returns the count of rows the
// wire actually exposes — min(total bonds, cap), the MAC-lowest `cap` when
// more bonds exist (bounded top-K, so the listed rows are deterministic
// regardless of filesystem order). The return is CAPPED, never the raw total:
// every consumer keys row indices and counts off this value, so a 10-bond
// directory must answer 8 — a count of 10 with only 8 readable rows sent
// executors indexing rows[] past what this function filled (uninitialized
// stack read) and promised the app rows the BT,0 getter would refuse. Rows
// carry the sanitized wire name (the lib caches the peer's GAP name in the
// file at pair time) and the active flag. Local File handles throughout —
// the 4.5.20 lesson: the shared LSMemory handle must never be touched from
// here.
//
// CROSS-TASK NOTE (4.8.1 review): this runs on the loop task while the BLE
// stack's deferred callbacks (bond_save_keys_dfr) write bond files from the
// core's callback task. That is SAFE at the operation level — the core's
// Adafruit_LittleFS takes a static FreeRTOS mutex around EVERY public
// File/FS operation (Adafruit_LittleFS.cpp _lockFS), so lfs itself is never
// touched concurrently. The 4.5.20 corruption was two tasks interleaving a
// SEQUENCE of operations through ONE shared File handle — the reason this
// function (and every BT path) uses only local handles. An enumeration that
// overlaps a bond write simply sees the directory before or after the
// commit; the next change-detected pass picks the result up.
uint8_t lsHostEnumerate(lsHostRow* rows, uint8_t cap) {
  if (rows == NULL || cap == 0) return 0;
  char activeMac[LSHOST_MAC_CHARS + 1];
  const bool haveActive = lsHostActiveMac(activeMac);

  if (!InternalFS.exists(BOND_DIR_PRPH)) return 0;
  File dirFile(BOND_DIR_PRPH, FILE_O_READ, InternalFS);
  if (!dirFile) return 0;

  uint8_t filled = 0;
  File f(InternalFS);
  while ((f = dirFile.openNextFile(FILE_O_READ))) {
    if (f.isDirectory()) { f.close(); continue; }

    // The bond filename stem is the 12-hex MAC (bonding.cpp get_fname);
    // take the basename defensively — name() has carried full paths in
    // some Adafruit_LittleFS revisions.
    const char* path = f.name();
    const char* stem = path;
    for (const char* p = path; *p != '\0'; p++) {
      if (*p == '/') stem = p + 1;
    }
    bool macOk = true;
    for (int i = 0; i < LSHOST_MAC_CHARS; i++) {
      const char c = stem[i];
      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) { macOk = false; break; }
    }
    if (!macOk || stem[LSHOST_MAC_CHARS] != '\0') { f.close(); continue; }

    lsHostRow row;
    strncpy(row.mac, stem, LSHOST_MAC_CHARS);
    row.mac[LSHOST_MAC_CHARS] = '\0';
    row.active = (haveActive && strcmp(row.mac, activeMac) == 0) ? 1 : 0;
    // Last MAC byte → the "(XX)" OLED discriminator.
    char byteHi[3] = { stem[LSHOST_MAC_CHARS - 2], stem[LSHOST_MAC_CHARS - 1], '\0' };
    row.lastMacByte = (int)strtoul(byteHi, NULL, 16);

    // File layout (bonding.cpp): 1-byte length + 80-byte keyset, 1-byte
    // length + name (NUL included), 1-byte length + CCCD. Read the name.
    char rawName[CFG_MAX_DEVNAME_LEN + 1] = { 0 };
    const int keyLen = f.read();
    if (keyLen > 0) f.seek(keyLen + f.position());
    const int nameLen = f.read();
    if (nameLen > 0) {
      const int toRead = (nameLen > CFG_MAX_DEVNAME_LEN) ? CFG_MAX_DEVNAME_LEN : nameLen;
      f.read((uint8_t*)rawName, toRead);
      rawName[CFG_MAX_DEVNAME_LEN] = '\0';
    }
    lsHostSanitizeName(rawName, row.name, sizeof(row.name));
    if (row.name[0] == '\0') {
      // Nameless bond (name read failed at pair time): fall back to the MAC
      // the way the lib's saver does (bonding.cpp:119-123).
      lsHostSanitizeName(row.mac, row.name, sizeof(row.name));
    }
    f.close();

    if (filled < cap) {
      rows[filled++] = row;
    } else {
      // Keep the cap MAC-lowest rows: replace the largest if this is smaller.
      uint8_t worst = 0;
      for (uint8_t i = 1; i < cap; i++) {
        if (strcmp(rows[i].mac, rows[worst].mac) > 0) worst = i;
      }
      if (strcmp(row.mac, rows[worst].mac) < 0) rows[worst] = row;
    }
  }
  dirFile.close();

  lsHostSortRows(rows, filled);
  // The wire count is the capped count, never the raw total — see the header
  // comment. `filled <= cap` by construction.
  return filled;
}

bool lsHostDisconnectCurrent() {
  if (needsInitialization) return false;
  if (Bluefruit.connected() == 0) return true;  // nothing to do — honest success
  uint16_t hdl[1];
  if (Bluefruit.getConnectedHandles(hdl, 1) == 0) return true;
  return Bluefruit.disconnect(hdl[0]);
}

// Any central connected, bonded or not — the pair-new gate (a new pairing
// needs advertising; advertising needs no connection).
bool lsHostAnyConnected() {
  if (needsInitialization) return false;
  return Bluefruit.connected() != 0;
}

void lsHostEnsureAdvertising() {
  if (needsInitialization) return;
  // restartOnDisconnect(true) re-advertises after a disconnect (BLEAdvertising
  // event handler); this covers the paths that bypass a disconnect (pair-new,
  // a set stopped for any other reason). Same forever-timeout as the boot path.
  if (!Bluefruit.Advertising.isRunning()) {
    Bluefruit.Advertising.start(0);
  }
}

// Remove one bond file. The filename keying uses the 6 address BYTES only
// (bonding.cpp get_fname ignores addr_type), so any type builds the right
// name; the identity bytes come from the enumerated row. 4.8.1: the lib's
// bond_remove_key returns void and discards InternalFS.remove's result, so
// it cannot report failure — this wrapper removes explicitly and VERIFIES
// (absent after = removed), so a failed removal refuses the BT,2/Hub forget
// (FAIL,3 + named line) instead of acking SUCCESS for a host that is still
// bonded.
bool lsHostForgetByMac(const char* mac12) {
  if (mac12 == NULL || strlen(mac12) != LSHOST_MAC_CHARS) return false;
  ble_gap_addr_t id;
  memset(&id, 0, sizeof(id));
  id.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
  for (int i = 0; i < 6; i++) {
    char byteStr[3] = { mac12[i * 2], mac12[i * 2 + 1], '\0' };
    id.addr[i] = (uint8_t)strtoul(byteStr, NULL, 16);
  }
  char fname[48];  // sizeof("/adafruit/bond_prph/%02X%02X%02X%02X%02X%02X") — the lib's get_fname is internal to bonding.cpp, so build the documented name from the public format macro
  snprintf(fname, sizeof(fname), BOND_FNAME_PRPH,
           id.addr[0], id.addr[1], id.addr[2], id.addr[3], id.addr[4], id.addr[5]);
  if (!InternalFS.exists(fname)) return true;  // already absent — forget is idempotent
  if (!InternalFS.remove(fname)) return false;
  return !InternalFS.exists(fname);  // verify — a "removed but still there" is a refusal, not a SUCCESS
}

// The push half of the BT surface (the CH,5 pattern): whenever the host list
// length or the active row changes after the boot prime, print
// MANUAL,0:BT,4:<count> / MANUAL,0:BT,5:<activeRow> — MANUAL + seq 0 matches
// no command waiter (LSScreen.h reportMenuState contract). Called every
// btFeedbackLoop pass (1 s) and after a BT,2 forget. The FIRST poll call
// after boot primes silently (see g_btPushedPrimed): a healthy boot stays
// silent, and the app's connect load reads BT,0 for the current state.
// force = true (the forget paths): a BT,2 that lands BEFORE the first poll
// pass must still deliver its contractual BT,4 — a forced call skips the
// prime-swallow and pushes the (still-sentinel) changed values.
void lsHostPushState(bool force) {  // default lives on the LSApiDeclarations.h declaration
  if (needsInitialization) return;
  lsHostRow rows[LSHOST_ENUM_CAP];
  const uint8_t count = lsHostEnumerate(rows, LSHOST_ENUM_CAP);
  int activeRow = 0;
  for (uint8_t i = 0; i < count && i < LSHOST_ENUM_CAP; i++) {
    if (rows[i].active) { activeRow = i + 1; break; }
  }
  if (!g_btPushedPrimed && !force) {
    g_btPushedPrimed = true;
    g_btPushedHostCount = count;
    g_btPushedActiveRow = activeRow;
    return;
  }
  g_btPushedPrimed = true;  // a forced first call syncs fully (sentinels still differ -> both lines below fire)
  if (count != g_btPushedHostCount) {
    g_btPushedHostCount = count;
    printResponseInt(true, false, true, 0, "BT,4", true, count);
  }
  if (activeRow != g_btPushedActiveRow) {
    g_btPushedActiveRow = activeRow;
    printResponseInt(true, false, true, 0, "BT,5", true, activeRow);
  }
}

// Named refusal line for the wire — a refused BT command must never read as
// a flash failure (the budget-refusal precedent, LSMemory writeObject).
void lsHostRefusalLine(String commandKey, int reason, int row) {
  String line = "ERROR: " + commandKey + " refused (row " + String(row) + "): ";
  switch (reason) {
    case LSHOST_REFUSE_NOT_INITIALIZED: line += "the BLE stack is not initialized (USB-mode boot or init failure)"; break;
    case LSHOST_REFUSE_CALIBRATION:     line += "calibration/center flow in flight"; break;
    case LSHOST_REFUSE_LATCH:           line += "a drag/scroll hold is engaged — release it first"; break;
    case LSHOST_REFUSE_FROZEN:          line += "a menu transition owns the display — retry in a moment"; break;
    case LSHOST_REFUSE_RANGE:           line += "no such host row (re-read BT,0 after any forget)"; break;
    case LSHOST_REFUSE_ALREADY_ACTIVE:  line += "this host is already the connected one"; break;
    case LSHOST_REFUSE_RADIO:           line += "the radio refused the operation — retry in a moment"; break;
    default:                            line += "unknown reason"; break;
  }
  serialTxEnqueueLine(line);
}

#endif  // LSBLE_H
