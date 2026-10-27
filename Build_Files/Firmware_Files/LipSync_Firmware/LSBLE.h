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
#include <bluefruit.h>
#include <nrf_sdm.h>

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)

BLEDis bledis;
BLEHidAdafruit blehid;
bool needsInitialization = true;

class LSBLEMouse {

  public:
    inline LSBLEMouse(void);
    inline void begin(const char* s = "LipSync");
    inline void end(void);
    inline void move(signed char x, signed char y);
    inline void moveAll(signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
    inline void scroll(signed char wheel = 0);
    inline void pan(signed char pan = 0);
    inline void click(uint8_t b = MOUSE_LEFT);
    inline void press(uint8_t b = MOUSE_LEFT);   // press LEFT by default
    inline void release(uint8_t b = MOUSE_LEFT); // release LEFT by default
    inline bool isPressed(uint8_t b = MOUSE_LEFT); // check LEFT by default
    inline bool isConnected(void);
  protected:
    uint8_t _buttons;
    void buttons(uint8_t b);
  private:
    void mouseReport(signed char b, signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
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

// 4.5.20: reset-proof verdict persistence (defined in LipSync_Firmware.ino
// after the globals) — called at this task's entry and before every exit so
// the NEXT boot can report what the dying one recorded.
void bleVerdictPersist();

void initializeBluefruit(const char* s) {
  bleVerdictPersist();  // entry record: stage 0 = "died during begin" if this is the last write

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
  if (!Bluefruit.begin()) {
    g_bleInitStage = 1;
    Serial.println("BOOT: BLE FAILED at SoftDevice enable");
    bleVerdictPersist();
    return;
  }
  Serial.println("BOOT: BLE stack up");
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
      Serial.println("BOOT: BLE FAILED at DIS begin");
      bleVerdictPersist();
      return;
    }
  }
  {
    err_t hidRc = blehid.begin();
    if (hidRc != NRF_SUCCESS) {
      g_bleInitStage = 3;
      g_bleInitRc = hidRc;
      Serial.println("BOOT: BLE FAILED at HID begin");
      bleVerdictPersist();
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
    Serial.println("BOOT: BLE FAILED at advertising start");
    bleVerdictPersist();
    return;
  }
  Serial.println("BOOT: BLE advertising");
  g_bleInitStage = 5;
  needsInitialization = false;  // FULL SUCCESS ONLY — every guarded call site keys on this
  bleVerdictPersist();  // st=5: if the boot STILL dies after this, the fault is past the init — advertising/runtime
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

void LSBLEMouse::mouseReport(int8_t b, int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
  // 4.5.10: the BLE stack comes up on its own task now (see the 16 KB
  // init task in LipSync_Firmware.ino) — an SVC call on a never-begun or
  // still-enabling SoftDevice HardFaults, which is a whole boot class
  // worse than a dropped report. Skip until needsInitialization clears;
  // the buttons state still mutates, so the first post-init movement
  // report carries it.
  if (needsInitialization) return;
  blehid.mouseReport(b, x, y, wheel, pan);
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

void LSBLEMouse::click(uint8_t b)
{
  _buttons = b;
  mouseReport(_buttons, 0, 0, 0, 0);
  _buttons = 0;
  mouseReport(_buttons, 0, 0, 0, 0);
}

void LSBLEMouse::buttons(uint8_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
    mouseReport(_buttons, 0, 0, 0, 0);
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
