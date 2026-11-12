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
#include "LSHostSwitch.h"  // the pure host-switch layer: rows, refusals, the 4.9.0 RPA admission helpers

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
    inline bool takeDisconnectedWithButtonState(void);  // take-and-clear: the BLE host went away while this object still held click/drag state (4.9.0 — the 4.8.1 USB-flag mirror)
  protected:
    uint8_t _buttons;
    void buttons(uint8_t b);
  private:
    bool mouseReport(signed char b, signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
    bool _buttonStatePending = false;
    bool _clickPending = false;
    bool _clickPressedSent = false;
    bool _disconnectedWithButtonState = false;  // raised by service()'s disconnect cleanup, consumed once by the loop
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
    // (4.9.0) raising the take-and-clear flag when button state was actually
    // held — the loop ends the DRAG latch with it. btFeedbackLoop's 1 s
    // disconnected-observation misses a drop→sub-second-reconnect (bench
    // 4.8.14: macOS reclaims the link sub-second), which used to leave the
    // latch alive with the wire button already cleared — cursor moves, no
    // button, until the user's release self-corrected it. The exact mirror
    // of LSUSBMouse::service's unmount flag; no action-bearing event.
    // 4.9.1: the mirror is now exact — pending CLICK or BUTTON state with
    // _buttons already cleared (a release whose transmit the drop ate) is
    // the same lost-state class, and the USB twin covers it.
    if (_buttons != 0 || _clickPending || _buttonStatePending) _disconnectedWithButtonState = true;
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

bool LSBLEMouse::takeDisconnectedWithButtonState(void) {
  // Take-and-clear (4.9.0): raised by service()'s disconnect cleanup when
  // click/drag state was actually held; consumed once by the loop. Mirrors
  // LSUSBMouse::takeUnmountedWithButtonState.
  const bool v = _disconnectedWithButtonState;
  _disconnectedWithButtonState = false;
  return v;
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

/*****************************
   MULTI-HOST SWITCHING — THE SWITCH WINDOW (upstream #16, bench B4 gate)
   *****************************
   The wrong-host race (bench B4, 2026-11-06): when switching AWAY from the
   just-disconnected host, THAT host reconnects faster than the target — macOS
   grabs the link back sub-second (0/5 away-from-Mac) and the target never gets
   a window. Bench trail 4.8.3-4.8.17 (design doc Q4 addendum): the
   SoftDevice-filter contingency is NOT implementable on S140 6.1.1 —
   whitelist+discoverable is 0x3201-forbidden, no legacy connectable-
   non-scannable type exists, and the directed target field AND the whitelist
   filter are not enforced on a directed set (macOS connect-by-address walked
   into every filtered window). The shipped mechanism is therefore DEVICE-SIDE:
   the window advertises DIRECTED at the target identity (the spec's
   fast-reconnect mechanism, 4.8.17 device-side trial) and the BOUNCER (4.8.12)
   enforces the lockout in firmware — any non-target connection is dropped
   within milliseconds and the window keeps waiting (the just-disconnected
   host's reconnect engine cycles harmlessly; the target's connect is the one
   that holds). The 4.8.14-16 open-advertising shape is one flag away (begin's
   comment records the exact substitution) if directed fails the bench trial.

   Window lifetime (all loop-task; g_switchWindowOpen is the single flag):
   - ARMED before the disconnect (lsHostArmSwitchWindow) — stops any running
     stock advertising (the window replaces it) and loads the target bond's
     identity + IRK through the lib's public bond_load_keys. NO SoftDevice
     whitelist or identity list is armed (see the bench trail above); the IRK
     is captured for the tick's RPA resolver (an iOS-style privacy target
     reconnects with a rotating RPA whose bytes never equal the stored
     identity — the resolution admits it pre-bond).
   - SUSPENDED auto-restart for the window lifecycle (4.8.7 review): the stock
     DISCONNECTED handler restarts OPEN (FP_ANY) advertising the moment the
     disconnect event lands on the BLE event thread — a sub-second window in
     which the just-disconnected host wins the link back (bench B4: 0/5) and
     the tick would then report a false "target won". The executor suspends
     restartOnDisconnect across the switch, so NOTHING is on air between the
     disconnect and the window's directed advertise; the flag is restored on
     every window ending (close, cancel, every fallback).
   - BEGUN after the disconnect (lsHostBeginSwitchWindow) — stock open
     advertising (start(0)) is (re)started, held for the window's bound with
     the tick's keep-open ensure; CONN_COUNT during the disconnect teardown is
     retried bounded (the role is freed milliseconds later).
   - CLOSED by lsHostSwitchWindowTick (polled from the main loop at
     millisecond granularity, btFeedbackLoop as the 1 s backstop): a host
     connected whose identity MATCHES the armed target — by stored identity
     bytes, or by RPA resolution against the armed IRK, or (IRK-less bonds)
     after the bond grace flips getPeerAddr to the stored identity — or the
     millis bound elapsed. Closing restores the stock radio state: auto-
     restart re-armed, disarm best-effort, open advertising resumed when no
     host is connected. A non-target connection is bounced (named on the
     wire) and NEVER reported as a successful switch; past the storm bound
     the window closes with the named MISMATCH report.
   Fallback everywhere is OPEN advertising — the pre-contingency behavior —
   never a wedge: a failed SD call degrades to the race, it does not hang the
   radio (the B2/B3 evidence shows open advertising still completes many
   switches; the window exists to make the race structurally impossible).

   Address/IRK source: the lib's public bond_load_keys (bonding.h) — the same
   path the reconnect crypto uses — NOT hand-parsed bond-file offsets (the
   first draft parsed offsets and got both the name-read length semantics and
   the IRK offset wrong; the lib function is RPA-aware and offset-proof).
*/

#define LSHOST_SWITCH_WINDOW_TIMEOUT_S  60  // window bound (long, on purpose — the device-side trial ladder for Windows' reconnect; the bouncer holds the Mac off, and the bound still guarantees the switch can never strand the radio)
#define LSHOST_SWITCH_WINDOW_TIMEOUT_MS (LSHOST_SWITCH_WINDOW_TIMEOUT_S * 1000UL)
#define LSHOST_TICK_MAX_CONNECTIONS 2  // tick identity check: the peripheral holds ONE link during a window (Bluefruit.begin() runs prph_count=1, so a second link is impossible at the SD level — 2 is defensive headroom for getConnectedHandles, not a real topology)
#define LSHOST_BOUNCE_DECISION_GRACE_MS 3000UL  // an RPA peer whose identity cannot be decided from the connect bytes (IRK-less bond) gets this long to complete bonding — bond completion flips getPeerAddr to the stored identity; past the grace it is bounced as a non-target

// --- window state + directed target (single instance; loop task only). One
// switch window at a time is enforced by construction (the executor is the
// only opener and the tick the only closer).
static bool g_switchWindowOpen = false;
static unsigned long g_switchWindowStartMs = 0;
static ble_gap_addr_t g_switchWlAddr;                 // directed target (the bond's stored identity, coerced to its bytes)
// (4.9.0 review) the g_switchIdKey identity-list storage is gone with the SD
// lists themselves (bench 4.8.8-4.8.17: neither list is armable for this
// advertising class); the IRK lives in g_switchTargetIrk below, captured for
// the tick's RPA resolver.
static uint8_t g_switchTargetIrk[BLE_GAP_SEC_KEY_LEN];  // the target's IRK when the bond carries one
static bool g_switchTargetIrkValid = false;           // true = g_switchTargetIrk is a non-zero IRK the tick may resolve RPAs against

// Ensure open advertising is running; returns whether advertising IS running
// on return (4.9.0: the rc is checked — the bench trail's adv_start refusals
// in the post-disconnect SD-teardown window are real, and BT,3 must not ack
// SUCCESS while nothing is on air). Same forever-timeout as the boot path.
// NOTE (round-3 review): with a switch window open it returns FALSE while
// the window's directed set may be on air — false means "open advertising is
// not (being) started", not "the radio is silent". Both boolean consumers
// pre-gate on lsHostSwitchWindowOpen() for exactly this reason; keep that
// gate on any new caller.
// (4.9.0 round-2 review) NEVER while a switch window is open: between the
// directed set's ~1 s arms nothing is on air BY DESIGN, and the old shape
// would treat that gap as "not running" and start open advertising — the
// just-disconnected racer's favorite door, and the tick would then skip
// re-arming the directed set (isRunning stays true). The window owns the
// radio until it closes; the close path clears the flag before calling this.
bool lsHostEnsureAdvertising() {
  if (needsInitialization) return false;
  if (g_switchWindowOpen) return false;  // the window owns the radio
  // restartOnDisconnect(true) re-advertises after a disconnect (BLEAdvertising
  // event handler); this covers the paths that bypass a disconnect (pair-new,
  // a set stopped for any other reason).
  if (!Bluefruit.Advertising.isRunning()) {
    Bluefruit.Advertising.start(0);  // rc checked via the isRunning probe below — start() can refuse in the teardown window
  }
  return Bluefruit.Advertising.isRunning();
}


// Auto-restart suspension across the window lifecycle (4.8.7 review). The
// stock value is true (set once at stack init below); the lib offers no
// getter, so the suspension saves that known stock value and every window
// ending restores it. Only this file's window code ever writes the flag.
static bool g_switchAutoRestartSuspended = false;

// Begin-failure forensics (4.8.11): the connection count and the total
// CONN_COUNT retry wait at the moment lsHostBeginSwitchWindow gave up — the
// executor's fallback INFO line appends them so a repeat teardown-race
// failure names its state on the wire (bench 4.8.10 spent its whole retry
// budget without saying anything about the connection).
static int g_switchWindowFailConn = -1;
static int g_switchWindowFailWaitedMs = 0;

// Bouncer counter (4.8.12): non-target connects dropped during the current
// window; reset when a window opens/closes. Also the storm bound — past
// LSHOST_BOUNCE_MAX REAL connections the window closes with the named
// mismatch report.
static int g_switchBounceCount = 0;
// Per-connection bounce tracking (4.8.16 — bench 4.8.15: ONE Mac connection
// was counted 600x in 200 ms; the tick spun against the same dying handle
// while the SD tore the link down, tripped the cap, and closed the window).
// A bounce is counted/logged ONCE per connection: while the same handle is
// still present it is the SAME connection tearing down (skip quietly); the
// tracking clears when the link goes empty, so a reconnecting peer (even on
// a reused handle number) counts as new. 4.9.0: the tracking now ALSO carries
// the bounce-decision grace — the handle's first observation starts an
// LSHOST_BOUNCE_DECISION_GRACE_MS window (an undecidable RPA peer gets bond
// time; a public/static non-target is decided at first sight), and a handle
// that lingers far past its bounce is treated as a NEW connection (a refused
// disconnect must never surrender the lockout quietly forever).
static bool g_switchBouncedActive = false;
static uint16_t g_switchBouncedHdl = 0xFFFF;
static unsigned long g_switchBouncedConnectMs = 0;  // first tick observation of the tracked handle
static bool g_switchBounceCounted = false;          // the tracked handle's bounce already fired (same-handle passes are teardown, not new connections)
static unsigned long g_switchBounceDispatchedMs = 0; // 4.9.4: when the tracked handle's bounce actually dispatched (0 = none yet) — the teardown horizon is measured from here, not from connect
// (4.9.0 round-3 review) DEFERRED ADVERTISING RESTART: a window ending can
// land in the sub-second gap where the directed set has self-terminated but
// the SD's ADV_SET_TERMINATED event has not been dispatched — the lib's
// _runnning flag still reads true, stop() is refused, and the termination
// event afterward clears the flag WITHOUT restarting (the directed branch).
// A one-shot ensureAdvertising probe in that gap reads "running" and skips
// the start: silent radio-off. The flag is set by every window ending while
// no host is connected, and serviced per loop pass by
// lsHostServiceSwitchWindow once the event has actually landed.
// 4.9.1: the service can no longer CLEAR the flag on a young isRunning()
// reading — that is exactly the stale reading the flag exists for (the old
// shape cleared it in the gap, the event landed afterward, and the radio
// went silently off). The arm time bounds the trust: the event gap is
// sub-second, so a set still flagged "running" well past the bound is
// genuinely on air and the pending settles.
static bool g_switchAdvRestartPending = false;
static unsigned long g_switchAdvRestartPendingMs = 0;
#define LSHOST_ADV_RESTART_STALE_RUN_MS 2000UL

// 4.9.1: the HID-subscription watchdog's grace must not survive a switch
// window — the window's early return freezes a mid-grace anchor, and the
// link that connects afterward (the freshly switched-to target) would
// inherit the elapsed timer and be disconnected on its first post-window
// pass, mid-CCCD-restore. Every window ending calls this; the watchdog
// block defines it. (Forward declaration here — the window code sits
// above the watchdog in this file.)
static void lsHidWatchdogResetGrace(void);


// Suspend the stock DISCONNECTED auto-restart (open FP_ANY advertising) for
// the switch lifecycle: between the disconnect and the window's filtered
// advertise nothing may be on air, or the just-disconnected host wins the
// gap (bench B4: sub-second reconnection).
static void lsHostSuspendAutoRestart(void) {
  if (!g_switchAutoRestartSuspended) {
    g_switchAutoRestartSuspended = true;
    Bluefruit.Advertising.restartOnDisconnect(false);
  }
}

// Restore stock auto-restart (every window ending: close, cancel, fallback).
static void lsHostRestoreAutoRestart(void) {
  if (g_switchAutoRestartSuspended) {
    g_switchAutoRestartSuspended = false;
    Bluefruit.Advertising.restartOnDisconnect(true);
  }
}

// Coerce an address type that contradicts the bytes' top two bits — the SD
// validates "upper two bits correspond to the specified address type" on
// whitelist and identity-list entries, and a stored BOND type can disagree
// with the actual bytes (bench 2026-11-07: the Mac's bond stored type=0
// PUBLIC for bytes carrying the 11 marker of a random static address → the
// window's adv_set_configure refused with 0x3201
// BLE_ERROR_GAP_INVALID_BLE_ADDR). Convention: top two bits 0b11 = static
// random, anything else = public/global for list-entry purposes; RPA/RRA
// types (caution: those never belong in either list as an identity) coerce
// by the same byte convention. Coercion keeps the identity list and the
// whitelist entry CONSISTENT — the SD resolves one against the other.
static inline void lsHostCoerceAddrType(ble_gap_addr_t* a) {
  const uint8_t topBits = (uint8_t)(a->addr[5] >> 6);
  if (a->addr_type == BLE_GAP_ADDR_TYPE_PUBLIC) {
    if (topBits == 0x03) a->addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;  // PUBLIC cannot carry the 11 marker
  } else if (a->addr_type == BLE_GAP_ADDR_TYPE_RANDOM_STATIC) {
    if (topBits != 0x03) a->addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;         // static random must carry 11
  } else {
    a->addr_type = (topBits == 0x03) ? BLE_GAP_ADDR_TYPE_RANDOM_STATIC : BLE_GAP_ADDR_TYPE_PUBLIC;
  }
}

// Text form of the armed whitelist entry for the wire INFO line ("AA:BB:..:FF/N").
static char g_switchWlText[24];

// Read the target bond's identity (address + type + IRK) through the lib's own
// loader. inMac12 is the 12-hex bond-file stem; the filename lookup ignores
// addr_type (bonding.cpp get_fname), so the parse below is only the lookup
// key. The AUTHORITATIVE identity+IRK come from the loaded keyset
// (bond_keys_t.peer_id = ble_gap_id_key_t) — the bytes the host actually uses
// on the air, including its real address type (the Mac/Windows bonds are
// static-address, iOS-style bonds are RPA+IRK). Returns false when the bond is
// absent or the loader refuses (caller falls back to open advertising).
static bool lsHostBondReadIdentity(const char* inMac12, ble_gap_id_key_t* outId) {
  if (inMac12 == NULL || strlen(inMac12) != LSHOST_MAC_CHARS || outId == NULL) return false;
  ble_gap_addr_t lookup;
  memset(&lookup, 0, sizeof(lookup));
  lookup.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;  // filename lookup ignores type
  for (int i = 0; i < 6; i++) {
    char byteStr[3] = { inMac12[i * 2], inMac12[i * 2 + 1], '\0' };
    lookup.addr[i] = (uint8_t)strtoul(byteStr, NULL, 16);
  }
  bond_keys_t keys;
  memset(&keys, 0, sizeof(keys));
  if (!bond_load_keys(BLE_GAP_ROLE_PERIPH, &lookup, &keys)) return false;
  *outId = keys.peer_id;
  // Degenerate guard: a keyset whose stored identity is all-zero (never
  // observed in the lib's writer, but a zero whitelist entry would be a
  // wildcard-shaped hole). Fall back to the bond-file stem bytes.
  static const uint8_t zero[6] = { 0 };
  if (memcmp(outId->id_addr_info.addr, zero, 6) == 0) {
    outId->id_addr_info = lookup;
  }
  return true;
}

// ARM (call while the old host is still connected, BEFORE disconnecting):
// stops any running stock advertising (replaced by the window) and loads the
// target bond's identity + IRK through the lib's bond loader. The identity
// becomes the directed advertise's target (begin) and the tick's identity
// match; a non-zero IRK is captured for the tick's RPA resolver (an
// iOS-style privacy target reconnects with a rotating RPA whose bytes never
// equal the stored identity — resolution admits it pre-bond). NO SoftDevice
// whitelist or identity list is armed (bench trail 4.8.3-4.8.17: the SD's
// filters are not enforceable for this advertising class — the bouncer is
// the lockout). Any refusal returns false (the executor then uses open
// advertising). The arm/begin outcomes are NAMED on the wire by the
// executor's INFO line — bench 2026-11-07: the silent fallback left a failed
// window indistinguishable from a working one (the B4 prep switch was
// intercepted by the iPhone and the
// wire could not say whether the window ever opened).
bool lsHostArmSwitchWindow(const char* targetMac12, int* outAddrType, int* outHasIrk) {
  if (needsInitialization) return false;
  if (outAddrType != NULL) *outAddrType = -1;
  if (outHasIrk != NULL) *outHasIrk = 0;
  g_switchTargetIrkValid = false;
  if (Bluefruit.Advertising.isRunning()) {
    // Replaced by the window. Fail-closed, enforced (4.8.7 review — the
    // comment used to CLAIM this without checking the rc): a failed stop
    // leaves a live adv set this arm cannot reconfigure; refuse the arm so
    // the caller falls back to open advertising instead of half-arming.
    if (!Bluefruit.Advertising.stop()) return false;
  }
  ble_gap_id_key_t id;
  if (!lsHostBondReadIdentity(targetMac12, &id)) return false;
  // The stored type can contradict the bytes (bench 2026-11-07: the Mac's
  // bond stored PUBLIC for bytes carrying the 11 static-random marker). The
  // coerced identity is BOTH the directed advertise's p_peer_addr and the
  // wire INFO line's target= text — coerce once.
  lsHostCoerceAddrType(&id.id_addr_info);
  g_switchWlAddr = id.id_addr_info;
  if (outAddrType != NULL) *outAddrType = (int)g_switchWlAddr.addr_type;
  snprintf(g_switchWlText, sizeof(g_switchWlText), "%02X:%02X:%02X:%02X:%02X:%02X/%d",
           g_switchWlAddr.addr[0], g_switchWlAddr.addr[1], g_switchWlAddr.addr[2],
           g_switchWlAddr.addr[3], g_switchWlAddr.addr[4], g_switchWlAddr.addr[5],
           (int)g_switchWlAddr.addr_type);
  // IRK CAPTURE (4.9.0): the target's IRK, when the bond carries one, feeds
  // the tick's RPA resolver — a privacy host (iOS default) reconnects with a
  // rotating resolvable address whose bytes never equal the stored identity,
  // so a bytes-only match would bounce the legitimate target for the whole
  // window. irk=0 on the INFO line means exactly "the bond carries no IRK"
  // (4.8.7 review): the tick then falls back to the bond-decision grace for
  // RPA peers (bond completion flips getPeerAddr to the stored identity).
  // The IRK is deliberately NOT armed into a SoftDevice identity list — a
  // p_peer_addr in the identity list makes the SD substitute an RPA as the
  // directed target address instead of the raw identity, breaking the
  // packet's target match for hosts that do not resolve RPAs during
  // reconnect (bench 4.8.9).
  static const uint8_t zeroIrk[BLE_GAP_SEC_KEY_LEN] = { 0 };
  const bool hasIrk = (memcmp(id.id_info.irk, zeroIrk, BLE_GAP_SEC_KEY_LEN) != 0);
  if (hasIrk) {
    memcpy(g_switchTargetIrk, id.id_info.irk, BLE_GAP_SEC_KEY_LEN);
    g_switchTargetIrkValid = true;
  }
  if (outHasIrk != NULL) *outHasIrk = hasIrk ? 1 : 0;
  return true;
}

// DISARM (best-effort restore of the stock SD state — empty whitelist, empty
// identity list). Called when the window never starts and when it closes.
void lsHostDisarmSwitchWindow(void) {
  (void)sd_ble_gap_whitelist_set(NULL, 0);
  (void)sd_ble_gap_device_identities_set(NULL, NULL, 0);
}

// BEGIN (call AFTER the disconnect): start the DIRECTED window advertise at
// the target identity (1 s per arm — high-duty directed self-terminates by
// spec; the tick re-arms while the window's bound holds). TWO races to close:
// (1) the stock restartOnDisconnect handler runs on the BLE event thread and
//     may have ALREADY restarted open advertising (FP_ANY) by the time we get
//     here — stop it first, deterministically (and the caller suspended the
//     auto-restart so the disconnect→begin gap has NOTHING on air);
// (2) the SD may still be TEARING DOWN the just-disconnected connection —
//     sd_ble_gap_adv_start refuses until the disconnect completes (bench
//     2026-11-07: begin failed on BOTH attempts, the racer won, and the new
//     fallback line is what named the cause). Wait, bounded, for
//     Bluefruit.connected() to reach 0 before starting — a few ms on the loop
//     task, ordered after the 2 s dwell so nothing user-facing stalls;
// (3) the same handler can fire between start()'s configure and its
//     adv_start (its !_runnning check is still false mid-call) — its
//     start() then owns the set and our adv_start refuses
//     (NRF_ERROR_INVALID_STATE). One stop + retry closes it.
// Returns NRF_SUCCESS (0) on success, else the last SoftDevice error code
// (the executor's INFO line names it). On final failure: restore the stock
// auto-restart + disarm + open advertising (the pre-contingency behavior —
// never a wedge).
uint32_t lsHostBeginSwitchWindow(void) {
  if (needsInitialization) return 0xFFFFFFFFUL;
  // (2) wait out the disconnect teardown — bounded so a wedged SD cannot hang
  // the loop task (the millis backstop in the tick already bounds the window).
  // The lib's connected() can read 0 BEFORE the SoftDevice has freed the
  // peripheral role (bench 2026-11-07: adv_start then refused 0x30012 =
  // NRF_ERROR_CONN_COUNT — "connectable advertiser cannot be started" while
  // the old connection's role teardown was still in flight), so the retry
  // loop below ALSO retries CONN_COUNT specifically: that one error heals in
  // milliseconds on its own. Non-CONN_COUNT failures do not heal by waiting
  // and break out immediately (first attempt of the old shape).
  for (int wait = 0; wait < 100 && Bluefruit.connected() != 0; wait++) delay(2);
  // OPEN-ADVERTISING WINDOW, RESTORED (4.9.3 — the directed trial's final
  // verdict, on the wire): the 4.8.17 directed trial FAILED with both hosts —
  // Windows silently ignores directed packets (bench 2026-11-07), and the
  // iPhone's connect attempts cannot establish through a directed set either
  // (bench 2026-11-09: "connection attempts fail" while the only live arm is
  // the ~1 s one; the Mac's preps only ever worked inside that first second).
  // The 4.8.14-16 OPEN door — every host's reconnect has always ridden open
  // advertising — with the 4.8.12 bouncer as the keeper and 4.9.0's RPA
  // resolution as the target decision is the combination where a privacy
  // host's connect can actually REACH the decision logic. CONN_COUNT teardown
  // retry kept (the live-link teardown class).
  uint32_t rc = 0;
  int waitedMs = 0;
  g_switchWindowFailConn = Bluefruit.connected();
  g_switchWindowFailWaitedMs = 0;
  for (int attempt = 0; attempt < 60; attempt++) {
    // Already live (the stock auto-restart handler can race us here) — the
    // window simply owns the running open advertise.
    if (Bluefruit.Advertising.isRunning()) { rc = NRF_SUCCESS; break; }
    if (Bluefruit.Advertising.start(0) || Bluefruit.Advertising.isRunning()) { rc = NRF_SUCCESS; break; }
    rc = 0xFFFFFFFFUL;  // start() refused
    if (Bluefruit.connected() != 0) {  // role teardown in flight — CONN_COUNT class
      delay(20);
      waitedMs += 20;
      continue;
    }
    break;  // refused for a reason waiting cannot fix
  }
  if (rc == NRF_SUCCESS) {
    g_switchWindowOpen = true;
    g_switchWindowStartMs = millis();
    g_switchWindowFailConn = -1;
    g_switchWindowFailWaitedMs = 0;
    g_switchBounceCount = 0;
    g_switchBouncedActive = false;
    g_switchBouncedHdl = 0xFFFF;
    g_switchBounceCounted = false;
    g_switchBounceDispatchedMs = 0;  // 4.9.4: horizon anchor clears with its connection tracking
    g_switchAdvRestartPending = false;  // the window owns the radio now
    return NRF_SUCCESS;
  }
  g_switchWindowFailConn = Bluefruit.connected();
  g_switchWindowFailWaitedMs = waitedMs;
  // Final failure: restore the stock radio state — auto-restart FIRST (so
  // lsHostEnsureAdvertising's open advertising survives future disconnects),
  // then disarm, then open advertising (the pre-contingency behavior).
  lsHostRestoreAutoRestart();
  lsHostDisarmSwitchWindow();
  lsHostEnsureAdvertising();
  return rc;
}

// Cancel an open window WITHOUT owning the final radio state (4.8.7 review —
// the re-entry hole: a second lsHostExecuteSwitch while window 1 was open
// armed over it but never cleared its state, so a later failure left a stale
// flag whose backstop stop/restarted live advertising and could report a
// phantom "target won"). Clears the state, stops the window advertise,
// disarms both SD lists and restores the stock auto-restart; what runs next
// (arm/begin/fallback) owns the radio from here.
void lsHostCancelSwitchWindow(void) {
  if (!g_switchWindowOpen) return;
  g_switchWindowOpen = false;
  g_switchWindowStartMs = 0;
  g_switchBounceCount = 0;
  g_switchBouncedActive = false;
  g_switchBouncedHdl = 0xFFFF;
  g_switchBounceCounted = false;
  g_switchBounceDispatchedMs = 0;  // 4.9.4: horizon anchor clears with its connection tracking
  g_switchTargetIrkValid = false;
  (void)Bluefruit.Advertising.stop();
  lsHostRestoreAutoRestart();
  lsHostDisarmSwitchWindow();
  // (round-3 review) a cancel can land in the stale-run/event-pending gap;
  // arm the deferred settle so a forget-path cancel cannot leave the radio
  // silently off. (A following arm/begin owns the radio and clears the flag.)
  // 4.9.1: also end the HID watchdog's grace tracking here — a link that
  // connects after the window must not inherit a grace frozen mid-window
  // (the freshly switched-to target used to be disconnected on its first
  // watchdog pass, mid-CCCD-restore).
  if (!Bluefruit.connected()) {
    g_switchAdvRestartPending = true;
    g_switchAdvRestartPendingMs = millis();
  }
  lsHidWatchdogResetGrace();
}

// CLOSE: stop the window advertise, disarm the SD lists, restore the stock
// auto-restart and resume stock open advertising when no host is connected
// (the target IS connected on the success path — starting advertising then
// would be wrong; restartOnDisconnect owns the post-disconnect advertise as
// always).
void lsHostCloseSwitchWindow(void) {
  if (!g_switchWindowOpen) return;
  g_switchWindowOpen = false;
  g_switchWindowStartMs = 0;
  g_switchBounceCount = 0;
  g_switchBouncedActive = false;
  g_switchBouncedHdl = 0xFFFF;
  g_switchBounceCounted = false;
  g_switchBounceDispatchedMs = 0;  // 4.9.4: horizon anchor clears with its connection tracking
  g_switchTargetIrkValid = false;
  (void)Bluefruit.Advertising.stop();  // window set; harmless if the SD already terminated it (rc ignored on purpose: the deferred-restart flag below covers a stale-run refusal)
  lsHostRestoreAutoRestart();
  lsHostDisarmSwitchWindow();
  if (!Bluefruit.connected()) lsHostEnsureAdvertising();
  // Also arm the deferred restart: the one-shot ensure above can be fooled
  // by the stale _runnning flag in the event-pending gap (round-3 review).
  // 4.9.1: the pending carries its arm time (the service must not trust a
  // young isRunning reading), and the HID watchdog's grace tracking ends
  // with the window (see the cancel path).
  if (!Bluefruit.connected()) {
    g_switchAdvRestartPending = true;
    g_switchAdvRestartPendingMs = millis();
  }
  lsHidWatchdogResetGrace();
}

// (4.9.0 round-2 review) the window's millis bound must fire on EVERY path —
// including the non-target early returns (an undecidable-RPA grace cycle, a
// refused-disconnect teardown wait, a stale-handle re-decide): those used to
// return before the tail's bound check, so a lingering non-target could keep
// the window (and the lockout) alive past its documented 60 s worst case —
// up to the bounce cap's ~30 min. Call before every non-target `return 0`:
// true = the bound elapsed, the window was closed, the caller returns -1.
// (The g_switchWindowStartMs == 0 sentinel mirrors begin: a window opened in
// the boot millisecond would evade its own bound — unreachable in practice
// since begin runs long after setup, noted per the round-3 review.)
bool lsHostSwitchWindowBoundExpired(void) {
  if (!g_switchWindowOpen) return false;
  if (g_switchWindowStartMs == 0) return false;
  if (millis() - g_switchWindowStartMs < LSHOST_SWITCH_WINDOW_TIMEOUT_MS) return false;
  lsHostCloseSwitchWindow();
  return true;
}

// Per-tick during a switch window (polled from btFeedbackLoop at 1 s):
// returns 1 = the ARMED TARGET connected (window closed), 0 = still waiting,
// -1 = millis bound elapsed or re-arm failed (window closed, open advertising
//      resumed),
// -2 = a NON-TARGET host is connected (window closed; named on the wire —
//      possible if a host's stack responds to a directed packet not addressed
//      to it, or in a race the directed mechanism did not cover).
// (4.8.8) the directed arm self-terminates ~1 s (spec/SD limit) — this tick
// RE-ARMS it while the bound holds; between arms NOTHING is on air (the
// just-disconnected racer stays locked out — the guarantee is the directed
// addressing, not wall-to-wall advertising). A failed re-arm closes the
// window (open fallback, never a stuck quiet radio).
// Identity check (4.8.7 review — the false "target won"): the old shape
// reported success for ANY connection. The armed target is g_switchWlAddr;
// a connected peer's identity must match its bytes before the switch may
// read as done. On the directed mechanism a NON-target connect should be
// impossible, so a mismatch here is a named anomaly, never a success.
// (4.9.0) the match is RPA-aware — see lsHostRpaResolvesIrk below.

// (4.9.0) the RPA byte test (lsHostAddrIsRpa) and the hash comparator
// (lsHostRpaHashMatches) live in LSHostSwitch.h — pure, host-tested. Only
// the AES-backed resolver stays here, where the SoftDevice call exists.

// Resolve a peer's resolvable address against the armed target's IRK —
// Bluetooth Core 5.3 Vol 3 Part H 2.2.2: ah(IRK, prand) = e(IRK, r') mod
// 2^24, with r' = prand in the least-significant 24 bits and zero padding
// above (Zephyr/Nordic rpa.c ah(): memcpy(res, r, 3) into an otherwise-zero
// 16-byte block, little-endian AES, output truncated to its least
// significant 24 bytes-as-array bytes). Byte order, both ends: the RPA is
// prand = addr[3..5] (addr[5] carries the 01 type bits) and hash =
// addr[0..2]; a match means this RPA belongs to the identity holding the
// IRK — the privacy target, admitted pre-bond. sd_ecb_block_encrypt is a
// big-endian (MSB-first) AES core, so the little-endian block and key are
// byte-reversed in and the ciphertext byte-reversed out (the exact
// definition of bt_encrypt_le).
// 4.9.1: the result is TRI-STATE. The old shape folded an SD ECB refusal
// into "no match" — but a refusal is INDEFINITE (the ECB is a shared AES
// resource; the connect/encryption storm the bouncer itself generates can
// refuse or serialize a call at exactly the moment the target connects),
// while a computed hash mismatch is DEFINITIVE. The tick grants the
// bond-decision grace to an undecided peer instead of bouncing it.
#define LSHOST_RPA_EXCLUDED 0   // resolved, does NOT match the IRK — definitive not-the-target
#define LSHOST_RPA_MATCH 1      // resolved and matches — the privacy target
#define LSHOST_RPA_UNDECIDED (-1)  // the SD refused the ECB call — identity unknown
int lsHostRpaResolvesIrk(const uint8_t irk[16], const uint8_t peerAddr[6]) {
  if (!lsHostAddrIsRpa(peerAddr)) return LSHOST_RPA_EXCLUDED;
  uint8_t clearLe[16] = { 0 };
  clearLe[0] = peerAddr[3];
  clearLe[1] = peerAddr[4];
  clearLe[2] = peerAddr[5];
  nrf_ecb_hal_data_t ecb;
  memset(&ecb, 0, sizeof(ecb));
  for (int i = 0; i < 16; i++) {
    ecb.key[i] = irk[15 - i];          // big-endian AES key
    ecb.cleartext[i] = clearLe[15 - i];  // big-endian block
  }
  if (sd_ecb_block_encrypt(&ecb) != NRF_SUCCESS) return LSHOST_RPA_UNDECIDED;
  return (lsHostRpaHashMatches(peerAddr, ecb.ciphertext) != 0) ? LSHOST_RPA_MATCH : LSHOST_RPA_EXCLUDED;
}
int lsHostSwitchWindowTick(void) {
  if (!g_switchWindowOpen) return 0;

  if (Bluefruit.connected()) {
    uint16_t hdl[LSHOST_TICK_MAX_CONNECTIONS];
    const uint8_t n = Bluefruit.getConnectedHandles(hdl, LSHOST_TICK_MAX_CONNECTIONS);
    bool targetConnected = false;
    uint16_t nonTargetHdl = 0xFFFF;
    bool nonTargetDecidable = true;  // see the three-tier comment below
    for (uint8_t i = 0; i < n; i++) {
      BLEConnection* conn = Bluefruit.Connection(hdl[i]);
      if (conn == NULL) continue;
      ble_gap_addr_t peer = conn->getPeerAddr();
      // IDENTITY MATCH (4.9.0, three tiers):
      // (1) the stored identity bytes (public/static-address hosts — the
      //     bench Mac/Windows class);
      // (2) RPA resolution against the armed IRK — an iOS-style privacy
      //     target reconnects with a rotating resolvable address whose bytes
      //     never equal the stored identity, so the bytes-only match bounced
      //     the legitimate target for the whole window (each retry repeated);
      //     resolution admits it pre-bond;
      // (3) an RPA peer the armed bond cannot resolve (irk=0) is UNDECIDED
      //     at first sight — the bounce-decision grace below gives its
      //     bonding time to complete, which flips getPeerAddr to the stored
      //     identity and lets tier (1) admit it.
      const bool isRpaPeer = lsHostAddrIsRpa(peer.addr);
      bool match = (memcmp(peer.addr, g_switchWlAddr.addr, 6) == 0);
      bool rpaUndecided = false;
      if (!match && isRpaPeer && g_switchTargetIrkValid) {
        const int rpaState = lsHostRpaResolvesIrk(g_switchTargetIrk, peer.addr);
        match = (rpaState == LSHOST_RPA_MATCH);
        // 4.9.1: an ECB REFUSAL is not an exclusion — the AES engine is
        // shared, and the connect/encryption storm the bouncer itself
        // generates is exactly when a call can be refused. An undecided
        // peer gets the bond-decision grace like a no-IRK peer (the
        // next tick resolves it; the window bound still ends the wait).
        rpaUndecided = (rpaState == LSHOST_RPA_UNDECIDED);
      }
      if (match) {
        targetConnected = true;
      } else {
        nonTargetHdl = hdl[i];
        // A non-target is DECIDABLE unless its identity cannot be judged at
        // all: an RPA the armed bond carries no IRK for, or an RPA whose
        // resolution the SD just refused (4.9.1). Decidable INCLUDES an RPA
        // the armed IRK computationally EXCLUDED (hash mismatch = definitive
        // not-the-target) — those bounce immediately, preserving the
        // bench-proven millisecond lockout for the public/static racer class
        // (a grace on every first sight would hand the racer 3 s of link per
        // cycle and reopen the wrong-host race).
        nonTargetDecidable = !((isRpaPeer && !g_switchTargetIrkValid) || rpaUndecided);
      }
    }
    if (targetConnected) {
      lsHostCloseSwitchWindow();
      return 1;
    }
    // THE BOUNCER (4.8.12 — bench 4.8.11's verdict): S140 does NOT enforce
    // the directed target field NOR the whitelist filter on a directed set —
    // macOS's connect-by-address retry walked straight into every windowed
    // attempt. The lockout cannot come from the controller; it comes from US:
    // a non-target connect is dropped and the window KEEPS WAITING for the
    // target. The just-disconnected host's reconnect engine cycles (connect →
    // dropped → retry, macOS at its own backoff cadence) while the target's
    // connect — whenever it comes — is the one that holds. This is the same
    // disconnect primitive the whole switch has used all along; it needs no
    // SD filtering. The bounce count is bounded (a pathological storm closes
    // the window with the named -2 report instead of spinning forever).
#define LSHOST_BOUNCE_MAX 600
    if (nonTargetHdl != 0xFFFF) {
      const bool knownHandle = (g_switchBouncedActive && nonTargetHdl == g_switchBouncedHdl);
      if (!knownHandle) {
        // FIRST OBSERVATION of this handle (4.9.0): record it. A decidable
        // non-target (public/static bytes, or an RPA the armed IRK already
        // excluded) proceeds to the bounce in this same pass — the
        // bench-proven millisecond lockout is untouched. An UNDECIDABLE RPA
        // peer (no IRK in the bond) gets the bond-decision grace below:
        // self-advancing passes before the grace ends are this connection
        // still establishing, and bond completion flips getPeerAddr to the
        // stored identity so the match branch above can admit the target.
        g_switchBouncedActive = true;
        g_switchBouncedHdl = nonTargetHdl;
        g_switchBouncedConnectMs = millis();
        g_switchBounceCounted = false;
        g_switchBounceDispatchedMs = 0;  // 4.9.5: the anchor clears with its tracking here too — the prior connection's stamp must never outlive it (it is unreadable while Counted is false, but the invariant should not depend on that)
      }
      if (g_switchBounceCounted) {
        // Already bounced: the same handle still present is this connection
        // tearing down (the SD needs ~100-200 ms; the tick runs at
        // milliseconds) — skip quietly, no count, no log, no disconnect spam
        // (4.8.16). The tracking clears when the link goes empty (below), so
        // a reconnecting peer counts anew even if the SD reuses the handle
        // number. 4.9.0: a handle that lingers past the teardown horizon is
        // NOT trusted as "the same teardown" — a refused disconnect must
        // never surrender the lockout quietly forever (the connect would hold
        // the link for the window's remainder). Clear the tracking; the next
        // pass re-decides it as a fresh connection, still bounded by the
        // bounce cap.
        // 4.9.4: the horizon is measured from the BOUNCE, not from the
        // connect. It used to key on g_switchBouncedConnectMs (set once at
        // first observation), so a NON-target that connected, sat through
        // the undecidable-RPA grace, and was then bounced at ~3 s had only
        // ~9 s of horizon left — and ANY non-target whose teardown ran past
        // 12 s from connect (a slow/loaded peer, or a bounce that landed
        // late in the grace) tripped the re-decide while its ORIGINAL
        // connection was still tearing down. That re-decide is meant for a
        // REFUSED disconnect only; on a SUCCESSFUL bounce it re-counts and
        // re-disconnects the same dying connection, inflating the bounce
        // counter toward LSHOST_BOUNCE_MAX (600) — whose only exit is the
        // MISMATCH close, i.e. the honest NOT-confirmed report for a switch
        // the bouncer was still legitimately holding. Anchor: the dispatch
        // stamp (4.9.5 correction — the stamp is taken at the decision point,
        // BEFORE the disconnect attempt, so a REFUSED disconnect is also
        // dispatch-anchored; the connect-stamp fallback inside
        // lsHostBounceHorizonAnchor is only the millis()==0 wrap-tick guard).
        const unsigned long bounceHorizonStartMs = lsHostBounceHorizonAnchor(g_switchBounceDispatchedMs,
                                                                            g_switchBouncedConnectMs);
        if (lsHostBounceTeardownStillRunning(millis() - bounceHorizonStartMs,
                                             LSHOST_BOUNCE_DECISION_GRACE_MS * 4UL)) {
          // teardown in progress — wait it out quietly (the bound still applies)
          return lsHostSwitchWindowBoundExpired() ? -1 : 0;
        }
        g_switchBouncedActive = false;
        // stale tracked handle — re-decide it as a new connection next pass
        return lsHostSwitchWindowBoundExpired() ? -1 : 0;
      }
      if (!nonTargetDecidable
          && millis() - g_switchBouncedConnectMs < LSHOST_BOUNCE_DECISION_GRACE_MS) {
        // undecidable RPA peer — hold through the bond-decision grace (the
        // bound still applies; a lingered non-target must not extend it)
        return lsHostSwitchWindowBoundExpired() ? -1 : 0;
      }
      // Decision point (a decidable non-target arriving here on its first
      // pass, or an undecidable one whose grace elapsed without the identity
      // surfacing). ONE BOUNCE PER CONNECTION (4.8.16): count/log/dispatch
      // exactly once, then the teardown-skip above owns its same-handle
      // passes.
      g_switchBounceCounted = true;
      g_switchBounceDispatchedMs = millis();  // 4.9.4: the teardown horizon starts at the bounce, not at the connect
      if (g_switchBounceCount < LSHOST_BOUNCE_MAX) {
        g_switchBounceCount++;
        if (g_switchBounceCount <= 3 || (g_switchBounceCount % 20) == 0) {
          BLEConnection* conn = Bluefruit.Connection(nonTargetHdl);
          char peer13[LSHOST_MAC_CHARS + 1] = "????????????";
          if (conn != NULL) {
            ble_gap_addr_t peer = conn->getPeerAddr();
            sprintf(peer13, "%02X%02X%02X%02X%02X%02X",
                    peer.addr[0], peer.addr[1], peer.addr[2],
                    peer.addr[3], peer.addr[4], peer.addr[5]);
          }
          serialTxEnqueueLine(String("INFO: BT window bouncing non-target ") + peer13
                              + " (#" + g_switchBounceCount + ") — still waiting for the target");
        }
        // The disconnect's rc is DELIBERATELY ignored (bench 4.8.14: it can
        // race the peer's own mid-teardown retry); the per-connection tracking
        // above is what keeps the loop honest and bounded.
        (void)Bluefruit.disconnect(nonTargetHdl);
        // keep waiting for the target (the bound still applies)
        return lsHostSwitchWindowBoundExpired() ? -1 : 0;
      }
      // Cap reached on a genuinely persistent non-target: close with the
      // honest NOT-confirmed report (the pre-bouncer behavior).
      BLEConnection* conn = Bluefruit.Connection(nonTargetHdl);
      char peer13[LSHOST_MAC_CHARS + 1] = "????????????";
      if (conn != NULL) {
        ble_gap_addr_t peer = conn->getPeerAddr();
        sprintf(peer13, "%02X%02X%02X%02X%02X%02X",
                peer.addr[0], peer.addr[1], peer.addr[2],
                peer.addr[3], peer.addr[4], peer.addr[5]);
      }
      char target13[LSHOST_MAC_CHARS + 1];
      sprintf(target13, "%02X%02X%02X%02X%02X%02X",
              g_switchWlAddr.addr[0], g_switchWlAddr.addr[1], g_switchWlAddr.addr[2],
              g_switchWlAddr.addr[3], g_switchWlAddr.addr[4], g_switchWlAddr.addr[5]);
      serialTxEnqueueLine(String("INFO: BT window MISMATCH (connected ") + peer13
                          + " != target " + target13 + ") — switch NOT confirmed");
      lsHostCloseSwitchWindow();
      return -2;
    }
  } else {
    g_switchBouncedActive = false;  // link empty — the next connect is a NEW connection
    g_switchBounceCounted = false;
    g_switchBounceDispatchedMs = 0;  // 4.9.4: horizon anchor clears with its connection tracking
  }

  if (g_switchWindowStartMs != 0
      && millis() - g_switchWindowStartMs >= LSHOST_SWITCH_WINDOW_TIMEOUT_MS) {
    lsHostCloseSwitchWindow();
    return -1;  // timed out — open advertising resumed by the close
  }
  // Keep-open (4.9.3): the window IS stock open advertising (start(0) runs
  // forever) — this re-ensures it if anything stopped it mid-window. The
  // LIVE case is the bouncer's own bounce: a non-target connect stops the
  // advertising set (the connection takes the radio role), the disconnect
  // frees it, and the stock DISCONNECTED auto-restart is SUSPENDED for the
  // whole window lifecycle (lsHostSuspendAutoRestart) — without this branch
  // the radio sits dark for the window's remainder and the target can never
  // connect (4.9.4 review: the branch called lsHostEnsureAdvertising(), whose
  // first guard REFUSES while a window is open — a 4.9.0 directed-era rule
  // whose rationale, "do not fill the directed set's inter-arm gap with open
  // advertising", no longer applies to the open-window shape and which made
  // the keep-open a silent no-op. Start the set here directly: while this
  // window is open, open advertising IS the window).
  if (!Bluefruit.Advertising.isRunning() && !Bluefruit.connected()) {
    (void)Bluefruit.Advertising.start(0);  // rc immaterial — the tick retries every pass
  }
  return 0;  // still waiting
}

// Per-loop-pass window service (4.8.8; 4.8.14 open-advertising shape): runs
// the tick at millisecond granularity so the bouncer drops a non-target
// connect as fast as possible while a window holds; no-op otherwise. Called
// from the main loop; the btFeedbackLoop tick call remains as the 1 s
// backstop (idempotent).
void lsHostServiceSwitchWindow(void) {
  if (g_switchWindowOpen) {
    (void)lsHostSwitchWindowTick();
    return;
  }
  // A window ended while no host was connected: make sure open advertising
  // actually comes back up AFTER the SD's pending termination event has
  // landed (see g_switchAdvRestartPending).
  if (g_switchAdvRestartPending) {
    if (Bluefruit.connected()) {
      g_switchAdvRestartPending = false;  // a host owns the link; auto-restart owns the future
    } else if (Bluefruit.Advertising.isRunning()) {
      // 4.9.1: this branch may be reading the STALE flag (the directed set
      // terminated, ADV_SET_TERMINATED not yet dispatched) — the exact gap
      // the pending flag exists for. A young reading proves nothing; only
      // trust "running" once it has persisted past the event-gap bound, by
      // which time a stale flag has long resolved to false and a genuinely
      // on-air open advertise stays.
      if (millis() - g_switchAdvRestartPendingMs > LSHOST_ADV_RESTART_STALE_RUN_MS) {
        g_switchAdvRestartPending = false;
      }
    } else if (lsHostEnsureAdvertising()) {
      g_switchAdvRestartPending = false;
    }
  }
}

// Is a switch window open? (4.9.0 — the BT,3 pair-new gate: the window's
// bouncer owns the radio and would drop the peer pair-new exists to admit.)
bool lsHostSwitchWindowOpen(void) {
  return g_switchWindowOpen;
}



/*****************************
   HID-SUBSCRIPTION WATCHDOG (4.8.18 — the bench 2026-11-07 "connected but
   deaf" class)
   *****************************
   After heavy connect/drop churn a host's HID stack can hold a link whose
   notification subscription never re-enables: the device shows CONNECTED,
   live view is healthy, and no cursor moves (the SoftDevice refuses every
   report with NRF_ERROR_INVALID_STATE while the CCCD is off — quoted from the
   S140 hvx contract). Tonight's fixes were all host-side (Bluetooth toggle,
   reboot, remove+re-add).

   The watchdog makes that state self-correcting: while a BONDED peer holds a
   link whose mouse-input CCCD is not enabled, after a 5 s grace (pairing/
   encryption/CCCD restore take a moment), the link is DISCONNECTED — the
   host's own retry then re-establishes and re-subscribes cleanly (the fix
   that worked all night, driven by us). Bounded: 3 cycles max per LINK
   EPISODE (4.9.1 re-key: the budget resets when the link ends or a new
   link is observed — the old per-session cap let host A's burned budget
   silently disable self-correction and diagnostics for every later host's
   fresh link; a persistent non-subscriber on ONE link is still never
   cycled forever); past the bound the link is left to the host with one
   named INFO line, and the same is true of the disconnect-refusal cap
   (4.9.1: the refusal cap now actually stops the attempts — the old shape
   logged the cap and kept retrying every pass). A subscribed link
   resets everything.

   Guards: BONDED peers only (a BLE-DFU session is conventionally unbonded
   and is never cycled; USB DFU is the documented update path regardless);
   suppressed while a switch window is open (the window's own tick owns the
   radio then). The report index is BLEHidAdafruit's REPORT_ID_MOUSE = 3
   (private enum, read via the bounded lib accessor inputNotifyEnabled()).
*/

#define LSHOST_HID_WD_GRACE_MS    5000
#define LSHOST_HID_WD_MAX_CYCLES  3
#define LSHOST_HID_WD_MAX_REFUSED 5
#define HID_REPORT_ID_MOUSE       3

static unsigned long g_hidWdConnectMs = 0;  // 0 = not tracking an unsubscribed link
static uint16_t g_hidWdHdl = 0xFFFF;        // the link the grace timer belongs to (4.9.0)
static int g_hidWdCycles = 0;
static int g_hidWdRefused = 0;
static bool g_hidWdRefusedCap = false;      // 4.9.1: 5 consecutive refusals — hands the link to the host FOR REAL (the old shape logged the cap and kept retrying every pass)
static bool g_hidWdCappedLogged = false;

// 4.9.1: the grace anchor ends with the link episode. Called from the
// switch-window close/cancel paths (the window freezes the watchdog for its
// whole duration; the link that connects afterward must start a fresh
// grace) and from the watchdog's own new-link/link-less observations.
static void lsHidWatchdogResetGrace(void) {
  g_hidWdConnectMs = 0;
  g_hidWdHdl = 0xFFFF;
}

void lsHidWatchdogLoop(void) {
  if (needsInitialization) return;
  if (g_comMode != CONF_COM_MODE_BLE) return;
  if (g_switchWindowOpen) return;  // the window's tick owns the radio

  if (!Bluefruit.connected()) {
    lsHidWatchdogResetGrace();
    // 4.9.1: the cycle budget is now per LINK EPISODE, not per session —
    // the link-less observation means whatever link had the budget is gone.
    // (The old per-session cap meant host A's burned budget silently
    // disabled self-correction AND diagnostics for every later host's
    // fresh link, including a legitimately slow-to-subscribe one.)
    g_hidWdCycles = 0;
    g_hidWdRefused = 0;
    g_hidWdRefusedCap = false;
    g_hidWdCappedLogged = false;
    return;
  }

  uint16_t hdl[1];
  if (Bluefruit.getConnectedHandles(hdl, 1) == 0) return;
  BLEConnection* conn = Bluefruit.Connection(hdl[0]);
  if (conn == NULL || !conn->bonded()) { lsHidWatchdogResetGrace(); return; }  // DFU guard

  if (blehid.inputNotifyEnabled(HID_REPORT_ID_MOUSE)) {
    // Subscribed — healthy. Reset everything.
    lsHidWatchdogResetGrace();
    g_hidWdCycles = 0;
    g_hidWdRefused = 0;
    g_hidWdRefusedCap = false;
    g_hidWdCappedLogged = false;
    return;
  }

  // The grace belongs to the link it started on (4.9.0, re-keyed 4.9.1): a
  // new handle observation starts a fresh grace AND a fresh cycle budget —
  // the SoftDevice reuses handle numbers, so a same-numbered reconnect must
  // never inherit the previous link's elapsed timer (the 4.9.0 handle-keyed
  // guard let exactly that through; the S140 always allocates the lowest
  // free handle, making reuse the COMMON case).
  if (g_hidWdHdl != hdl[0]) {
    g_hidWdHdl = hdl[0];
    g_hidWdConnectMs = millis();
    g_hidWdCycles = 0;
    g_hidWdRefused = 0;
    g_hidWdRefusedCap = false;
    g_hidWdCappedLogged = false;
    return;
  }
  if (g_hidWdConnectMs == 0) { g_hidWdConnectMs = millis(); return; }  // grace begins
  if (millis() - g_hidWdConnectMs < LSHOST_HID_WD_GRACE_MS) return;

  // 4.9.1: the refusal cap STOPS the intervention — the old shape logged
  // "leaving the link to the host" and then kept attempting a disconnect
  // every pass forever (only the log was capped). A persistent non-subscriber
  // on THIS link episode is left to the host, as the header contract says.
  if (g_hidWdRefusedCap) return;

  if (g_hidWdCycles < LSHOST_HID_WD_MAX_CYCLES) {
    // A cycle is consumed only when the disconnect actually dispatched —
    // a refusal (the SD mid-teardown a race left behind, bench 4.8.15's
    // class) must not burn the session budget without cycling anything.
    // (4.9.0 round-2 precision note: the lib's disconnect() returns true
    // when the handle is already gone, so a raced teardown still consumes a
    // cycle — negligible, the counters reset on the next observed
    // subscription. What this guard really prevents is the REFUSED call
    // (NRF_ERROR_INVALID_STATE-class) burning the budget.)
    if (!Bluefruit.disconnect(hdl[0])) {
      if (++g_hidWdRefused >= LSHOST_HID_WD_MAX_REFUSED) {
        g_hidWdRefused = 0;
        g_hidWdRefusedCap = true;  // hands off until the link ends or a subscription is seen
        if (!g_hidWdCappedLogged) {
          g_hidWdCappedLogged = true;
          serialTxEnqueueLine("INFO: HID watchdog capped (disconnects refused) — leaving the link to the host");
        }
        return;
      }
      // Retry on the next pass without consuming a cycle; a short re-grace
      // keeps the retry cadence at the 1 s pass instead of the grace.
      g_hidWdConnectMs = millis() - LSHOST_HID_WD_GRACE_MS + 500UL;
      return;
    }
    g_hidWdCycles++;
    g_hidWdRefused = 0;
    g_hidWdConnectMs = 0;  // re-grace after the cycle
    serialTxEnqueueLine(String("INFO: HID watchdog cycling a subscription-less link (cycle ")
                        + g_hidWdCycles + ")");
    return;
  }
  if (!g_hidWdCappedLogged) {
    g_hidWdCappedLogged = true;
    serialTxEnqueueLine("INFO: HID watchdog capped (3 cycles without recovery) — leaving the link to the host");
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
