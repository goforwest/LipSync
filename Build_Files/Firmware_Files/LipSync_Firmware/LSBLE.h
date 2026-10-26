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

// 4.5.14: the probe's SoftDevice fault handler. An SD assert is unrecoverable
// by definition (the SD resets the chip on return), so there is nothing to do
// but let it — the core's own nrf_error_cb at CFG_DEBUG=0 is an empty body
// that hangs in while(1) yield() at CFG_DEBUG!=0; ours must never hang the
// init task if the impossible happens.
static void lipsyncBleProbeFaultHandler(uint32_t id, uint32_t pc, uint32_t info) {
  (void) id; (void) pc; (void) info;  // the SD calls NVIC_SystemReset() when we return
}

void initializeBluefruit(const char* s) {
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
  // 4.5.14 bench round 8: this probe MIRRORS begin()'s SoftDevice/BLE-level
  // sequence call-for-call — sd_softdevice_enable, the 8 sd_ble_cfg_set
  // calls with begin()'s exact stock constants (defaults: prph=1,
  // central=0, attr table 0x1000, MTU 23, stock queue sizes, service
  // changed on), then sd_ble_enable — capturing each return code. Results
  // ride the BOOT/HB lines as pstep=/pcode=/need=:
  //   pstep 0     = probe not run (non-BLE boot)
  //   pstep 1..10 = that call failed; pcode = its SoftDevice error code
  //                 (1=enable, 2=VS_UUID, 3=ROLE_COUNT, 4=SERVICE_CHANGED,
  //                  5=ATTR_TAB_SIZE, 6=CONN_CFG_GATT, 7=CONN_CFG_GAP,
  //                  8=CONN_CFG_GATTS, 9=CONN_CFG_GATTC, 10=sd_ble_enable —
  //                  need= then holds the SD's required APP RAM base, the
  //                  smoking gun when pcode=NO_MEM)
  //   pstep 11    = the whole probe PASSED — if begin() STILL fails, the
  //                 failure is in begin()'s tail (sd_ble_opt_set /
  //                 Periph.begin / name set / event semaphores / task
  //                 creation) and the next round instruments those.
  // If the probe passes, the SoftDevice is DISABLED again and begin()
  // re-runs the identical sequence that just passed — a passing probe
  // means begin() itself comes up (advertising, blue blink, pairable). A
  // failing probe means begin() dies at the same call — but now named.
  uint8_t sdAlreadyEnabled = 0;
  (void) sd_softdevice_is_enabled(&sdAlreadyEnabled);
  g_bleSdWasEnabled = (sdAlreadyEnabled != 0) ? 1 : 0;
  if (sdAlreadyEnabled) {
    (void) sd_softdevice_disable();  // MBR-serviced SVC: legal whether or not the SD is up
  }
  // 4.5.15 bench round 9: pcode=0x1001 DECODED — NRF_ERROR_SDM_INCORRECT_
  // INTERRUPT_CONFIGURATION: the SDM scans the NVIC at enable time and
  // refuses if any ENABLED interrupt sits at an SD-reserved priority
  // level (0-3). The bench's probe (pstep=1) proved our own mirrored
  // sd_softdevice_enable() fails with exactly that code on every BLE boot;
  // the vendored Wire enabled TWIM at priority 3 (inherited from upstream)
  // and our BLE boot reaches this task only AFTER the screen/sensor
  // polling has enabled it. The stock example (which works on this unit)
  // never touches Wire; 4.1.0 worked only because its Bluefruit.begin()
  // ran BEFORE any Wire.begin(). The Wire now takes priority 4 at the
  // source (see Wire_nRF52.cpp), and this sweep is the belt-and-braces
  // plus the on-wire proof: every enabled IRQ below the boundary is
  // recorded (bad1=/bad2= bit masks, IRQ n = bit n — the BOOT/HB lines
  // carry them) and demoted to 4 BEFORE the probe runs, so sd_softdevice_
  // enable sees a legal NVIC no matter what any future boot path enables.
  // demoted= counts the demotions; bad1/bad2 hold the PRE-demotion state
  // so the log names the true offender even on a boot where the demotion
  // saved it.
  {
    for (int irq = 0; irq < 48; irq++) {  // nRF52840: 48 external IRQs (0-47)
      const uint32_t bit = 1UL << (irq & 31);
      volatile uint32_t* iser = (irq < 32) ? &NVIC->ISER[0] : &NVIC->ISER[1];
      if ((*iser & bit) && NVIC_GetPriority((IRQn_Type)irq) < 4) {
        if (irq < 32) {
          g_bleIrqBad1 |= bit;
        } else {
          g_bleIrqBad2 |= bit;
        }
        NVIC_SetPriority((IRQn_Type)irq, 4);
        g_bleProbeDemoted++;
      }
    }
  }
  {
    nrf_clock_lf_cfg_t probeClock = { NRF_CLOCK_LF_SRC_XTAL, 0, 0, NRF_CLOCK_LF_ACCURACY_20_PPM };
    extern uint32_t __data_start__[];  // the app RAM base begin() passes to every cfg call (linker symbol, 0x20006000)
    g_bleProbeStep = 1;
    g_bleProbeCode = sd_softdevice_enable(&probeClock, lipsyncBleProbeFaultHandler);
    if (g_bleProbeCode == 0) {
      uint32_t ramStart = (uint32_t) __data_start__;
      ble_cfg_t probeCfg;
      memset(&probeCfg, 0, sizeof(probeCfg));
      probeCfg.common_cfg.vs_uuid_cfg.vs_uuid_count = BLE_UUID_VS_COUNT_DEFAULT;
      g_bleProbeStep = 2;
      g_bleProbeCode = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &probeCfg, ramStart);
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.gap_cfg.role_count_cfg.periph_role_count = 1;
        probeCfg.gap_cfg.role_count_cfg.central_role_count = 0;
        probeCfg.gap_cfg.role_count_cfg.central_sec_count = 0;
        g_bleProbeStep = 3;
        g_bleProbeCode = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.gatts_cfg.service_changed.service_changed = 1;
        g_bleProbeStep = 4;
        g_bleProbeCode = sd_ble_cfg_set(BLE_GATTS_CFG_SERVICE_CHANGED, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.gatts_cfg.attr_tab_size.attr_tab_size = CFG_SD_ATTR_TABLE_SIZE;
        g_bleProbeStep = 5;
        g_bleProbeCode = sd_ble_cfg_set(BLE_GATTS_CFG_ATTR_TAB_SIZE, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.conn_cfg.conn_cfg_tag = CONN_CFG_PERIPHERAL;
        probeCfg.conn_cfg.params.gatt_conn_cfg.att_mtu = BLE_GATT_ATT_MTU_DEFAULT;
        g_bleProbeStep = 6;
        g_bleProbeCode = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.conn_cfg.conn_cfg_tag = CONN_CFG_PERIPHERAL;
        probeCfg.conn_cfg.params.gap_conn_cfg.conn_count = 1;
        probeCfg.conn_cfg.params.gap_conn_cfg.event_length = BLE_GAP_EVENT_LENGTH_DEFAULT;
        g_bleProbeStep = 7;
        g_bleProbeCode = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.conn_cfg.conn_cfg_tag = CONN_CFG_PERIPHERAL;
        probeCfg.conn_cfg.params.gatts_conn_cfg.hvn_tx_queue_size = BLE_GATTS_HVN_TX_QUEUE_SIZE_DEFAULT;
        g_bleProbeStep = 8;
        g_bleProbeCode = sd_ble_cfg_set(BLE_CONN_CFG_GATTS, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        memset(&probeCfg, 0, sizeof(probeCfg));
        probeCfg.conn_cfg.conn_cfg_tag = CONN_CFG_PERIPHERAL;
        probeCfg.conn_cfg.params.gattc_conn_cfg.write_cmd_tx_queue_size = BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT;
        g_bleProbeStep = 9;
        g_bleProbeCode = sd_ble_cfg_set(BLE_CONN_CFG_GATTC, &probeCfg, ramStart);
      }
      if (g_bleProbeCode == 0) {
        g_bleProbeStep = 10;
        g_bleProbeCode = sd_ble_enable(&ramStart);
        g_bleProbeNeed = ramStart;  // on NO_MEM this is the REQUIRED app RAM base — compare against 0x20006000
      }
      if (g_bleProbeCode == 0) {
        g_bleProbeStep = 11;  // everything begin() does up through sd_ble_enable has PASSED
      }
      (void) sd_softdevice_disable();  // leave the clean slate begin() expects, whatever happened above
    }
  }
  if (!Bluefruit.begin()) {
    g_bleInitStage = 1;
    Serial.println("BOOT: BLE FAILED at SoftDevice enable");
    return;
  }
  Serial.println("BOOT: BLE stack up");
  Bluefruit.Periph.setConnInterval(9, 16);  // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(4);                  // Check bluefruit.h for supported values
  Bluefruit.setName(s);
  bledis.setManufacturer("MakersMakingChange");
  bledis.setModel("LipSync Mouse");
  if (!bledis.begin()) {
    g_bleInitStage = 2;
    Serial.println("BOOT: BLE FAILED at DIS begin");
    return;
  }
  if (!blehid.begin()) {
    g_bleInitStage = 3;
    Serial.println("BOOT: BLE FAILED at HID begin");
    return;
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
    return;
  }
  Serial.println("BOOT: BLE advertising");
  g_bleInitStage = 5;
  needsInitialization = false;  // FULL SUCCESS ONLY — every guarded call site keys on this
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
