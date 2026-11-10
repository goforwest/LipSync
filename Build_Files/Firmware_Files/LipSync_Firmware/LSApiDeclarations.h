/*
* File: LSApiDeclarations.h
* Firmware: LipSync
* Developed by: MakersMakingChange
* Version: v4.3 (14 October 2026)
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

// Explicit declarations for every function the class headers and the .ino
// tabs use across tab boundaries (roadmap item 5). The Arduino build merges
// the .ino tabs and auto-generates prototypes with a shallow ctags parse —
// fragile in practice: a `static` local variable inside any .ino function
// silently drops prototype generation for the tabs that follow, failing the
// build with "was not declared in this scope" inside headers that call these
// functions (diagnosed empirically during the 4.2.0 work). Including this
// header removes that dependency entirely.
//
// Signatures MUST stay identical to the definitions — a mismatched
// declaration is a silent overload, and the call resolves to nothing at
// link time. When an LSAPI function's signature changes, change it here in
// the same commit.
//
// Working rule until the last header includes this: no `static` locals in
// .ino functions — class members or plain locals only.

#ifndef _LSAPIDECLARATIONS_H
#define _LSAPIDECLARATIONS_H

#include <WString.h>
#include "LSHostSwitch.h"  // Upstream #16: the pure host-switch row model + decisions (no Arduino types)

// ---- Serial settings API (defined in LSAPI.ino) ----
// The two-argument cores and their String redefinitions that the dispatch
// table references. responseEnabled/apiEnabled control serial print output;
// the String overloads adapt the cores to the API function-pointer shape.

// Device identity and modes
void getModelNumber(bool responseEnabled, bool apiEnabled);
void getModelNumber(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getVersionNumber(bool responseEnabled, bool apiEnabled);
void getVersionNumber(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getDeviceID(bool responseEnabled, bool apiEnabled);
void getDeviceID(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getOperatingMode(bool responseEnabled, bool apiEnabled);
void getOperatingMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setOperatingMode(bool responseEnabled, bool apiEnabled, int inputOperatingMode);
void setOperatingMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getCommunicationMode(bool responseEnabled, bool apiEnabled);
void getCommunicationMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setCommunicationMode(bool responseEnabled, bool apiEnabled, int inputCommunicationMode);
void setCommunicationMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
bool storeModePair(int operatingMode, int communicationMode);  // Atomic menu-mode persistence; caller resets once
void setModePreset(bool responseEnabled, bool apiEnabled, int inputPreset);
void setModePreset(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getSoundMode(bool responseEnabled, bool apiEnabled);
void getSoundMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSoundMode(bool responseEnabled, bool apiEnabled, int inputSoundMode);
void setSoundMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getLightMode(bool responseEnabled, bool apiEnabled);
void getLightMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setLightBrightnessLevel(bool responseEnabled, bool apiEnabled);
// Upstream #58: hard sip/puff band settings (live in /settings2.dat).
float getHardSipPressureThreshold(bool responseEnabled, bool apiEnabled);
void getHardSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setHardSipPressureThreshold(bool responseEnabled, bool apiEnabled, float inputHardSipThreshold);
void setHardSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled);
void getHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled, float inputHardPuffThreshold);
void setHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getHardSipAction(bool responseEnabled, bool apiEnabled);
void getHardSipAction(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setHardSipAction(bool responseEnabled, bool apiEnabled, int inputHardSipAction);
void setHardSipAction(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getHardPuffAction(bool responseEnabled, bool apiEnabled);
void getHardPuffAction(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setHardPuffAction(bool responseEnabled, bool apiEnabled, int inputHardPuffAction);
void setHardPuffAction(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getLightMode(bool responseEnabled, bool apiEnabled);
void getLightMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setLightMode(bool responseEnabled, bool apiEnabled, int inputLightMode);
void setLightMode(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Cursor, scroll and acceleration
int getCursorSpeed(bool responseEnabled, bool apiEnabled);
void getCursorSpeed(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setCursorSpeed(bool responseEnabled, bool apiEnabled, int inputSpeedLevel);
void setCursorSpeed(bool responseEnabled, bool apiEnabled, String optionalParameter);
void increaseCursorSpeed(bool responseEnabled, bool apiEnabled);
void decreaseCursorSpeed(bool responseEnabled, bool apiEnabled);
int getScrollLevel(bool responseEnabled, bool apiEnabled);
void getScrollLevel(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setScrollLevel(bool responseEnabled, bool apiEnabled, int inputScrollLevel);
void setScrollLevel(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getJoystickAcceleration(bool responseEnabled, bool apiEnabled);
void getJoystickAcceleration(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setJoystickAcceleration(bool responseEnabled, bool apiEnabled, int inputAccelerationLevel);
void setJoystickAcceleration(bool responseEnabled, bool apiEnabled, String optionalParameter);
// (4.9.0 review) increaseJoystickAcceleration/decreaseJoystickAcceleration
// declarations removed with their dead definitions (zero call sites).

// Joystick geometry: initialization, calibration and deadzones
void getJoystickInitialization(bool responseEnabled, bool apiEnabled);
void getJoystickInitialization(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setJoystickInitialization(bool responseEnabled, bool apiEnabled);
void setJoystickInitialization(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getJoystickCalibration(bool responseEnabled, bool apiEnabled);
void getJoystickCalibration(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setJoystickCalibration(bool responseEnabled, bool apiEnabled);
void setJoystickCalibration(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled);
void getJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled, float inputDeadzone);
void setJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled);
void getJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled, float inputUpperDeadZone);
void setJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Live readings
void getJoystickValue(bool responseEnabled, bool apiEnabled);
void getJoystickValue(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getPressureValue(bool responseEnabled, bool apiEnabled);
void getPressureValue(bool responseEnabled, bool apiEnabled, String optionalParameter);

// I2C bus health diagnostic (read-only; roadmap item 3)
void getBusHealth(bool responseEnabled, bool apiEnabled);
void getBusHealth(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Sip & puff thresholds
float getSipPressureThreshold(bool responseEnabled, bool apiEnabled);
void getSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSipPressureThreshold(bool responseEnabled, bool apiEnabled, float inputSipThreshold);
void setSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getPuffPressureThreshold(bool responseEnabled, bool apiEnabled);
void getPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setPuffPressureThreshold(bool responseEnabled, bool apiEnabled, float inputPuffThreshold);
void setPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Feedback levels
int getLightBrightnessLevel(bool responseEnabled, bool apiEnabled);
void getLightBrightnessLevel(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setLightBrightnessLevel(bool responseEnabled, bool apiEnabled, int inputLightLevel);
void setLightBrightnessLevel(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Input restrictions (roadmap item 11): four boolean settings, all defaulting
// to the unrestricted legacy behavior. The getters refresh the live gate
// globals; the gates live in the input evaluation path.
int getSapDisabled(bool responseEnabled, bool apiEnabled);
void getSapDisabled(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSapDisabled(bool responseEnabled, bool apiEnabled, int inputSapDisabled);
void setSapDisabled(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getMenuButtonsOnly(bool responseEnabled, bool apiEnabled);
void getMenuButtonsOnly(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setMenuButtonsOnly(bool responseEnabled, bool apiEnabled, int inputMenuButtonsOnly);
void setMenuButtonsOnly(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getModeLocked(bool responseEnabled, bool apiEnabled);
void getModeLocked(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setModeLocked(bool responseEnabled, bool apiEnabled, int inputModeLocked);
void setModeLocked(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getCursorHoldDuringSap(bool responseEnabled, bool apiEnabled);
void getCursorHoldDuringSap(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setCursorHoldDuringSap(bool responseEnabled, bool apiEnabled, int inputCursorHold);
void setCursorHoldDuringSap(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Input timing windows (roadmap item 15, upstream #62/#9): three boundaries
// in ms, stored 0 = the compiled default. Getters return the window in force;
// setters keep short < long < max ordering.
int getInputWindowShort(bool responseEnabled, bool apiEnabled);
void getInputWindowShort(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setInputWindowShort(bool responseEnabled, bool apiEnabled, unsigned long inputWindowMs);
void setInputWindowShort(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getInputWindowLong(bool responseEnabled, bool apiEnabled);
void getInputWindowLong(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setInputWindowLong(bool responseEnabled, bool apiEnabled, unsigned long inputWindowMs);
void setInputWindowLong(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getInputWindowMax(bool responseEnabled, bool apiEnabled);
void getInputWindowMax(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setInputWindowMax(bool responseEnabled, bool apiEnabled, unsigned long inputWindowMs);
void setInputWindowMax(bool responseEnabled, bool apiEnabled, String optionalParameter);

// External switch action remapping (roadmap item 11's #20 follow-up,
// firmware 4.5.0): one endpoint per (jack, mode) — SA/SB/SC = jacks 1/2/3
// mouse mode, GA/GB/GC = gamepad mode. The shared core getter answers and
// applies "short,long,max"; the setter takes 0 (compiled default) or
// CONF_SWITCH_ACTION_PACK_BASE + packed triple with allowlisted cells.
String getSwitchAction(bool responseEnabled, bool apiEnabled, String commandKey);
void setSwitchAction(bool responseEnabled, bool apiEnabled, String commandKey, int packedWireValue);
void getSwitchActionSA(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSwitchActionSA(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getSwitchActionSB(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSwitchActionSB(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getSwitchActionSC(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSwitchActionSC(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getSwitchActionGA(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSwitchActionGA(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getSwitchActionGB(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSwitchActionGB(bool responseEnabled, bool apiEnabled, String optionalParameter);
void getSwitchActionGC(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setSwitchActionGC(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Hub menu, diagnostics and device actions
void controlHubMenu(bool responseEnabled, bool apiEnabled, int inputMenuControl);
void controlHubMenu(bool responseEnabled, bool apiEnabled, String optionalParameter);

// ---- Multi-host Bluetooth switching (upstream #16, firmware 4.8.0+) ----
// The pure decision layer is LSHostSwitch.h (included above); these are the
// BLE-side helpers (defined in LSBLE.h — they read the vendored lib's bond
// directory) and the BT endpoint shells (defined in LSAPI.ino). The bond
// store itself is the library's /adafruit/bond_prph files — this feature adds
// NO settings file and NO settings keys (design: docs/design/16-multi-host-ble.md).
bool lsBleStackReady();  // !needsInitialization — every BT call site gates on this
uint8_t lsHostEnumerate(lsHostRow* rows, uint8_t cap);  // MAC-sorted bond rows; returns the exposed (capped) row count
bool lsHostActiveMac(char* out13);  // 12-hex bond address of the connected host, false when none/unbonded
bool lsHostDisconnectCurrent();     // disconnect the connected host, true when nothing to do
bool lsHostEnsureAdvertising();     // (re)start open advertising if the set is not running; returns whether advertising IS running (4.9.0 — callers that promise pair-new must check it)
// (4.9.0) lsHostAddrIsRpa / lsHostRpaHashMatches live in LSHostSwitch.h (the
// pure, host-tested layer); lsHostRpaResolvesIrk is internal to LSBLE.h
// (the only caller is the tick in the same translation unit).
bool lsHostArmSwitchWindow(const char* targetMac12, int* outAddrType = NULL, int* outHasIrk = NULL);   // switch window: read+coerce the target identity for the DIRECTED advertise (call BEFORE the disconnect; false = fallback to open advertising; reports the identity's addr type and IRK presence for the wire INFO line)
uint32_t lsHostBeginSwitchWindow(void); // switch window: start the DIRECTED advertise at the target identity (call AFTER the disconnect; returns NRF_SUCCESS or the SD error code; failure = disarmed + open advertising)
void lsHostDisarmSwitchWindow(void);                   // clear the SD whitelist + identity list (stock empty state; no-op storage after 4.8.8 — the directed mechanism arms neither)
void lsHostCloseSwitchWindow(void); // close an open window: stop the window advertise, disarm, restore auto-restart, resume open advertising when no host is connected
void lsHostCancelSwitchWindow(void); // cancel an open window without owning the final radio state (a second switch re-owns the radio from its own arm/begin/fallback)
int  lsHostSwitchWindowTick(void);  // poll an open window: 1 = armed target connected, 0 = waiting (directed set re-armed), -1 = bound elapsed / re-arm failed, -2 = NON-TARGET connected (switch NOT confirmed; named on the wire)
void lsHostServiceSwitchWindow(void);  // per-loop-pass window service (millisecond-granularity directed re-arms); no-op when no window is open
bool lsHostSwitchWindowOpen(void);     // 4.9.0: is a switch window open? (the BT,3 gate — the bouncer would drop the peer pair-new admits)
void lsHidWatchdogLoop(void);  // HID-subscription watchdog (btFeedbackLoop, 1 s): cycle a bonded link whose mouse CCCD never enables (bounded 3, grace 5 s); no-op unless BLE is up and no window is open
extern int g_comMode;          // 0 = None, 1 = USB, 2 = Wireless (defined in LipSync_Firmware.ino; the watchdog reads it)
bool lsHostForgetByMac(const char* mac12);  // remove one bond file (explicit InternalFS.remove + absence verification — the lib's void bond_remove_key cannot report failure)
void lsHostPushState(bool force = false);  // MANUAL,0:BT,4/BT,5 pushes with change-detect + silent boot prime (the CH,5 contract); force skips the prime for the forget paths
void lsHostRefusalLine(String commandKey, int reason, int row);  // named ERROR line for a refusal

// Switch-window outcome (4.8.7 review): the window's fate travels to the
// CALLER instead of the executor enqueueing wire lines — the serial BT,1
// handler emits its INFO lines AFTER its own SUCCESS/FAIL frame (the old
// executor enqueue order put an unsolicited INFO: ahead of the response),
// and the Hub-menu path emits no phantom "BT,1" lines at all (the screen
// already narrates the switch; unsolicited BT,1 telemetry misleads).
struct lsHostSwitchOutcome {
  bool windowOpened;
  bool armFailed;                        // true = the arm refused (begin never attempted)
  char targetMac12[LSHOST_MAC_CHARS + 1];
  int addrType;                          // armed identity's SD addr type (post-coercion), -1 when unknown
  int hasIrk;                            // 1 = the bond carries an IRK (captured for the tick's RPA resolver; no SD identity list exists — bench 4.8.8-4.8.17)
  uint32_t beginRc;                      // NRF_SUCCESS or the failing SD rc (0x01=configure, 0x02=tx_power, 0x03=adv_start | SD code)
  char wlText[24];                       // the armed directed target "AA:BB:CC:DD:EE:FF/N" (post-coercion) for the wire INFO line
};
int lsHostExecuteSwitch(int targetRow, const char* expectedMac12, lsHostSwitchOutcome* out = NULL);  // TOCTOU re-check + row-identity check (NULL expected = index contract) + disconnect + the whitelist switch window (upstream #16 B4 contingency); ALLOW or a refusal reason. With a non-NULL out, reports the window's fate; every failure path restores stock radio state.
int lsHostExecuteForget(int targetRow, const char* expectedMac12);  // TOCTOU re-check + row-identity check + (disconnect) + bond-file removal
void applyHardBandActionOverrides();  // upstream #58: HA/HB -> live SAP table (initSapActions at boot; the HA/HB setters apply live)
bool lsHostAnyConnected();               // any central connected (bonded or not) — pair-new gate
void getBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter);
void switchBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter);
void forgetBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter);
void pairNewBluetoothHost(bool responseEnabled, bool apiEnabled);
void pairNewBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Live Hub-menu state read (CH,0:0, firmware 4.5.1+): [active, pageId, row]
// from LSScreen::fillMenuState — the device-truth source the app's remote
// re-syncs its mirror against after every press (refused selects and
// frozen-window drops both ack SUCCESS, so the state line is the only
// honest answer to "where is the Hub now?").
void getHubMenuState(bool responseEnabled, bool apiEnabled);
void getHubMenuState(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getDebugMode(bool responseEnabled, bool apiEnabled);
void getDebugMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void setDebugMode(bool responseEnabled, bool apiEnabled, int inputDebugMode);
void setDebugMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void exportSettings(bool responseEnabled, bool apiEnabled);
void exportSettings(bool responseEnabled, bool apiEnabled, String optionalParameter);
void runTest(bool responseEnabled, bool apiEnabled, int inputTest);
void runTest(bool responseEnabled, bool apiEnabled, String optionalParameter);
void softReset(bool responseEnabled, bool apiEnabled);
void softReset(bool responseEnabled, bool apiEnabled, String optionalParameter);
bool resetSettings(bool responseEnabled, bool apiEnabled);
void resetSettings(bool responseEnabled, bool apiEnabled, String optionalParameter);
void doFactoryReset(bool responseEnabled, bool apiEnabled);
void doFactoryReset(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Deliberate update-mode entry: ack, draw the update-mode splash, reset into
// the stock serial-DFU bootloader (firmware-update improvements item 1)
void enterUpdateMode(bool responseEnabled, bool apiEnabled);
void enterUpdateMode(bool responseEnabled, bool apiEnabled, String optionalParameter);

// ---- Serial print helpers (defined in LSAPI.ino) ----
// Declared here because LSScreen.h's menu-state push (CH,5, the two-way Hub
// remote) prints through the same canonical formatter as every response —
// building the line by hand in the screen class would be a second wire
// format that can drift. Signatures MUST match the LSAPI.ino definitions.

void printResponseIntArray(bool responseEnabled,
                           bool apiEnabled,
                           bool responseStatus,
                           int responseNumber,
                           String responseCommand,
                           bool responseParameterEnabled,
                           String responsePrefix,
                           int responseParameterSize,
                           char responseParameterDelimiter,
                           int responseParameter[]);
// The scalar formatter, for callers outside LSAPI.ino (LSBLE.h's BT,4/BT,5
// pushes — upstream #16; same canonical-formatter rule as the array twin above).
void printResponseInt(bool responseEnabled,
                      bool apiEnabled,
                      bool responseStatus,
                      int responseNumber,
                      String responseCommand,
                      bool responseParameterEnabled,
                      int responseParameter);
// The string-param formatter (BT,0:<i> row answers carry <i>:<name>:<state>).
void printResponseString(bool responseEnabled,
                         bool apiEnabled,
                         bool responseStatus,
                         int responseNumber,
                         String responseCommand,
                         bool responseParameterEnabled,
                         String responseParameter);
void serialTxService();  // Non-blocking CDC response queue; called from loop()
bool serialTxEnqueueLine(const char* line, size_t length);
bool serialTxEnqueueLine(const String &line);

// ---- Cross-tab free functions (defined in LipSync_Firmware.ino) ----

// Device lifecycle
void softwareReset();
void releaseOutputAction();
void cancelSapOutputHold();  // 4.6.6: ends an engaged DRAG/SCROLL hold without emitting an input event (fault-interrupted holds)
void setDebugState(int inputDebugMode);
void readyToUse();
void errorScreen();

// Buzzer feedback helper called by the Hub menu (LSScreen.h)
void buzzerErrorCue();

// Input restriction machinery (roadmap item 11): boot-time load of the four
// restriction settings, and the single gate every resolved input action passes
// through (inputClass is INPUT_CLASS_SAP/_BUTTON/_SWITCH from LSConfig.h).
void initRestrictions();
int applyInputRestrictions(int actionIndex, int inputClass);

// Input timing machinery (roadmap item 15): maps an action-table row's
// compiled window boundary onto the user's stored windows, and the coarse
// threshold-level scale the Hub menu steps through (upstream #22).
unsigned long effectiveWindowMs(unsigned long defaultWindowMs);
float pressThresholdLevelUp(float currentThreshold);
float pressThresholdLevelDown(float currentThreshold);
int pressThresholdLevelFor(float currentThreshold);

// HID-layer diagnostics for the extended BH,0:0 line (bench 2026-10-16):
// [usbMounted, hidReady, hidRetrying, hidTimedOut, centerComplete,
//  reportAttempts, reportSuccesses, nonzeroReports, maxAbsReport]
void fillUsbHidDiagnostics(int out[9]);

// Joystick pipeline diagnostics for the extended BH,0:0 line (bench
// 2026-10-16, round 3): [processCount, skipState, radius*100, centerX*100,
// centerY*100, dirX, dirY, rangeValue, lastInputX, lastInputY, rawX*100,
// rawY*100, bufLastX*100, bufLastY*100]
void fillJoystickPipelineDiagnostics(int out[14]);

// External switch action remap machinery (roadmap item 11's #20 follow-up):
// boot-time load of the six stored mappings onto the live switch table, the
// allowlist check every remapped cell passes, and the cell apply/default
// helpers shared by the SA/SB/SC/GA/GB/GC endpoints.
void initSwitchActions();
extern unsigned long g_menuInputHoldMs;  // (4.5.7 bench fix) hold duration of the input currently dispatching — see selectMenuItem's hold-to-confirm gate
// 4.5.11 bench: the BLE init task's progress/failure stage, printed in the
// repeating boot header (the stage's own Serial line is discarded while the
// port is closed). Codes: 0 = not started/running, 1 = SoftDevice enable
// failed (Bluefruit.begin), 2 = DIS begin failed, 3 = HID begin failed,
// 4 = advertising start failed, 5 = full success.
extern volatile int g_bleInitStage;
// 4.5.13: the init task's SoftDevice probe, printed as sd= on the BOOT/HB
// lines: -1 = not probed yet (init still in flight), 0 = the SD was down
// when the app started (normal), 1 = the SD was ALREADY enabled (the
// bootloader left it up — LSBLE.h disables it before begin()).
extern volatile int8_t g_bleSdWasEnabled;
// 4.5.14: the begin()-mirror probe (see LSBLE.h) — pstep names the first
// failing call (1..10), pcode its SoftDevice error code, need the RAM base
// sd_ble_enable required. Printed on the BOOT/HB lines.
extern volatile uint8_t g_bleProbeStep;
extern volatile uint32_t g_bleProbeCode;
extern volatile uint32_t g_bleProbeNeed;
// 4.5.15: the pre-enable NVIC sweep (see LSBLE.h) — the masks of enabled
// IRQs found at SD-reserved priorities (< 4) at BLE-init time (bit = IRQ
// number, IRQs 0-31 in bad1, 32-47 in bad2) and the demotion count.
extern volatile uint32_t g_bleIrqBad1;
extern volatile uint32_t g_bleIrqBad2;
extern volatile uint8_t g_bleProbeDemoted;
// 4.5.19 (expert consult): the RAW err_t the failing DIS/HID begin returned —
// on the st=2 path this is sd_ble_gatts_service_add's own error code
// (0x4=NO_MEM, 0x7=INVALID_PARAM, 0xF=FORBIDDEN, 0x10=INVALID_ADDR).
extern volatile uint32_t g_bleInitRc;
// 4.5.29: computed at setup — the previous boot ended abnormally (witness
// marker, captured fault page, failed BLE verdict, or lockup reset); the
// first-host-byte BOOT report and the setup-time announcement print only
// when this is set. Healthy boots are SILENT (the released wire contract).
extern bool g_bootReportWorthy;
// 4.5.27 (Fable 5.1 consult, bench round 20 — THE ROOT CAUSE, FINALLY): the
// boot-trail/marker writes go to NRF_POWER->GPREGRET2 — and POWER is a
// SOFTDEVICE-RESTRICTED peripheral. With the SD up, every raw write is caught
// by the SD's memory-watch unit: id=NRF_FAULT_ID_APP_MEMACC (0x1001), the
// reported pc lands up to 2 instructions AFTER the offending store (the
// callee's entry — nrf_sdm.h:283), which is why the bench read String
// ctors, blinkLed and NVIC_SystemReset as the "killers" when they were
// bystanders: the stamps in their CALLERS (processSerialMessage pt=7,
// performLedAction pt=4, the witness ISR marker) were the deaths. The trail
// machinery — added in 4.5.6 as the freeze forensics — has been the killer
// of every SD-up boot since; 4.1.0's Bluetooth worked because it predates
// the machinery entirely. All retention access now routes through the
// sanctioned SVCs when the SoftDevice is up.
extern volatile bool g_softDeviceEnabled;  // set true right after Bluefruit.begin() succeeds (the SD is up from that instant until reset); read by the ISR paths — do NOT probe sd_softdevice_is_enabled() from an ISR, that is itself an SVC
// 4.6.6: true ONLY while the BLE init task is inside Bluefruit.begin(). The
// SD is enabled INSIDE that call, so between sd_softdevice_enable running
// and the flag above being raisable, neither retention path is safe: the
// SVC path would issue SD-function SVCs with the SD down (the core's
// SVC_Handler is FreeRTOS's context-switch trampoline — not a no-op) and
// the raw path is an APP_MEMACC kill with the SD up. Diagnostic retention
// writers (trail stamps, the witness marker, the streak clear) skip the
// write for this sub-second window via lipsyncRetentionUnsafe(); must-run
// writers (fault capture) are unaffected.
extern volatile bool g_bleInitWindow;
// True while a retention write must not touch the POWER registers at all.
static inline bool lipsyncRetentionUnsafe() {
  return g_bleInitWindow && !g_softDeviceEnabled;
}
#include <nrf_soc.h>  // sd_power_gpregret_get/set/clr — the sanctioned POWER-register access with the SD up
static inline uint32_t lipsyncRetentionGet(uint32_t id) {
  if (g_softDeviceEnabled) {
    uint32_t v = 0;
    (void) sd_power_gpregret_get(id, &v);
    return v;
  }
  if (id) return NRF_POWER->GPREGRET2;
  return NRF_POWER->GPREGRET;
}
// clear the `clr` bits, then set the `set` bits — mask-based, so the loop's
// trail bits and the witness ISR's marker bits can no longer clobber each
// other's read-modify-write either (a bonus: the old full-register RMWs from
// two contexts were also a race). SVC from the witness ISR at priority 7 is
// legal (the SVC handler runs at a more urgent priority; 4.9.1 fix: a
// leftover-SD USB boot also takes the SVC path now — the witness arms at
// priority 7 whenever the chip reports the SD up, because an SVC cannot
// preempt an exception at priority 0 and the marker would pend behind the
// ISR's own reset and never land).
// KNOWN NARROW WINDOW (4.9.1, documented honestly): the SVC path is a SPLIT
// clr-then-set pair. A witness SREQ landing between the loop's two SVC calls
// (CLR landed, SET not yet) erases the trail bits the stamp was writing AND
// the previous stamp — the successor reads pt=0. The window is microseconds
// per stamp against a 1 s tick, and only costs forensics (never state). The
// ISR-side pair is safe (priority 7 cannot be preempted by the loop, and the
// ISR's own reset fires after both SVCs). Closing it fully would mean
// batching both halves into one SD call, which the API does not offer.
static inline void lipsyncRetentionUpdate(uint32_t id, uint8_t clr, uint8_t set) {
  if (g_softDeviceEnabled) {
    if (clr) (void) sd_power_gpregret_clr(id, clr);
    if (set) (void) sd_power_gpregret_set(id, set);
  } else {
    // (4.9.0 review) the mask-based argument only holds for the SVC path —
    // each SVC is atomic on the SD side. The RAW path (USB-only boots, where
    // the SD is down) is a plain RMW on a memory-mapped register, and the
    // witness ISR runs at priority 0 there — it can preempt a loop-context
    // update between its load and store, and the loop's stale-value store
    // then erases the ISR's marker (a hang forensics stamp lost -> pm=0
    // "unattributed"). The raw path only ever runs with the SoftDevice down,
    // so a PRIMASK critical section is safe by construction there (no SVC
    // can be pending on a dead SD) and closes the interleaving.
    volatile uint32_t* r = (id != 0) ? &NRF_POWER->GPREGRET2 : &NRF_POWER->GPREGRET;
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    *r = (*r & ~(uint32_t)clr) | (uint32_t)set;
    if (!primask) __enable_irq();
  }
}
// 4.5.20: the previous BLE boot's flash-persisted verdict, consumed at boot
// from /bleverdict.dat (see LipSync_Firmware.ino) — pst: -1 = none, 0 = died
// during begin, 1..4 = failed at that stage, 5 = init fully succeeded; prc =
// that boot's DIS/HID error code.
extern volatile int8_t g_blePrevSt;
extern volatile uint16_t g_blePrevRc;
// 4.5.21: raised by the BLE init task (entry + every exit path), serviced by
// the loop — the ONLY cross-task handoff in the verdict path (the task never
// touches the filesystem; see LipSync_Firmware.ino's 4.5.21 regression note).
extern volatile bool g_bleVerdictDirty;
bool switchRemapAllowed(int actionNumber, bool gamepad);  // (4.5.7 review) column-aware: the gamepad cells reject the mouse hold modes
void sanitizeInputWindows();                              // (4.5.7 review) boot-load validation for WS/WL/WM
void switchRemapApplyCell(int jack, int window, bool gamepad, int actionNumber);
int switchRemapDefaultCell(int jack, int window, bool gamepad);
int switchRemapLiveCell(int jack, int window, bool gamepad);  // 4.9.1: the live table cell (read-truth getters)
int hardBandLiveAction(bool hardSip);                          // 4.9.1: the live HA/HB row (read-truth getters)

// Witness v2 boot-trail (diagnostic, bench 2026-10-23): milestone stamps that
// survive resets in .noinit; the HB line prints the last one reached.
void bootTrailStamp(uint8_t step);

// Screen page-transition continuations passed to LSScreen::scheduleTransition()
// (the splash clear is LSScreen::finishSplashClear, a member so it can reset
// _screenStateTimerId when it fires)
void screenExitTransitionDone();
void screenScrollTransitionDone();
void screenChangeModeTransitionDone();
void screenReturnToMenuTransitionDone();
void screenFullCalibConfirmTransitionDone();
void screenFactoryResetConfirm2TransitionDone();
void screenModeLockedTransitionDone();
// Destructive hold-gate note continuation (bench 2026-11-06)
void screenHoldConfirmNoteTransitionDone();
// Upstream #16: the Bluetooth host pages' dwell continuations
void screenBtSwitchTransitionDone();
void screenBtNoteTransitionDone();

// Calibration state machine callbacks (timer continuations, referenced by
// the calibration timers and by setJoystickCalibration / setJoystickInitialization)
void performJoystickCenter(int* args);
void performJoystickCenterStep(int* args);
void performJoystickCalibration(int* args);
void performJoystickCalibrationStep(int* args);
void abortJoystickCalibration(bool calibFromApi, const char* reason);

#endif // _LSAPIDECLARATIONS_H
