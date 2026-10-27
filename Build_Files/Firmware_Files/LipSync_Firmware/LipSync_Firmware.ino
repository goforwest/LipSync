/* 
* File: LipSync_Firmware.ino
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

#include <WString.h>
#include <Wire.h>
#include <FreeRTOS.h>  // 4.5.10: the BLE init task's static allocation (see setup)
#include <task.h>
#include "LSUtils.h"
#include "LSApiDeclarations.h"  // Explicit cross-tab declarations (roadmap item 5) — early so every tab sees them
#include "LSConfig.h"
#include "LSTimer.h"
#include <ArduinoJson.h>
#include "LSOutput.h"
#include "LSUSB.h"
#include "LSBLE.h"
#include "LSCircularBuffer.h"
#include "LSInput.h"
#include "LSPressure.h"
#include "LSJoystick.h"
#include "LSMemory.h"
#include "LSScreen.h"
#include "LSBuzzer.h"
#include "LSWatchdog.h"

// Unique ID
String g_deviceUID = "";  // Global variable for storing unique identifier for board

// Communication mode and debug mode variables
int g_comMode;        // 0 = None , 1 = USB , 2 = Wireless
int g_operatingMode;  // 0 = None, 1 = Mouse, 2 = Gamepad, 3 = Safe
int g_soundMode;      // 0 = None, 1 = Basic, 2 = Advanced // TODO 2025-Feb-05 Currently not used - buzzer.begin sets sound mode from memory
int g_lightMode;      // 0 = None, 1 = Basic, 2 = Advanced

int g_debugMode;  // 0 = Debug mode is Off
                  // 1 = Joystick debug mode is On
                  // 2 = Pressure debug mode is On
                  // 3 = Buttons debug mode is On
                  // 4 = Switch debug mode is On
                  // 5 = Sip & Puff state debug mode is On

int g_errorCode = 0;  // Global variable for storing error code. 0 is no error. Additional errors defined in LSConfig.h

uint32_t g_lastRebootReason = 0;
bool g_watchdogReset = false;
uint8_t g_watchdogBootCount = 0;  // Consecutive watchdog-reset boots (see LSWatchdog.h)
uint8_t g_hangBootStreak = 0;     // Consecutive TIMER3-witnessed loop-hang deaths (survives the SREQ resets via GPREGRET2 bits 0-1 — see checkResetReason)
bool g_safeModeEnabled = false;
int  g_safeModeReason = 0;
bool readyToUseFirstTime = true;

bool g_btIsConnected = false;   // Bluetooth connection state

bool g_displayConnected = false;                   // Display connection state
bool g_joystickSensorConnected = false;            // Joystick sensor connection state
bool g_mouthpiecePressureSensorConnected = false;  // Mouthpiece pressure sensor connection state
bool g_ambientPressureSensorConnected = false;     // Ambient pressure sensor connection state

// LED module variables
ledStateStruct* ledCurrentState = new ledStateStruct;  // pointer to LED current state structure
bool ledActionEnabled = false;

// Input module variables
int buttonActionSize, switchActionSize;
unsigned long buttonActionMaxTime, switchActionMaxTime;
inputStateStruct buttonState, switchState;

int inputButtonPinArray[] = { CONF_BUTTON1_PIN, CONF_BUTTON2_PIN };
int inputSwitchPinArray[] = { CONF_SWITCH1_PIN, CONF_SWITCH2_PIN, CONF_SWITCH3_PIN };

LSInput ib(inputButtonPinArray, CONF_BUTTON_NUMBER);  // Instance of input button object (tactile buttons on Hub)
LSInput is(inputSwitchPinArray, CONF_SWITCH_NUMBER);  // Create an instance of the input switch object (external switches)

inputStateStruct sapActionState;

int sapActionSize;
unsigned long sapActionMaxTime = 0;

// Timer related variables
int actionTimerId[1];  // 1 action timer
LSTimer<int> actionTimer;

int calibrationTimerId[2];  // 2 calibration timers: 0 - , 1-
LSTimer<int> calibrationTimer;

int pollTimerId[10];  // 9 poll timers — the CAPTURED ids (4.5.7 review): every enable/disable path must use these, not the CONF_TIMER_* constants; the two are equal only while every boot registration lands sequentially into an empty pool, and one refused registration or reorder away from the constants is disabling the WRONG feature's loop
LSTimer<void> pollTimer;

int ledTimerId[5] = {-1, -1, -1, -1, -1};  // LED timers 0 - startup feedback, 1 - IBM, 2- normal blinks, 3 - Bluetooth Status, 4 - error — -1 = none registered (4.5.7 review: zero-init made blinkLed's delete-before-rearm guard delete pool slot 0 on its first call)
LSTimer<ledStateStruct> ledStateTimer;

int usbConnectTimerId[1];
LSTimer<int> usbConnectTimer;

unsigned int g_usbAttempt = 0;
unsigned int g_usbConnectDelay = CONF_USB_HID_INIT_DELAY;
unsigned long g_usbNotReadySinceMs = 0;  // When the current HID-not-ready stretch began; 0 = none in progress
unsigned int g_usbReenumCount = 0;        // Forced re-enumerations spent on the current episode
unsigned long g_watchdogLastFeedMs = 0;   // Loop-liveness witness (bench 2026-10-18): ms timestamp of the last watchdog feed
volatile bool g_watchdogArmed = false;   // True from initWatchdog() until the retry limit stops re-arming (ISR-read by the TIMER3 witness — 4.5.7 review)
// The .noinit once-guard for the USB-recovery auto-restart: a full chip reset
// is the one cure that has cleared every configured-but-HID-dead boot on the
// bench (2026-10-17: a wedge survived all three forced re-enumerations), but
// an automatic restart must never loop. This word is written with
// CONF_USB_RESTART_MAGIC immediately before the reset and checked at the next
// boot — .noinit survives NVIC_SystemReset (startup clears .bss by symbols,
// not whole RAM), so a matching magic means the previous boot already spent
// its one auto-restart this power cycle. Any healthy boot clears it, re-arming
// the guard for a future incident; a cold power-on reads random garbage, which
// the magic test simply fails.
// WITNESS v4 + PLATFORM FIX (bench 2026-10-23): the .noinit section DOES NOT
// survive resets on this platform — the bench proved it (every soft-reset and
// watchdog boot read boot#1/ptrail=0 back; the map places the section at
// 0x20007bac but its content is zeroed by the time the next boot's setup()
// runs — the bootloader/core scrubs it). Every cross-reset word therefore
// moves to the nRF52 POWER RETENTION REGISTERS, which survive every warm
// reset and clear only on cold power-on (the mechanism the 4.2.0 watchdog
// boot counter has used all along):
//   GPREGRET2 bits  0- 7 : consecutive-watchdog-boot counter (4.2.0, unchanged)
//   GPREGRET2 bits  8-15 : witness boot TRAIL — the last milestone stamped
//                         before a boot died; the next boot prints it as ptrail=
//   GPREGRET2 bits 16-23 : the USB-recovery once-guard (438643c), MIGRATED
//                         here from .noinit where it never survived the very
//                         reset it was armed for — the once-per-power-cycle
//                         auto-restart contract is restored on hardware
//   GPREGRET2 bits 24-31 : soft-reset boot counter (witness: counts boots in
//                         a reset chain; cold power-on re-seeds at 1)
// The core's own DFU magic lives in GPREGRET (1), not GPREGRET2.
// WITNESS v5 (bench 2026-10-23): the platform offers NO reliable cross-reset
// storage — the retention register GPREGRET2 is 8 bits wide (SVD: only bits
// 0-7 implemented; the 4.2.0 watchdog counter lives there), GPREGRET belongs
// to the bootloader's DFU magic, and the .noinit section is wiped by the
// boot path (the map even shows its orphan placement straddling
// __bss_start__, so most of it lands inside the bss-clear loop). The trail
// is therefore a plain RAM byte, printed live by the HB line: for a freezing
// boot, the LAST PRINTED trail names the site the loop died at. Cross-reset
// questions are answered by rst= (the nRF52 RESETREAS register — the one
// reset fact that does survive).
// WITNESS v7 (bench 2026-10-23): the freeze's CPU does NOT hardfault (the
// Seeed core's HardFault_Handler is an instant NVIC_SystemReset — a fault
// would read rst=0x4 on the next boot; every freeze successor reads rst=0x2,
// the 30-s watchdog: the CPU is SPINNING, not faulting) — and every bounded
// wait in the Wire/Tone/HardwarePWM paths checks out, so the spin site is
// somewhere no witness has survived to report: RAM dies at the reset. The
// trail therefore moves into the retention register's spare bits
// (GPREGRET2 bits 5-7 — the watchdog counter only needs bits 0-1): the last
// 3-bit milestone a dying boot stamped SURVIVES the watchdog reset, and the
// next boot prints it as pt=. Trail codes (3 bits): 1 setup-done, 2
// safeModePage, 3 safeMenu, 4 LED action, 5 softwareReset, 6
// usbCheckConnection.
uint8_t g_bootTrailPrev = 0;  // The DEAD boot's last trail code (read from retention at setup)
uint8_t g_hangMarkerPrev = 0;  // The DEAD boot's hang-witness marker (retention bits 2-4)
volatile bool g_shutdownInProgress = false;  // witness v9: softwareReset's deliberate blocking shutdown is NOT a hang — the witness exempts it
// (4.5.7 review) Seconds since the loop last fed the watchdog, counted BY
// THE WITNESS ITSELF: the old ISR called millis() (a FreeRTOS xTaskGetTickCount
// read — unsupported use at interrupt priority 0) and read non-volatile
// globals shared with loop context. The counter is a single aligned volatile
// word: the ISR increments it on every 1 s tick, watchdogLoop() (loop
// context) writes it back to 0 on every feed — a 32-bit aligned store/load
// is atomic on the ARM, so the worst case of the race is one tick of lag.
// >3 consecutive un-fed seconds (counter reaches 4) is a loop hang.
volatile uint32_t g_witnessSecondsUnfed = 0;
uint8_t readTrailByte()            { return (NRF_POWER->GPREGRET2 >> 5) & 0x07; }
void writeTrailByte(uint8_t step)  { NRF_POWER->GPREGRET2 = (NRF_POWER->GPREGRET2 & ~0xE0UL) | ((step & 0x07) << 5); }

// WITNESS v8 — THE HANG WITNESS (bench 2026-10-23): this round proved the
// freeze ends in a WATCHDOG reset that WIPES GPREGRET2 (every freeze
// successor read pt=0 while the dead boot demonstrably stamped trail=1/3;
// SREQ resets preserve it — pt=5 survived every SR). So the freeze's death
// site could never be reported. This converts the freeze's silent DOG death
// into an SREQ the trail survives: TIMER3 (free — the core only touches
// TIMER2 read-only for the bootloader version) fires an interrupt every
// second and watches the loop's watchdog feed stamp — the loop pass
// refreshes it on every iteration. If the loop hangs for >3 s, the IRQ
// writes a hang marker into retention bits 2-4 and resets via
// NVIC_SystemReset (SREQ): the trail bits 5-7 SURVIVE, the next boot prints
// pt= (the last milestone the dead boot stamped) and pm=1 (hang witnessed),
// and the bench cycle drops from 30 s to ~4 s. If the hang instead masks
// interrupts, the IRQ never fires and the hardware DOG still ends it
// (pt=0) — that split (pm=1 vs pm=0) is itself decisive: IRQs-live hang vs
// IRQs-masked hang.
//
// 4.5.13: declared up here because initHangWitness() reads it at ARM time.
// A BLE boot has the SoftDevice enable still PENDING when the witness arms
// (the init task starts at the very END of setup), so the witness must
// take an SD-legal priority from the start — see the priority note in the
// body below. g_bleInitPending is set by beginComOpMode long before this
// point in setup.
static volatile bool g_bleInitPending = false;   // a BLE boot deferred its init to the task
void initHangWitness() {
  NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
  NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  NRF_TIMER3->PRESCALER = 9;                       // 16 MHz / 512 = 31.25 kHz
  NRF_TIMER3->CC[0] = 31250;                       // 1 s
  NRF_TIMER3->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
  NRF_TIMER3->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
  // witness v9 + bench fix: HIGHEST priority — a hang that still blocks it
  // can only be PRIMASK-level masking. BUT the SoftDevice reserves specific
  // priority levels when it is enabled, and the bench (2026-10-26, rounds
  // 8-11) pinned the ACTUAL reserved set for this S140 6.1.1 to {0, 1, 4} —
  // the core's own TinyUSB port states it verbatim ("Priorities 0, 1, 4
  // (nRF52) are reserved for SoftDevice"), the stock example passes the
  // SDM's enable check with USBD at priority 2, and every failing BLE boot
  // in the hunt had an enabled interrupt at exactly 4 — including this
  // witness itself, whose 4.5.13 move to 4 (based on the WRONG "app-legal
  // = 4-7" model) blocked sd_softdevice_enable with 0x1001 on every BLE
  // boot since 4.5.10, when the BLE init moved AFTER the witness arms.
  // Priority 7 is legal under BOTH models (classic 4-7 and the confirmed
  // {0,1,4}-reserved), never masked by FreeRTOS's BASEPRI=2 critical
  // sections, and still preempts any thread-mode hang — the witness's
  // actual job. USB-only boots (SD never comes up) keep priority 0 for
  // the freeze hunt.
  NVIC_SetPriority(TIMER3_IRQn, (needsInitialization && !g_bleInitPending) ? 0 : 7);
  NVIC_ClearPendingIRQ(TIMER3_IRQn);
  NVIC_EnableIRQ(TIMER3_IRQn);
  NRF_TIMER3->TASKS_START = 1;
  g_witnessSecondsUnfed = 0;
}
extern "C" void TIMER3_IRQHandler(void) {
  NRF_TIMER3->EVENTS_COMPARE[0] = 0;
  // Counter-based hang detection (4.5.7 review — see g_witnessSecondsUnfed):
  // no millis() call, no non-volatile reads; the only loop-context writer is
  // watchdogLoop()'s reset. 4 consecutive un-fed seconds = the loop stalled
  // past 3 s with interrupts alive.
  if (!g_shutdownInProgress && g_watchdogArmed && ++g_witnessSecondsUnfed >= 4) {
    NRF_POWER->GPREGRET2 = (NRF_POWER->GPREGRET2 & ~0x1CUL) | (1UL << 2);  // marker 1: loop hang witnessed (trail bits preserved)
    NVIC_SystemReset();  // SREQ — retention survives; the next boot reports pt=/pm=
  }
}

// Live boot trail (witness v7, 4.5.7 review reconciliation) — ONE table,
// the one the stamps actually use. 3 bits, codes 1-7:
//   1  setup complete                (stamped at the end of setup())
//   2  safeModePage drawing           (LSScreen::safeModePage)
//   3  safe-mode menu registered      (LSScreen::safeModeMenu)
//   4  LED action running             (performLedAction)
//   5  softwareReset starting         (softwareReset — menu/serial/confirm resets)
//   6  usbCheckConnection running     (its entry stamp; the USB one-shot
//                                      restart resets from INSIDE this
//                                      context, so pt=6 names it truthfully
//                                      and pt=5 stays unambiguous)
//   7  serial command dispatching     (processSerialMessage — stamped when
//                                      a real message is handled, not on
//                                      every idle loop pass, so pt=7 names
//                                      an actual dispatch site)
// pt=0 means the dead boot never stamped (a pre-setup death).
void bootTrailStamp(uint8_t step) {
  writeTrailByte(step);
}

// 4.5.10 BENCH FIX — the BLE init task. The core's loop task (which runs
// setup() and loop()) has a hardcoded 4096-byte stack and configCHECK_FOR_
// STACK_OVERFLOW=1 (mode 1 — misses overflow-and-return), and the BLE boot's
// only-in-that-path call depth (Bluefruit.begin: SoftDevice enable, GATT
// table, HID service, advertising assembly — kilobytes) silently corrupted
// it: a delayed HardFault at ~2.6-4 s into the boot (rst=0x4, pm=0, pt=4 —
// after the boot-center LED stamps, around readyToUse and the first
// btmouse.move), instant reset, infinite BLE boot loop. The gamepad boot
// (no Bluefruit.begin) worked perfectly on the same bench session. The BLE
// init now runs on its own task with a 16 KB static stack, started at the
// end of setup; every BLE call site in the loop is guarded so the pre-init
// window is a no-op window, not a fault window.
volatile int g_bleInitStage = 0;                 // 4.5.11: init-task progress/failure stage (see LSApiDeclarations.h)
volatile int8_t g_bleSdWasEnabled = -1;          // 4.5.13: the init task's SD probe: -1 = not probed yet, 0 = the SoftDevice was down (normal), 1 = it was ALREADY enabled when the app started (bootloader leftover — see LSBLE.h)
volatile uint8_t g_bleProbeStep = 0;             // 4.5.14: the begin()-mirror probe's first failing call (0 = not run/non-BLE, 1..10 = that call failed, 11 = all passed) — see LSBLE.h
volatile uint32_t g_bleProbeCode = 0;            // 4.5.14: the failing call's SoftDevice error code (0 = none)
volatile uint32_t g_bleProbeNeed = 0;           // 4.5.14: sd_ble_enable's required app RAM base (on NO_MEM, the smoking gun vs 0x20006000)
volatile uint32_t g_bleInitRc = 0;              // 4.5.19 (expert consult): the RAW err_t from the failing DIS/HID begin — sd_ble_gatts_service_add's own code on the st=2 path (0x4=NO_MEM, 0x7=INVALID_PARAM, 0xF=FORBIDDEN, 0x10=INVALID_ADDR)
volatile uint32_t g_bleIrqBad1 = 0;              // 4.5.15: pre-demotion mask of enabled IRQs 0-31 found at an SD-reserved priority (< 4) at BLE-init time (bit = IRQ number)
volatile uint32_t g_bleIrqBad2 = 0;              // 4.5.15: the same mask for IRQs 32-47
volatile uint8_t g_bleProbeDemoted = 0;          // 4.5.15: how many the sweep demoted to 4 before the enable
volatile int8_t g_blePrevSt = -1;                 // 4.5.20: the PREVIOUS BLE boot's flash-persisted init verdict (-1 = none, 0 = died DURING begin, 1..4 = failed at that stage, 5 = init fully succeeded) — read+consumed from /bleverdict.dat at boot
volatile uint16_t g_blePrevRc = 0;                // 4.5.20: that boot's DIS/HID raw error code
static volatile bool g_bleInitDone = false;       // the task finished (success or failure)

// 4.5.20/4.5.21 — BLE-VERDICT PERSISTENCE (the reset-proof capture): the
// BLE boots of rounds 13+ die faster than the host can open the port, so
// st=/rc=/pstep= were being lost with the boot itself. The verdict goes to
// the settings filesystem (which survives resets), and the next boot prints
// it as pst=/prc= and consumes the file.
//
// 4.5.21 REGRESSION FIX — THE BLE TASK NO LONGER TOUCHES THE FILESYSTEM. The
// 4.5.20 shape called bleVerdictPersist() from the init task, which opened
// LSMEMORY.h's ONE GLOBAL shared File handle concurrently with the loop
// task's settings writes — two FreeRTOS tasks sharing one stateful handle
// clobbered each other's file state and corrupted the settings file (bench
// round 14: after a CM,1:2 cycle the survivor read IN=0.00|0.00 CA=0.00|0.00
// CM=1 — the re-seed signature; the calibration was wiped and the mode
// reverted to the compiled default). The task now only raises a flag; the
// LOOP — which already owns every other FS operation, all sequential —
// performs the persist. All filesystem access is single-threaded again, and
// the verdict ride survives. The entry record still lands well before
// begin() finishes (the loop services the flag within milliseconds).
// 8-byte record with magic headers both ends: a torn write fails the magic
// check and reads as absent.
static const char* const BLE_VERDICT_FILE = "/bleverdict.dat";
volatile bool g_bleVerdictDirty = false;  // raised by the init task, serviced by the loop — the ONLY cross-task handoff (no FS access off-loop)
void bleVerdictPersist() {  // LOOP-CONTEXT ONLY (see the 4.5.21 note above)
  File verdictFile(InternalFS);  // its own handle — never the shared settings object
  InternalFS.remove(BLE_VERDICT_FILE);
  delay(1);
  if (verdictFile.open(BLE_VERDICT_FILE, FILE_O_WRITE)) {
    uint8_t rec[8] = { 0xB5,
                       (uint8_t)g_bleInitStage,
                       (uint8_t)g_bleProbeStep,
                       (uint8_t)(g_bleSdWasEnabled & 0xFF),
                       (uint8_t)(g_bleInitRc & 0xFF),
                       (uint8_t)((g_bleInitRc >> 8) & 0xFF),
                       g_bleProbeDemoted,
                       0xA5 };
    verdictFile.write(rec, sizeof(rec));
    delay(1);
    verdictFile.close();
  }
}
void bleVerdictConsume() {
  g_blePrevSt = -1;  // absent until proven present
  g_blePrevRc = 0;
  if (file.open(BLE_VERDICT_FILE, FILE_O_READ)) {
    uint8_t rec[8];
    const size_t n = file.read(rec, sizeof(rec));
    file.close();
    if (n == sizeof(rec) && rec[0] == 0xB5 && rec[7] == 0xA5) {
      g_blePrevSt = (int8_t)rec[1];
      g_blePrevRc = (uint16_t)rec[4] | ((uint16_t)rec[5] << 8);
    }
    InternalFS.remove(BLE_VERDICT_FILE);  // one-shot: the verdict describes the immediately-previous boot only
  }
}
static char g_bleDeviceName[CONF_BLE_DEVICE_NAME_SZ] = {0};  // the advertising name (static: outlives setup's frame)
static StackType_t g_bleInitStack[CONF_BLE_INIT_TASK_STACK_BYTES / sizeof(StackType_t)];
static StaticTask_t g_bleInitTCB;
static volatile bool g_hostHasSpoken = false;     // 4.5.10: the repeating boot header stops at the first host byte

// Joystick module variables and structures
int acceleration = 0;
int g_scrollLevel = 0;
int g_scrollNumRuns = 0;

int outputAction;
bool canOutputAction = true;
// 4.5.7 bench fix: the hold duration (ms) of the input that is CURRENTLY
// dispatching, for selectMenuItem's hold-to-confirm gate on the destructive
// rows. Set fresh on every evaluateOutputAction pass (0 = no input in
// flight — a stale value can never leak into a later decision), set from
// the releasing input's elapsed right before performOutputAction, and set
// to "always pass" by the serial CH,1 path (no hold concept on the wire).
unsigned long g_menuInputHoldMs = 0;
bool g_startupCenterReset = true;
bool g_resetCenterComplete = false;  // global variable for center reset status
bool g_calibrationError = false;     // Global variable for error in full calibration
bool g_calibrationRunning = false;   // 4.5.7 review: a full-calibration chain is in flight — CA,0/EX reads must not overwrite the accumulating corner slots (see getJoystickCalibration)
bool g_calibrationCenterStep = false;  // 4.5.7 review: the calibration chain's OWN internal step-5 center is starting (synchronous bypass for the center-flow in-flight gate — both chains share the calibrationTimerId[] variables, so a concurrent center would clobber the corner reading-timer id and silently drop the corner's persistence + ack)

bool settingsEnabled = false;  // Serial input settings command mode enabled or disabled

// Input restriction state (roadmap item 11) — loaded from the stored settings
// by initRestrictions(); the serial API setters keep them live.
bool g_sapDisabled = false;        // SD — sip & puff inputs cannot fire actions
bool g_menuButtonsOnly = false;    // MA — only the hub buttons may control the menu
bool g_modeLocked = false;         // LK — inputs cannot change the operating mode
bool g_freezeCursorDuringSap = false;  // CF — cursor output freezes while a sip/puff is in progress

// Input timing windows (roadmap item 15, upstream #62/#9) — stored 0 means
// the compiled default; the getters/setters keep these live. These are the
// STORED values; effectiveWindowMs() maps them onto the action tables.
unsigned long g_inputWindowShortMs = 0;  // WS — short | long boundary
unsigned long g_inputWindowLongMs = 0;   // WL — long | very-long boundary
unsigned long g_inputWindowMaxMs = 0;    // WM — very-long boundary

// External switch action remapping (roadmap item 11's #20 follow-up,
// firmware 4.5.0): the live copy of switchActionProperty the input loop
// evaluates against. initInput() seeds it from the compiled table and
// initSwitchActions() (called from initRestrictions, with the other
// input-related stored settings) overlays any stored remaps — so a device
// with no stored mapping (every settings file written by older firmware)
// behaves byte-for-byte like the compiled table.
inputActionStruct g_switchActionTable[sizeof(switchActionProperty) / sizeof(inputActionStruct)];

// Timing Variables for Debugging
unsigned long beginMillis;
unsigned long beforeComOpMillis;
unsigned long afterComOpMillis;


// Create instances of classes
LSMemory mem;     // Create an instance of LSMemory for managing flash memory.
LSJoystick js;    // Create an instance of the LSJoystick object
LSPressure ps;    // Create an instance of the LSPressure object
LSOutput led;     // Create an instance of the LSOutput LED object
LSScreen screen;  // Create an instance of the LSScreen Object for OLED Screen
LSBuzzer buzzer;  // Create an instance of the LSBuzzer Object

LSUSBMouse usbmouse;   // Create an instance of the USB mouse object
LSBLEMouse btmouse;    // Create an instance of the BLE mouse object
LSUSBGamepad gamepad;  // Create an instance of the USB gamepad object


//***BLE INIT TASK BODY***//
// Function   : bleInitTaskBody
//
// Description: Runs the full BLE stack bring-up (btmouse.begin ->
//              initializeBluefruit: SoftDevice enable, DIS, HID, advertising)
//              on a 16 KB static stack instead of the loop task's 4 KB.
//              On failure the failing stage's BOOT line has already printed
//              (initializeBluefruit checks every begin()); needsInitialization
//              stays true and every guarded call site stays inert. The task
//              suspends itself — it never returns.
static void bleInitTaskBody(void* arg) {
  (void)arg;
  btmouse.begin(g_bleDeviceName);
  g_bleInitDone = true;               // completion flag first...
  if (needsInitialization) {          // ...then the verdict (true = a stage failed)
    Serial.println(F("BOOT: BLE task FAILED (see stage line above)"));
  } else {
    Serial.println(F("BOOT: BLE task up"));
  }
  vTaskSuspend(NULL);  // a FreeRTOS task must never return
}

//***MICROCONTROLLER AND PERIPHERAL CONFIGURATION***//
// Function   : setup
//
// Description: This function handles the initialization of variables, pins, methods, libraries. This function only runs once at powerup or reset.
//
// Parameters :  void
//
// Return     : void
//*********************************//
void setup() {
  initGlobals();  //  Intialize global variables to default values
  checkResetReason();  //  Check reason for reset and reset the reset-reason register

  // Witness v7: capture what the DEAD boot last stamped (survives the reset
  // in the retention register), then start this boot's trail.
  g_bootTrailPrev = readTrailByte();
  g_hangMarkerPrev = (NRF_POWER->GPREGRET2 >> 2) & 0x07;
  NRF_POWER->GPREGRET2 &= ~0x1CUL;  // consume the marker — one hang reports once
  writeTrailByte(0);

  beginMillis = millis();  // Intialize timer

  // Keep the bus dark until the composite descriptor is complete. The core
  // pulls D+ (so the host can start enumerating) before setup() runs, and
  // the slow parts of boot (LittleFS mount, settings load) leave a long
  // window where only the CDC interface is registered: a host that is
  // actively hunting for the post-flash port can enumerate that CDC-only
  // device and never re-enumerate, leaving the HID interface down for the
  // whole boot (bench 2026-10-16: BH showed usbMounted=1, hidReady=0,
  // attempts climbing, successes=0 until a manual restart). Detaching here
  // makes every premature enumeration attempt fail; beginComOpMode()
  // attaches once the full descriptor exists.
  USBDevice.detach();

  Serial.begin(115200);

  // BOOT TRACE (4.5.9 bench round 3): a handful of short lines naming each
  // boot milestone. The bench's BLE boot looped with zero wire visibility
  // (the app could not even open the port), so every boot now announces its
  // reset reason up front and the milestones below name the death site even
  // inside a fast fault loop — the last line printed IS the fault's
  // address. Unprefixed lines are proven app-tolerated (three sessions of
  // HB/ERROR lines dropped as undecodable, no waiter false-match possible).
  Serial.print(F("BOOT: 4.5.9 rst=0x"));
  Serial.print(g_lastRebootReason, HEX);
  Serial.print(F(" pt="));
  Serial.print(g_bootTrailPrev);
  Serial.print(F(" pm="));
  Serial.println(g_hangMarkerPrev);

  initMemory();  // Initialize Memory

  //initUID(); // Intilize unique identifier
  getDeviceID(false, false);       // Retrieve Device ID from memory
  getVersionNumber(false, false);  // Retrieve version number from memory

  initLed();  // Initialize LED Feedback

  initBuzzer();  // Initialize Buzzer

  initInput();  // Initialize input buttons and input switches

  initRestrictions();  // Load the input restriction settings (roadmap item 11) before any input can fire

  // Safe-mode decision BEFORE any interface begins (4.5.7 review): the
  // decision half of checkSafeMode (button debounce + reset reason + the
  // hang-witness settling below) has no side effects on timers or the
  // screen — those live in toggleSafeMode, still called at its original
  // place below. Deciding first lets beginComOpMode skip the mode HID
  // interface and BLE on a safe boot, making the documented "safe mode
  // comes up CDC-only: serial recovery is the whole point" actually true
  // (4.5.6 shipped with the interfaces registered BEFORE the flip — an
  // inert-but-present mouse HID on a safe boot and BLE advertising on a
  // CM=2 safe boot).
  checkSafeMode();

  beforeComOpMillis = millis() - beginMillis;  // Note time before USB/BT connection
  beginComOpMode();                            // Initialize Operating Mode, Communication Mode, and start instance of mouse or gamepad
  afterComOpMillis = millis() - beginMillis;   // Note time after USB/BT connection


  checkI2C();  // Check that I2C devices are connected

  if (g_displayConnected) {
    initScreen();
  }

  if (USB_DEBUG) {
    unsigned long startSerialWaitTime = millis();
    unsigned long currentSerialWaitTime = millis();
    const unsigned long SERIAL_WAIT_TIMEOUT = 30000;
    if (g_displayConnected && !Serial) {
      screen.print4LineString("Waiting", "for serial", "connection", "...");
    }
    while (!Serial) {  //wait for serial connection to establish
      delay(1);
      currentSerialWaitTime = millis();
      if (currentSerialWaitTime - startSerialWaitTime > SERIAL_WAIT_TIMEOUT){
        break;
      }
    }
    Serial.println("USBDEBUG: Serial Connected");
    if (g_displayConnected) {
      screen.print4LineString(" ", "Serial", "connected.", " ");
    }
  }

  if (g_displayConnected) {
    screen.splashScreen();
  }

  if (g_mouthpiecePressureSensorConnected && g_ambientPressureSensorConnected) {
    initSipAndPuff();  // Initialize Sip And Puff
  }

  if (g_joystickSensorConnected) {
    initJoystick();  // Initialize Joystick
  }

  initAcceleration();  // Load the stored cursor acceleration level (roadmap item 9)

  initDebug();  // Initialize Debug Mode operation

  startupFeedback();  // Startup LED Feedback

  if (USB_DEBUG) { Serial.println("USBDEBUG: Initialize timers."); }
  // Configure poll timer to perform each feature as a separate loop
  pollTimerId[CONF_TIMER_JOYSTICK] = pollTimer.setInterval(CONF_JOYSTICK_POLL_RATE, 0, joystickLoop);  // poll rate, start delay, function
  pollTimerId[CONF_TIMER_PRESSURE] = pollTimer.setInterval(CONF_PRESSURE_POLL_RATE, 0, pressureLoop);
  pollTimerId[CONF_TIMER_INPUT] = pollTimer.setInterval(CONF_INPUT_POLL_RATE, 0, inputLoop);
  pollTimerId[CONF_TIMER_BLUETOOTH] = pollTimer.setInterval(CONF_BT_FEEDBACK_POLL_RATE, 0, btFeedbackLoop);
  pollTimerId[CONF_TIMER_DEBUG] = pollTimer.setInterval(CONF_DEBUG_POLL_RATE, 0, debugLoop);
  pollTimerId[CONF_TIMER_SCROLL] = pollTimer.setInterval(CONF_SCROLL_POLL_RATE, 0, joystickLoop);
  pollTimerId[CONF_TIMER_SCREEN] = pollTimer.setInterval(CONF_SCREEN_POLL_RATE, 0, screenLoop);
  pollTimerId[CONF_TIMER_USB] = pollTimer.setInterval(CONF_USB_POLL_RATE, 0, usbConnectionLoop);
  pollTimerId[CONF_TIMER_WATCHDOG] = pollTimer.setInterval(CONF_WATCHDOG_POLL_RATE, 0, watchdogLoop);

  // Boot-time registration audit (bench 2026-10-18, pool hygiene): a refused
  // registration returns -1 and every caller used to ignore it — a
  // "registered" loop that silently never ran is how a pool death hides
  // (the LSTimer now refuses loudly and counts, but the ids must be checked
  // at least once at the place they're captured). With the pool raised to
  // 16 this cannot fire today; if a future feature exhausts it, this line
  // names the dead feature at boot instead of leaving it a mystery.
  for (int timerIdx = 0; timerIdx < CONF_TIMER_NUM; timerIdx++) {
    if (pollTimerId[timerIdx] < 0) {
      Serial.print("ERROR: poll timer ");
      Serial.print(timerIdx);
      Serial.println(" failed to register — that loop WILL NOT RUN");
    }
  }


  pollTimer.disable(pollTimerId[CONF_TIMER_USB]); // TODO 2025-Feb-21 Disable usbConnectionLoop until implemented
  // If USB is not connected, try to reconnect
  if (g_comMode == CONF_COM_MODE_USB) {
    usbConnectTimerId[0] = usbConnectTimer.setTimeout(g_usbConnectDelay, usbCheckConnection);  // Call usbCheckConnection function when g_usbConnectDelay reached
  }


  //enablePoll(true);

  ledActionEnabled = true;

  //ledReadyFeedback();

  if (g_displayConnected) {
    pollTimer.enable(pollTimerId[CONF_TIMER_SCREEN]);
  } else {
    pollTimer.disable(pollTimerId[CONF_TIMER_SCREEN]);
  }

  if (g_mouthpiecePressureSensorConnected && g_ambientPressureSensorConnected) {
    pollTimer.enable(pollTimerId[CONF_TIMER_PRESSURE]);
    if (USB_DEBUG) { Serial.println("USBDEBUG: Pressure timer started."); }
  } else {
    pollTimer.disable(pollTimerId[CONF_TIMER_PRESSURE]);
    if (USB_DEBUG) { Serial.println("USBDEBUG: Pressure timer NOT started."); }
  }

  if (g_joystickSensorConnected) {
    pollTimer.enable(pollTimerId[CONF_TIMER_JOYSTICK]);
    pollTimer.enable(pollTimerId[CONF_TIMER_SCROLL]);
  } else {
    pollTimer.disable(pollTimerId[CONF_TIMER_JOYSTICK]);
    pollTimer.disable(pollTimerId[CONF_TIMER_SCROLL]);
  }

  // If any devices are not connected, handle error
  if (!g_displayConnected || !g_mouthpiecePressureSensorConnected || !g_ambientPressureSensorConnected || !g_joystickSensorConnected) {
    hardwareErrorCheck();
  }

  // THE FREEZE FIX, round 2 (bench 2026-10-23, the correlation that closed the
  // hunt): ledWaitFeedback used to run BEFORE checkI2C, allocating the LED
  // brightness channels on the shared HwPWM instance; hardwareErrorCheck's
  // playErrorSound then called tone(), and the Seeed core's tone() CLAIMS
  // THAT SAME PWM PERIPHERAL (Tone.cpp hard-codes PWM2) and rewrites its
  // sequence/loop configuration under the live LED channels — the first
  // note plays, the peripheral is left in a conflicted state, and the loop
  // froze 0.9-8.6 s later on 15/15 boots whose joystick sensor was absent
  // (the only boots that ever play the error sound; all-sensor safe boots
  // and healthy boots never froze). The two PWM users are now SEQUENCED:
  // the error sound plays first (with no LED channels allocated), releases
  // the peripheral via noTone(), and the all-LEDs-on feedback runs here —
  // after any sound, on every boot. Same cues for the user, no overlap.
  ledWaitFeedback();  // Turn on all LEDS (moved from before checkI2C — see above)

  //handleHardwareErrors(); // Currently not implemented

  toggleSafeMode(g_safeModeEnabled);

  // Arm the hardware watchdog on every boot — including safe mode and
  // watchdog-reset boots, which are exactly the boots that need it most (a
  // safe-mode unit already has hardware trouble, and the old
  // !g_watchdogReset && !g_safeModeEnabled guard left both with no recovery
  // from a wedged main loop other than unplugging). Only a run of
  // CONF_WATCHDOG_BOOT_RETRY_LIMIT consecutive watchdog resets stops
  // re-arming, so a genuinely wedged device settles in safe mode instead of
  // resetting forever.
  if (CONF_ENABLE_WATCHDOG && g_watchdogBootCount < CONF_WATCHDOG_BOOT_RETRY_LIMIT) { 
    initWatchdog();  // Initialize hardware watchdog
    initHangWitness();  // witness v8: the TIMER3 hang witness — converts a loop hang into an SREQ that preserves the trail
    bootTrailStamp(1);  // witness v7: setup complete (the only boot-phase code; deaths are all loop-phase)
  }

  // 4.5.10: the BLE init task starts LAST — after every sensor, screen and
  // timer init has completed on the loop task's stack, so the two never
  // contend for stack and the loop's guarded BLE calls stay inert until the
  // stack is up (needsInitialization is the gate).
  if (g_bleInitPending) {
    Serial.println(F("BOOT: BLE task create"));
    xTaskCreateStatic(bleInitTaskBody, "bleInit",
                      CONF_BLE_INIT_TASK_STACK_BYTES / sizeof(StackType_t),
                      NULL, CONF_BLE_INIT_TASK_PRIORITY,
                      g_bleInitStack, &g_bleInitTCB);
  }

  if (USB_DEBUG) { Serial.print("USBDEBUG: lastRebootReason: "); Serial.println(g_lastRebootReason); }
  if (USB_DEBUG) { Serial.println("USBDEBUG: Setup complete."); }

  Serial.println(F("BOOT: setup done"));

}  // end setup


//***START OF MAIN LOOP***//
// Function   : loop
//
// Description: This function loops consecutively and responses to changes.
//
// Parameters :  void
//
// Return     : void
//*********************************//
void loop() {
  // 4.5.10: any host byte stops the repeating boot header (below) — the
  // app's connect sends SETTINGS within milliseconds of opening the port,
  // so a connected session falls silent after one extra line at most.
  static bool finalBootLinePrinted = false;
  if (Serial.available() > 0) {
    // 4.5.12: the FIRST host byte also prints the header one final time —
    // the repeating ticks all fall inside TinyUSB's DTR-gated discard
    // window (writes before the host opens the port are dropped), which
    // is how three BLE boots' worth of st= data was lost. This line lands
    // in every session log right after connect, carrying the BLE verdict.
    if (!g_hostHasSpoken && !finalBootLinePrinted) {
      finalBootLinePrinted = true;
      int bleStateFinal = 0;
      if (g_bleInitPending) {
        bleStateFinal = g_bleInitDone ? (needsInitialization ? 3 : 2) : 1;
      }
      Serial.print(F("BOOT: "));
      Serial.print(CONF_LIPSYNC_VERSION_STRING);
      Serial.print(F(" rst=0x"));
      Serial.print(g_lastRebootReason, HEX);
      Serial.print(F(" pt="));
      Serial.print(g_bootTrailPrev);
      Serial.print(F(" pm="));
      Serial.print(g_hangMarkerPrev);
      Serial.print(F(" ble="));
      Serial.print(bleStateFinal);
      Serial.print(F(" st="));
      Serial.print(g_bleInitStage);
      Serial.print(F(" sd="));
      Serial.print(g_bleSdWasEnabled);  // -1 = probe not run (init still in flight), 0 = SD was down, 1 = SD was ALREADY enabled (bootloader leftover)
      Serial.print(F(" pstep="));
      Serial.print(g_bleProbeStep);
      Serial.print(F(" pcode=0x"));
      Serial.print(g_bleProbeCode, HEX);
      Serial.print(F(" need=0x"));
      Serial.print(g_bleProbeNeed, HEX);
      Serial.print(F(" bad1=0x"));
      Serial.print(g_bleIrqBad1, HEX);
      Serial.print(F(" bad2=0x"));
      Serial.print(g_bleIrqBad2, HEX);
      Serial.print(F(" dem="));
      Serial.print(g_bleProbeDemoted);
      Serial.print(F(" rc=0x"));
      Serial.print(g_bleInitRc, HEX);
      Serial.print(F(" pst="));
      Serial.print(g_blePrevSt);  // 4.5.20: the PREVIOUS BLE boot's persisted verdict (-1 = none, 0 = died during begin, 1..4 = stage failure, 5 = init succeeded)
      Serial.print(F(" prc=0x"));
      Serial.println(g_blePrevRc, HEX);
    }
    g_hostHasSpoken = true;
  }

  // REPEATING BOOT HEADER (4.5.10 bench fix): the one-shot boot trace was
  // proven insufficient — the app opens the port AFTER the boot, and CDC
  // output before an open port is discarded, so three sessions of BLE-boot
  // deaths left the wire silent. The header now repeats every second for
  // the first 30 s of the boot (or until the host speaks), carrying the
  // reset reason and the LIVE BLE task state — any serial monitor opened
  // mid-fault-loop captures the state within one second, and the last line
  // before a death names the fault's neighborhood.
  // 4.5.11's deferred BLE-boot attach (wait for the task verdict, or a
  // 10 s fallback) is RETIRED as of 4.5.18 — beginComOpMode attaches at
  // the stock point, before the SoftDevice comes up (see its comment).

  static unsigned long lastBootHeaderMs = 0;
  if (!g_hostHasSpoken && millis() < 30000UL && (millis() - lastBootHeaderMs >= 1000UL)) {
    lastBootHeaderMs = millis();
    int bleState = 0;  // 0 = not a BLE boot
    if (g_bleInitPending) {
      if (g_bleInitDone) {
        bleState = needsInitialization ? 3 : 2;  // 3 = failed, 2 = up
      } else {
        bleState = 1;  // task running
      }
    }
    Serial.print(F("BOOT: "));
    Serial.print(CONF_LIPSYNC_VERSION_STRING);
    Serial.print(F(" rst=0x"));
    Serial.print(g_lastRebootReason, HEX);
    Serial.print(F(" pt="));
    Serial.print(g_bootTrailPrev);
    Serial.print(F(" pm="));
    Serial.print(g_hangMarkerPrev);
    Serial.print(F(" ble="));
    Serial.print(bleState);
    Serial.print(F(" st="));
    Serial.print(g_bleInitStage);  // 1=SD enable 2=DIS 3=HID 4=advertising 5=success
    Serial.print(F(" sd="));
    Serial.print(g_bleSdWasEnabled);  // -1 = probe not run yet, 0 = SD was down, 1 = SD was already up (bootloader leftover)
    Serial.print(F(" pstep="));
    Serial.print(g_bleProbeStep);     // build shape: 11 = probe-mirror build (passed), 12 = direct-begin build (4.5.18+), 0/1..10 = historical
    Serial.print(F(" pcode=0x"));
    Serial.print(g_bleProbeCode, HEX);
    Serial.print(F(" need=0x"));
    Serial.print(g_bleProbeNeed, HEX);
    Serial.print(F(" bad1=0x"));
    Serial.print(g_bleIrqBad1, HEX);   // 4.5.15: enabled IRQs 0-31 found at an SD-reserved priority at init time (bit = IRQ)
    Serial.print(F(" bad2=0x"));
    Serial.print(g_bleIrqBad2, HEX);   // 4.5.15: IRQs 32-47 of the same sweep
    Serial.print(F(" dem="));
    Serial.print(g_bleProbeDemoted);
    Serial.print(F(" rc=0x"));
    Serial.print(g_bleInitRc, HEX);
    Serial.print(F(" pst="));
    Serial.print(g_blePrevSt);  // 4.5.20: the previous BLE boot's persisted verdict
    Serial.print(F(" prc=0x"));
    Serial.println(g_blePrevRc, HEX);
  }

  // Feed the hardware watchdog directly from the live loop (bench
  // 2026-10-18, the unbreakable safe-mode reset cycle): the heartbeat
  // witness proved that in a safe-mode boot the feed's soft timer on the
  // shared pollTimer pool stops firing — feed= froze at 4428 ms while
  // mode=3 safe=1 wdt=1 and the loop printed heartbearts for another
  // minute — so the armed 30 s watchdog starved, reset the device, whose
  // watchdog flag booted it into safe mode again, where the feed died
  // again: a 30 s reset cycle unbreakable until the cable was re-seated.
  // The loop itself demonstrably stays alive, so feed here on every pass:
  // the feed no longer depends on any timer machinery, and the watchdog
  // keeps its true job — catching a genuinely dead loop. The 5 s
  // pollTimer registration stays as a harmless second feeder.
  if (g_watchdogArmed) {
    watchdogLoop();
  }

  // Degraded-state heartbeat (bench 2026-10-18, boot-with-unseated-cable
  // forensics): a 4.5.0 boot with the interface cable unseated reset at an
  // exact 30 s cadence — the app connected into the window, loaded ~19
  // settings, then the device vanished ("connection lost"), cycle after
  // cycle, unrecoverable until the cable was re-seated. Every static
  // suspect is ruled out (the vendored Wire bounds every transaction at
  // 50 ms and recovers wedges; the joystick/pressure/screen paths back
  // off; the safe-mode path leaves the watchdog timer enabled; the API
  // table is not filtered by safe mode) — so the freeze mechanism needs a
  // witness that survives it. A degraded or safe-mode boot prints one line
  // every 5 s; a healthy boot prints nothing. The line's very existence
  // proves the main loop is alive, and the three numbers split the
  // remaining hypotheses: feed= growing past 30 s while lines keep
  // printing = the feed timer died while the loop lives; lines stopping =
  // the loop froze; wdt=1 feed=never = the watchdog was armed but its
  // timer never registered (the LSTimer pool has shown "No free timers").
  static unsigned long lastHeartbeatMs = 0;
  // 4.5.12: a BLE boot whose init FAILED is degraded — the heartbeat now
  // carries the verdict (ble=/st=) for the whole session, so a connected
  // app sees the failure state repeatedly instead of a silent unpairable
  // device. A successful BLE boot (needsInitialization false) is healthy.
  const bool bleInitFailed = (g_bleInitPending && g_bleInitDone && needsInitialization);
  const bool degradedNow = g_safeModeEnabled || !g_displayConnected || !g_joystickSensorConnected
                         || !g_mouthpiecePressureSensorConnected || !g_ambientPressureSensorConnected
                         || bleInitFailed;
  // RELEASED BEHAVIOR (4.5.7 review): the bench's 2026-10-23 "witness
  // window" (every boot printing at 250 ms/1 s cadence for the first 90 s)
  // shipped in the V4.5.6 tag even though its own comment called it "a
  // diagnostic patch, not the release behavior" and CHANGES.txt claimed
  // "healthy boots still print nothing" — ~105 unsolicited HB lines
  // interleaved into every app connect + settings load, the exact
  // wire-noise class 4.5.2/4.5.5 eliminated. The released contract stands
  // again: degraded/safe boots print one line every 5 s (the freeze-hunt
  // forensics endpoint — rst/pt/pm/trail/bus/recov/sns all ride the line),
  // healthy boots print nothing. Bench instrumentation belongs in bench
  // builds, not the tag.
  const unsigned long heartbeatIntervalMs = 5000;
  if (degradedNow && (millis() - lastHeartbeatMs >= heartbeatIntervalMs)) {
    lastHeartbeatMs = millis();
    Serial.print(F("HB: ms="));
    Serial.print(millis());
    Serial.print(F(" mode="));
    Serial.print(g_operatingMode);
    Serial.print(F(" safe="));
    Serial.print(g_safeModeEnabled ? 1 : 0);
    Serial.print(F(" wdt="));
    Serial.print(g_watchdogArmed ? 1 : 0);
    Serial.print(F(" feed="));
    if (g_watchdogLastFeedMs == 0) {
      Serial.print(F("never"));
    } else {
      Serial.print(millis() - g_watchdogLastFeedMs);
      Serial.print(F("ms"));
    }
    Serial.print(F(" rst=0x"));
    Serial.print(g_lastRebootReason, HEX);
    Serial.print(F(" pt="));
    Serial.print(g_bootTrailPrev);
    Serial.print(F(" pm="));
    Serial.print(g_hangMarkerPrev);
    Serial.print(F(" trail="));
    Serial.print(readTrailByte());
    Serial.print(F(" bus="));
    Serial.print(Wire.busHealthy() ? 1 : 0);
    Serial.print(F(" recov="));
    Serial.print(Wire.recoveryCount());

    Serial.print(F(" sns="));
    Serial.print(g_joystickSensorConnected ? 1 : 0);
    Serial.print(g_mouthpiecePressureSensorConnected ? 1 : 0);
    Serial.print(g_ambientPressureSensorConnected ? 1 : 0);
    Serial.print(g_displayConnected ? 1 : 0);
    if (bleInitFailed) {
      Serial.print(F(" ble=3 st="));
      Serial.print(g_bleInitStage);
      Serial.print(F(" sd="));
      Serial.print(g_bleSdWasEnabled);
      Serial.print(F(" pstep="));
      Serial.print(g_bleProbeStep);
      Serial.print(F(" pcode=0x"));
      Serial.print(g_bleProbeCode, HEX);
      Serial.print(F(" need=0x"));
      Serial.print(g_bleProbeNeed, HEX);
      Serial.print(F(" bad1=0x"));
      Serial.print(g_bleIrqBad1, HEX);
      Serial.print(F(" bad2=0x"));
      Serial.print(g_bleIrqBad2, HEX);
      Serial.print(F(" dem="));
      Serial.print(g_bleProbeDemoted);
      Serial.print(F(" rc=0x"));
      Serial.print(g_bleInitRc, HEX);
      Serial.print(F(" pst="));
      Serial.print(g_blePrevSt);
      Serial.print(F(" prc=0x"));
      Serial.println(g_blePrevRc, HEX);
    } else {
      Serial.println();
    }
  }

  // 4.5.21: the verdict-persist service point — the BLE init task raises the
  // flag, the loop (sole owner of all filesystem access) performs the write.
  // Runs within milliseconds of the flag, well before begin() completes.
  if (g_bleVerdictDirty) {
    g_bleVerdictDirty = false;
    bleVerdictPersist();
  }

  ledStateTimer.run();  // Timer for lights
  
  usbConnectTimer.run();
  

  if (g_joystickSensorConnected) {
    calibrationTimer.run();  // Timer for calibration measurements
  }
  

  if (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) {
    actionTimer.run();  
  }

  pollTimer.run();  // Timer for normal joystick functions

  buzzer.update();  // Fire queued buzzer notes (non-blocking sound sequencing)

  settingsEnabled = serialSettings(settingsEnabled);  // Process Serial API commands
  //yield();
}


//***INITIALIZE GLOBALS FUNCTION***//
// Function   : initGlobals
//
// Description: This function initializes global variables to default values
//
// Parameters : void
//
// Return     : void
//****************************************//
void initGlobals() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initGlobals()"); } //  Won't display as function called before serial established

  g_lastRebootReason = 0;
  g_ambientPressureSensorConnected = false;
  g_joystickSensorConnected = false;
  g_displayConnected = false;
  g_btIsConnected = false;
  g_errorCode = 0;
  g_debugMode = 0;
  g_usbNotReadySinceMs = 0;
  g_usbReenumCount = 0;
  //g_lightMode = 1;  //  Currently unused
  //g_soundMode = 1; //  Currently unused
  g_operatingMode = 1;
  g_comMode = 1;
  //g_firstLoop = true;

  g_watchdogReset = false;
  g_watchdogBootCount = 0;
  g_safeModeEnabled = false;
  g_safeModeReason = 0;

  g_sapDisabled = false;
  g_menuButtonsOnly = false;
  g_modeLocked = false;
  g_freezeCursorDuringSap = false;

  g_inputWindowShortMs = 0;
  g_inputWindowLongMs = 0;
  g_inputWindowMaxMs = 0;

}


//***HARDWARE ERROR CHECK FUNCTION***//
// Function   : hardwareErrorCheck
//
// Description: This function checks for errors with the LipSync hardware
//
// Parameters : void
//
// Return     : void
//****************************************//
void hardwareErrorCheck(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: hardwareErrorCheck()"); }

  // I2C Connection Errors (Display, Joystick, Mouthpiece Pressure, Ambient Pressure)
  if (!g_displayConnected) {
    // Display not connected / detected
    buzzer.playErrorSound();
    Serial.println("ERROR: Display not detected");
    ledError(3);
    g_safeModeEnabled = true;
    g_safeModeReason = CONF_SAFE_MODE_REASON_HARDWARE;
  }

  if (!g_joystickSensorConnected && !g_mouthpiecePressureSensorConnected && !g_ambientPressureSensorConnected) {
    // All joystick sensors not detected
    buzzer.playErrorSound();
    Serial.println("ERROR: No sensors detected in joystick. Check interface cable.");
    g_safeModeEnabled = true;
    g_safeModeReason = CONF_SAFE_MODE_REASON_HARDWARE;
    
  } else if (!g_joystickSensorConnected || !g_mouthpiecePressureSensorConnected || !g_ambientPressureSensorConnected) {
    // One or more sensors but not all
    
    g_safeModeEnabled = true;
    g_safeModeReason = CONF_SAFE_MODE_REASON_HARDWARE;
    
    buzzer.playErrorSound();
    if (!g_joystickSensorConnected) {
      Serial.println("ERROR: Joystick sensor not detected.");
    }
    if (!g_mouthpiecePressureSensorConnected) {
      Serial.println("ERROR: Mouthpiece Pressure Sensor not detected.");
    }
    if (!g_ambientPressureSensorConnected) {
      Serial.println("ERROR: Ambient Pressure Sensor not detected.");
    }
    
  }


}


//***HANDLE HARDWARE ERRORS ***//
// Function   : handleHardwareErrors
//
// Description: This function handles hardware errors
//
// Parameters : void
//
// Return     : void
//****************************************//
void handleHardwareErrors(void) {
  
}


//***ERROR CHECK FUNCTION***//
// Function   : errorCheck
//
// Description: This function checks for errors with the LipSync device
//
// Parameters : void
//
// Return     : void
//****************************************//
void errorCheck(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: errorCheck()"); }

  if ((g_operatingMode == CONF_OPERATING_MODE_MOUSE) && (g_comMode == CONF_COM_MODE_USB)
      && (!usbmouse.isReady() || usbmouse.usbRetrying || usbmouse.timedOut)) {
    g_errorCode = CONF_ERROR_USB;
  } else if ((g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) && !gamepad.isReady()) {
    g_errorCode = CONF_ERROR_USB;
  } else {
    g_errorCode = CONF_ERROR_NONE;  // 0
  }

  if (USB_DEBUG) { Serial.print("USBDEBUG: errorCode: "); Serial.println(g_errorCode); }
  // Add if cases for other errors
}


//***ERROR SCREEN FUNCTION***//
// Function   : errorScreen
//
// Description: This function calls the corresponding error screen function
//
// Parameters : void
//
// Return     : void
//****************************************//
void errorScreen(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: errorScreen()"); }
  errorCheck();

  if (!screen.isMenuActive()) {
    if (g_errorCode == CONF_ERROR_USB) {
      // Only draw when there is a display on an answerable bus — a draw against
      // held I2C hardware costs seconds of bounded timeouts (see usbCheckConnection)
      if (g_displayConnected && Wire.busHealthy()) {
        screen.noUsbPage();
      }
    }
  }
}


//***CHECK SAFE MODE FUNCTION***//
// Function   : checkSafeMode
//
// Description: Checks if both hub buttons are pressed on startup in order to enter safe boot mode.
//
// Parameters : void
//
// Return     : void
//****************************************//
void checkSafeMode(void) {

  if (USB_DEBUG) { Serial.println("USBDEBUG: checkSafeMode()"); }

  // Read Hub button values
  int buttonSelectState1 = !digitalRead(CONF_BUTTON1_PIN);
  int buttonNextState1 =   !digitalRead(CONF_BUTTON2_PIN);

  delay(50);

  // Read Hub button values again
  int buttonSelectState2 = !digitalRead(CONF_BUTTON1_PIN);
  int buttonNextState2 =   !digitalRead(CONF_BUTTON2_PIN);

  if (buttonSelectState1 == HIGH && buttonSelectState2 == HIGH && buttonNextState1 == HIGH && buttonNextState2 == HIGH) {
    g_safeModeEnabled = true;
    g_safeModeReason = CONF_SAFE_MODE_REASON_INPUT;  //  Both hub buttons pushed on startup

  } else if (g_watchdogReset && CONF_ENABLE_WATCHDOG) {
    g_safeModeEnabled = true;
    g_safeModeReason = CONF_SAFE_MODE_REASON_WATCHDOG;

  } else if (g_hangBootStreak >= CONF_WATCHDOG_BOOT_RETRY_LIMIT) {
    // Hang-witness settling (4.5.7 review): the previous three-or-more
    // consecutive boots each died in a TIMER3-witnessed loop hang (an
    // interrupts-alive stall — the SREQ resets PRESERVE the retention
    // register, so unlike the watchdog-reset budget this streak actually
    // accumulates). Rest this boot in safe mode: the safe-mode loop is the
    // minimal machinery, and the watchdog/witness are not re-armed (the
    // streak feeds the existing g_watchdogBootCount gate in setup()) — the
    // documented "after N straight crash boots, the device settles in
    // safe mode instead of resetting forever" contract, finally
    // functional on the path that can actually count.
    g_safeModeEnabled = true;
    g_safeModeReason = CONF_SAFE_MODE_REASON_HANG;
    Serial.println("WARNING: SAFE MODE ENABLED - Repeated witnessed loop hangs");

  } else {
    g_safeModeEnabled = false;
  }

  if (g_safeModeEnabled) {
    g_operatingMode = CONF_OPERATING_MODE_SAFE; // Change operating mode to safe mode
  }


}

//***TOGGLE SAFE MODE FUNCTION***//
// Function   : toggleSafeMode
//
// Description: Checks if both hub buttons are pressed on startup in order to enter safe boot mode.
//
// Parameters : bool : safeModeEnabled : Whether safe mode is enabled or disabled
//
// Return     : void
//****************************************//
void toggleSafeMode(bool safeModeEnabled) {
  
  if (safeModeEnabled) {
    // Prevent mouse and gamepad outputs from being sent
    
    // Provide feedback based on reason for activating safe mode
    switch(g_safeModeReason) {
      case CONF_SAFE_MODE_REASON_INPUT:
      {
        Serial.println("WARNING: SAFE MODE ENABLED - Hub Buttons"); 
        break;
      }
      case CONF_SAFE_MODE_REASON_WATCHDOG:
      {
        Serial.println("WARNING: SAFE MODE ENABLED - Watchdog"); 
        break;
      }
      case CONF_SAFE_MODE_REASON_HARDWARE:
      {
        Serial.println("WARNING: SAFE MODE ENABLED - Hardware");
        break;
      }
      default:
      {
        Serial.println("WARNING: SAFE MODE ENABLED"); 
        break;
      }
         
    }

    g_operatingMode = CONF_OPERATING_MODE_SAFE; // Set working operating mode to safe mode to prevent unwanted input in case of hardware error

    // activate safe boot mode screen on display
    screen.safeModePage(g_safeModeReason);
    screen.disableTimeout();
    screen.update();

      
    // Disable poll timers
    //pollTimer.disable(pollTimerId[CONF_TIMER_SCREEN]);
    pollTimer.disable(pollTimerId[CONF_TIMER_PRESSURE]);
    pollTimer.disable(pollTimerId[CONF_TIMER_JOYSTICK]);
    pollTimer.disable(pollTimerId[CONF_TIMER_SCROLL]);
    pollTimer.disable(pollTimerId[CONF_TIMER_DEBUG]);
    pollTimer.disable(pollTimerId[CONF_TIMER_BLUETOOTH]);

    } else {
    

  }


}



//***READY TO USE FUNCTION***//
// Function   : readyToUse
//
// Description: This function checks for errors and startup conditions, and then displays the Ready To Use screen.
//              If errors are present, the error screen function is called.
//
// Parameters : void
//
// Return     : void
//****************************************//
void readyToUse(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: readyToUse()"); }

  errorCheck();  // Check for errors

  if (readyToUseFirstTime && g_resetCenterComplete && !g_safeModeEnabled) {

    if (g_errorCode == CONF_ERROR_NONE) {
      buzzer.playReadySound();
      screen.splashScreen2();
      readyToUseFirstTime = false;
    } else {
      buzzer.playErrorSound();
      errorScreen();
    }

    if (SHOW_CONNECTION_TIME) {
      screen.connectionTimingPage(beforeComOpMillis, afterComOpMillis);
      Serial.print("Time until before com op mode: ");
      Serial.println(beforeComOpMillis);
      Serial.print("Time until after com op mode: ");
      Serial.println(afterComOpMillis);
    }
  }
}

//***ENABLE POLL FUNCTION***//
// Function   : enablePoll
//
// Description: This function enables polling by enabling poll timers. It can be used to disable main features
//              during initialization.
//
// Parameters : isEnabled : bool : enable polling if it's True
//
// Return     : void
//****************************************//
void enablePoll(bool isEnabled) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: enablePoll()"); }

  if (isEnabled) {
    getDebugMode(false, false);
    pollTimer.enable(pollTimerId[CONF_TIMER_PRESSURE]);
    pollTimer.enable(pollTimerId[CONF_TIMER_INPUT]);
    pollTimer.enable(pollTimerId[CONF_TIMER_BLUETOOTH]);
  } else {
    pollTimer.disable(pollTimerId[CONF_TIMER_JOYSTICK]);
    pollTimer.disable(pollTimerId[CONF_TIMER_PRESSURE]);
    pollTimer.disable(pollTimerId[CONF_TIMER_INPUT]);
    pollTimer.disable(pollTimerId[CONF_TIMER_BLUETOOTH]);
    pollTimer.disable(pollTimerId[CONF_TIMER_DEBUG]);
    pollTimer.disable(pollTimerId[CONF_TIMER_SCROLL]);
  }
}

//*********************************//
// Memory Functions
//*********************************//

//***INITIALIZE MEMORY FUNCTION***//
// Function   : initMemory
//
// Description: This function initializes flash memory to store settings
//
// Parameters : void
//
// Return     : void
//****************************************//
void initMemory() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initMemory()"); }
#if CONF_FS_FORMAT_ON_BOOT
  // 4.5.22 RESCUE PATH (see LSConfig.h): a fresh format before the mount
  // heals a filesystem torn by the 4.5.20 race + mid-write witness kills —
  // the class of corruption that hangs/crash-loops the boot at the FIRST FS
  // operation, before any tone, screen, or serial protocol. Runs on the
  // RECOVER build only; mem.begin() below mounts the fresh filesystem.
  InternalFS.format();
  delay(2);
#endif
  mem.begin();  // Begin memory
  bleVerdictConsume();  // 4.5.20: consume the previous BLE boot's flash-persisted verdict (pst=/prc= on the wire) — after the FS is up, before any header prints it
  
  // Open settings file (CONF_SETTINGS_FILE) from flash memory.
  // If not present, create file based on defaults in CONF_SETTINGS_JSON
  mem.initialize(CONF_SETTINGS_FILE, CONF_SETTINGS_JSON);  
}

//***RESET MEMORY FUNCTION***//
// Function   : resetMemory
//
// Description: This function formats and removes existing text files in flash memory.
//              It initializes flash memory to store settings after formatting.
//
// Parameters : void
//
// Return     : void
//****************************************//
void resetMemory() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: resetMemory()"); }
  mem.format();                                            // Format and remove existing text files in flash memory
  mem.initialize(CONF_SETTINGS_FILE, CONF_SETTINGS_JSON);  // Initialize flash memory to store settings
}

//***Read UID FUNCTION***//
// Function   : readUID
//
// Description: This function reads the device's unique ID from the microcontroller
//
// Parameters : void
//
// Return     : const char* : deviceIDString : 8 hexadecimal string UID from board
//****************************************//
const char* readUID() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: readUID()"); }

  static char deviceIDString[9];
  uint32_t deviceID = 0;

  //#ifdef NRF52480_XXAA
  deviceID = NRF_FICR->DEVICEID[0];  // First part of Device ID for NRF52840 boards
  //#endif

  itoa(deviceID, deviceIDString, 16);  // Convert uint32_t to hex string

  return deviceIDString;
}


//*********************************//
// Screen Functions
//*********************************//

//***INITIALIZE SCREEN FUNCTION***//
// Function   : initScreen
//
// Description: This function initializes the screen
//
// Parameters : void
//
// Return     : void
//****************************************//
void initScreen() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initScreen()"); }
  screen.begin();         // Begin screen module
  //screen.splashScreen();  // Show splash screen
}

// (4.5.5 audit follow-up) The free closeMenu()/clearSplashScreen() wrappers
// that used to live here are gone: closeMenu had no callers (the serial
// CH,1:4 close and the STOP_MENU input action now call the freeze-aware
// LSScreen::closeMenu), and clearSplashScreen's body moved into
// LSScreen::finishSplashClear — a member so the continuation can reset
// _screenStateTimerId when it fires (the free function could not, which
// left the id naming a freed pool slot).

//***SCREEN TRANSITION CONTINUATION FUNCTIONS***//
// Free-function continuations for LSScreen::scheduleTransition() — the
// non-blocking replacements for the delay() calls that used to freeze the
// main loop while informational pages were shown. The screen timer calls
// these when a page's dwell time ends.
//
// Function   : screen transition continuations
//
// Description: Each hands the follow-up action to the matching LSScreen
//              method, which also clears the pending-transition state.
//
// Parameters : void
//
// Return     : void
//****************************************//
void screenExitTransitionDone() {
  screen.finishExitMenu();
}

void screenScrollTransitionDone() {
  screen.beginLongTextScroll();
}

void screenChangeModeTransitionDone() {
  screen.finishChangeMode();
}

void screenReturnToMenuTransitionDone() {
  screen.returnToMenu();
}

void screenFullCalibConfirmTransitionDone() {
  screen.fullCalibrationConfirmMenu();
}

void screenFactoryResetConfirm2TransitionDone() {
  screen.factoryResetConfirm2Menu();
}

void screenModeLockedTransitionDone() {
  screen.finishModeLocked();
}

//***SHOW CENTER RESET COMPLETE PAGE SCREEN FUNCTION***//
// Function   : showCenterResetComplete
//
// Description: This function shows the center reset complete page
//
// Parameters : void
//
// Return     : void
//****************************************//
void showCenterResetComplete() {
  // Show center reset complete page once center reset is done
  screen.centerResetCompletePage();
}

//***SCREEN LOOP FUNCTION***//
// Function   : screenLoop
//
// Description: This function handles updates to the screen.
//
// Parameters : void
//
// Return     : void
//****************************************//
void screenLoop() {
  //if (USB_DEBUG) { Serial.println("USBDEBUG: screenLoop"); }

  screen.update();  // Update the menu and screen

  //if (USB_DEBUG) { Serial.println("USBDEBUG: end of screenLoop"); }
}

//*********************************//
// Buzzer Functions
//*********************************//

//***INITIALIZE BUZZER FUNCTION***//
// Function   : initBuzzer
//
// Description: This function initializes the Buzzer as feedback method.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initBuzzer() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: Initializing Buzzer"); }

  buzzer.begin();             // Initialize buzzer for sound feedback
  buzzer.playStartupSound();  // Play a sound to indicate power-up
}

//***BUZZER LOOP FUNCTION***//
// Function   : buzzerLoop
//
// Description: This function handles updates to the buzzer.
//
// Parameters : void
//
// Return     : void
//****************************************//
void buzzerLoop() {
  //if (USB_DEBUG) { Serial.println("USBDEBUG: buzzerLoop"); }

  // Request update
  buzzer.update();
}

//***BUZZER SOUND ON FUNCTION***//
// Function   : buzzerSoundOn
//
// Description: This function turns on the sound from the buzzer by calling the buzzer enable function.
//
// Parameters : void
//
// Return     : void
//****************************************//
void buzzerSoundOn() {
  buzzer.enable();
}


//***BUZZER SOUND OFF FUNCTION***//
// Function   : buzzerSoundOff
//
// Description: This function turns off the sound from the buzzer.
//
// Parameters : void
//
// Return     : void
//****************************************//
void buzzerSoundOff() {
  buzzer.disable();
}

//***BUZZER ERROR CUE FUNCTION***//
// Function : buzzerErrorCue
//
// Description: One short, non-blocking error tone for refused interactions
//              (e.g. selecting a mode while the mode lock is on, roadmap
//              item 11) — "that input is locked", not "the device is
//              broken". Forwards to LSBuzzer::playErrorCue (4.5.7 review:
//              this used to call the ~1 s blocking playErrorSound, freezing
//              the whole loop on every refused press).
//
// Parameters : void
//
// Return : void
//****************************************//
void buzzerErrorCue() {
  buzzer.playErrorCue();
}


//*********************************//
// Acceleration Functions
//*********************************//

//***INITIALIZE ACCELERATION FUNCTION***//
// Function   : initAcceleration
//
// Description: This function loads the stored cursor acceleration level into
//              the live global the joystick output path reads (roadmap item
//              9 — the level is a stored setting since v4.0; the output
//              curve now actually applies it).
//
// Parameters : void
//
// Return     : void
//****************************************//
void initAcceleration() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initAcceleration()"); }
  acceleration = getJoystickAcceleration(false, false);
}

//*********************************//
// Communication Mode Functions
//*********************************//

//***INITIALIZE COMMUNICATION FUNCTION***//
// Function   : initCommunicationMode
//
// Description: This function initializes communication mode or configures communication mode
//              based on stored settings in the flash memory (0 = None , 1 = USB , 2 = Wireless)
//
// Parameters : void
//
// Return     : void
//****************************************//
void initCommunicationMode() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initCommunicationMode()"); }

  g_comMode = getCommunicationMode(false, false);

  if (USB_DEBUG) {
    Serial.print("USBDEBUG: g_comMode: ");
    Serial.println(g_comMode);
  }
}


//***USB CONNECTION LOOP FUNCTION***//
// Function   : usbConnectionLoop
//
// Description: This function checks status of USB connection
//
// Parameters : void
//
// Return     : void
//****************************************//
void usbConnectionLoop() {
  // Parked since 2025-Feb-21 (upstream TODO: "Disable usbConnectionLoop
  // until implemented"): the USB connection state it computed had no
  // consumer — errorCheck/usbCheckConnection read usbmouse.isReady()
  // directly — and the write-only global it fed is gone. The registration
  // below stays: the boot pool's nine registrations (including this one)
  // are what the app-repo lstimer-test harness models, and the slot stays
  // reserved for the feature. Disabled at boot; never runs.
}


//***USB CHECK CONNECTION FUNCTION***//
// Function   : usbCheckConnection
//
// Description: This function checks if the USB connection is attempting to retry mounting, not ready, or timed out
//              In this case an error screen is shown, and the function is called again after a set time.
//              If the USB connection has not been made, it calls another instance of usb.begin.
//
// Parameters : void
//
// Return     : void
//****************************************//
void usbCheckConnection(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: usbCheckConnection()"); }
  bootTrailStamp(6);  // witness v7: the USB connection check is running

  // Self-healing re-enumeration (bench 2026-10-16, second stuck-boot capture):
  // twice now a post-flash 4.5.0 boot came up CONFIGURED but permanently
  // HID-less — serial alive (the host completed SET_CONFIGURATION; TinyUSB's
  // tud_mounted() is cfg_num != 0 in this stack), every mouse report refused
  // (attempts climbing, successes 0) until a manual restart, and the
  // detach-until-descriptor-complete boot order did not prevent it. Whatever
  // the host's exact misstep (locking in a HID-less configuration, or wedging
  // the interrupt endpoint), it does not recover on its own, so the device
  // must force the issue: after CONF_USB_REENUM_AFTER_MS of continuous
  // not-ready, drop and re-raise the bus once — a fresh enumeration rebuilds
  // both the host's configuration and TinyUSB's endpoint state. Bounded at
  // CONF_USB_REENUM_MAX per episode, tracked only in the modes where a USB
  // HID interface is expected (BLE mouse and safe mode legitimately have
  // none). The detach drops the CDC session with it — but a device in this
  // state is already unusable, and on the post-flash boots where it strikes
  // the cycle lands within seconds of boot, before the app reconnects.
  bool usbHidExpected = ((g_operatingMode == CONF_OPERATING_MODE_MOUSE) && (g_comMode == CONF_COM_MODE_USB))
                     || (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD);
  if (usbHidExpected) {
    bool hidReadyNow = ((g_operatingMode == CONF_OPERATING_MODE_MOUSE) && usbmouse.isReady())
                    || ((g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) && gamepad.isReady());
    if (hidReadyNow) {
      if (g_usbReenumCount > 0) {
        Serial.print("USB recovered after forced re-enumeration #");
        Serial.println(g_usbReenumCount);
      }
      g_usbNotReadySinceMs = 0;  // Episode over
      g_usbReenumCount = 0;
      // The once-guard now lives in the reset reason itself: this healthy
      // boot will end in whatever reset ends it, and the auto-restart only
      // ever fires on boots that did NOT begin with a software reset.
    } else {
      if (g_usbNotReadySinceMs == 0) {
        g_usbNotReadySinceMs = millis();  // Episode begins
      } else if ((millis() - g_usbNotReadySinceMs >= CONF_USB_REENUM_AFTER_MS) && (g_usbReenumCount < CONF_USB_REENUM_MAX)) {
        g_usbReenumCount++;
        Serial.print("USB forced re-enumeration #");
        Serial.print(g_usbReenumCount);
        Serial.print(" at ");
        Serial.println(millis());
        USBDevice.detach();
        delay(CONF_USB_REENUM_DWELL_MS);  // Dark long enough for the host to tear the old configuration down
        USBDevice.attach();
        g_usbConnectDelay = CONF_USB_HID_INIT_DELAY;  // Resume fast checks while the fresh enumeration settles
        g_usbNotReadySinceMs = millis();              // Fresh budget for the fresh enumeration
      } else if ((millis() - g_usbNotReadySinceMs >= CONF_USB_REENUM_AFTER_MS) && (g_usbReenumCount >= CONF_USB_REENUM_MAX)
                 && ((g_lastRebootReason & NRF_POWER_RESETREAS_SREQ_MASK) == 0)) {  // never auto-restart a boot that began with a software reset (the once-guard: the platform wipes .noinit, but RESETREAS survives)
        // The re-enumeration budget is spent and the HID interface is still
        // dead while serial lives — the host is not recovering this
        // composite, and no bus-level cycle left will change that (bench
        // 2026-10-17: a wedge survived three forced cycles). A full device
        // restart has cleared every such boot on the bench — fresh USB
        // peripheral, fresh address, seconds of bus darkness — so spend the
        // one auto-restart this power cycle allows. The next boot wins the
        // enumeration the way every restart boot has; if it somehow wedges
        // again, the SREQ once-guard keeps this branch from firing and the
        // page + retry ticks carry on for the app/user to see honestly.
        // (4.5.7 review: the bootTrailStamp(5) that used to live here is
        // GONE — pt=5 is softwareReset's code, and double-booking it made
        // a post-mortem unable to tell a deliberate user/menu reset from
        // this USB-recovery restart. The stamp(6) at this function's entry
        // already names the true death context.)
        Serial.print("USB restarting device to recover the HID interface at ");
        Serial.println(millis());
        screen.restartPage();
        delay(500);  // Bounded: drain the serial line and let the framebuffer push complete (bounded by Wire)
        NVIC_SystemReset();
      }
    }
  } else {
    g_usbNotReadySinceMs = 0;
    g_usbReenumCount = 0;
  }


  if (usbmouse.usbRetrying || gamepad.usbRetrying) {

    if (usbmouse.usbRetrying) {
      Serial.print("Reattempting USB Mouse ");
      Serial.println(millis());

      usbmouse.begin();
    } else if (gamepad.usbRetrying) {
      Serial.print("Reattempting USB Gamepad ");
      Serial.println(millis());

      gamepad.begin();
    }

    screen.drawUsbErrorPageOnce();  // Draw at most once per episode (LSScreen gates display/bus health)

    // increase variable g_usbConnectDelay. Capped at 10 s (bench-observed
    // 2026-10-15): the ×1.2 growth used to run to 120 s to space an EXPENSIVE
    // tick — the old retry branch re-ran begin() and redrew the full
    // framebuffer every time (the 4.2.0 amplifier). Today's tick is cheap
    // (isReady reads, at most one draw), so a long backoff only delays the
    // ready screen behind actual host recovery: after a post-flash boot the
    // first ticks land before the host finishes configuring the composite
    // device, and by the time it is ready the next tick can be ~40 s out —
    // the No-USB page lingers on a device that is already working.
    if (g_usbConnectDelay < 10000) {
      g_usbConnectDelay = g_usbConnectDelay * 1.2;  // Increase time after each unsuccessfull attempt
    }

    usbConnectTimerId[0] = usbConnectTimer.setTimeout(g_usbConnectDelay, usbCheckConnection);  // Keep retrying connection until USB connection is made

  } else if (((g_operatingMode == CONF_OPERATING_MODE_MOUSE) && (g_comMode == CONF_COM_MODE_USB) && (!usbmouse.isReady()))  // in usb mouse mode and usb mouse is not ready
      || ((g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) && !gamepad.isReady())){                                         // in usb gamepad mode and usb gamepad is not ready

    screen.drawUsbErrorPageOnce();

    usbConnectTimerId[0] = usbConnectTimer.setTimeout(g_usbConnectDelay, usbCheckConnection);  // Keep retrying connection until USB connection is made


  } else if (usbmouse.timedOut && !usbmouse.isReady()) {
    // A report was attempted and the interface is genuinely still down —
    // keep the page and keep checking. A latched timedOut with a READY
    // interface (mouse reports suspended in gamepad/debug mode never clear
    // the latch themselves) falls through to the healthy branch instead of
    // parking here forever.
    screen.drawUsbErrorPageOnce();
    usbConnectTimerId[0] = usbConnectTimer.setTimeout(g_usbConnectDelay, usbCheckConnection);  // Keep retrying connection until USB connection is made
  } else {
    usbmouse.timedOut = false;  // Stale latch: the interface is ready, the episode is over
    usbmouse.consecutiveSkips = 0;  // 4.5.7 review: the skip counter must reset with the latch, or the first transient not-ready tick after recovery re-latches after ONE skip instead of three
    gamepad.timedOut = false;

    // Connection healthy. The page clear used to be one-shot — this branch
    // never re-armed — and resetUsbErrorPage cleared the episode latch even
    // when its draw was gated off, so a single tick landing while the bus
    // sat in its post-recovery window stranded the No-USB page on the OLED
    // for the rest of the boot (bench 2026-10-16, twice: stuck until a
    // manual restart on a device whose serial session worked the whole
    // time). The clear now reports success only when the ready page actually
    // landed; while it has not, re-arm and try again. The retry is cheap
    // (a gate check, at most one draw) and ends for good once the page
    // clears or there was never anything painted.
    if (screen.resetUsbErrorPage()) {
      readyToUse();
    } else {
      usbConnectTimerId[0] = usbConnectTimer.setTimeout(1000, usbCheckConnection);
    }
  }
}

//***TINYUSB MOUNT/UNMOUNT WITNESS CALLBACKS***//
// Function   : tud_mount_cb / tud_umount_cb
//
// Description: TinyUSB calls tud_mount_cb once the host has completed
//              SET_CONFIGURATION (in this stack tud_mounted() is cfg_num != 0)
//              and tud_umount_cb when the bus resets away from that state.
//              These hooks print one line per event so an exported session log
//              carries the host's configure/unconfigure timeline — the missing
//              witness in the post-flash stuck-boot forensics (bench
//              2026-10-16: BH showed "configured but HID never ready" with no
//              way to see WHEN the host configured or whether it ever tried
//              twice). extern "C" is required: TinyUSB looks for these as C
//              symbols and the sketch compiles as C++.
//
// Parameters : void
//
// Return     : void
//****************************************//
extern "C" void tud_mount_cb(void) {
  Serial.print("USB-EVT: configured at ");
  Serial.println(millis());
}

extern "C" void tud_umount_cb(void) {
  Serial.print("USB-EVT: unconfigured at ");
  Serial.println(millis());
}



//*********************************//
// Operating Mode Functions
//*********************************//



//***INITIALIZE OPERATING MODE FUNCTION***//
// Function   : initOperatingMode
//
// Description: This function initializes the operating mode from the stored
//              settings in flash memory (1 = Mouse, 2 = Gamepad; SAFE (3) is
//              a runtime-only state and never a valid stored value — see the
//              clamp below)
//
// Parameters : void
//
// Return     : void
//****************************************//
void initOperatingMode() {

  //g_operatingMode = getOperatingMode(false,false);
  g_operatingMode = mem.readInt(CONF_SETTINGS_FILE, "OM"); // retrieve operating mode from memory
  // Boot-load clamp (4.5.7 review): mode 0 has no beginComOpMode case and no
  // sender (the API now rejects it, CONF_OPERATING_MODE_MIN is 1) — but a
  // stored 0 from an old/hand-edited file booted a device whose BLE-poll
  // switch then hammered an uninitialized SoftDevice with a 1 s stall per
  // tick. Out of range falls back to the default mouse mode.
  if ((g_operatingMode < CONF_OPERATING_MODE_MIN) || (g_operatingMode > CONF_OPERATING_MODE_SETTABLE_MAX)) {
    g_operatingMode = CONF_OPERATING_MODE_DEFAULT;  // also catches a stored SAFE(3) — not a valid boot mode (4.5.7 review)
  }

}

//***CHANGE OPERATING MODE FUNCTION***//
// Function   : changeOperatingMode
//
// Description: This function configures the state of operation based on the current and desired operating mode
//
// Parameters : inputOperatingMode : int : The operating mode to change to
//
// Return     : void
//****************************************//
void changeOperatingMode(int inputOperatingState) {
  if (inputOperatingState == g_operatingMode) {
    // do nothing
  } else {
    softwareReset();
  }
  g_operatingMode = inputOperatingState;
}


//***INITIALIZE OPERATING MODE FUNCTION***//
// Function   : beginComOpMode                  // TODO: rename this?
//
// Description: This function calls functions to initialize communication mode and operating mode
//              and begins instance of either USB mouse, Bluetooth mouse, or USB Gamepad
//
// Parameters : void
//
// Return     : void
//****************************************//
void beginComOpMode() {

  // Safe mode comes up CDC-only (4.5.7 review): checkSafeMode() runs BEFORE
  // this function now, so the decision is already made. Attach the bus for
  // serial recovery, but read NEITHER the stored operating mode (which would
  // overwrite the SAFE that checkSafeMode set) NOR the stored communication
  // mode (a stored BLE mode must not initialize the SoftDevice on a safe
  // boot), and begin NO mode interface. usbCheckConnection's episode
  // machinery already excludes safe mode from its HID-expected modes.
  if (g_safeModeEnabled) {
    if (USB_DEBUG) { Serial.println("USBDEBUG: beginComOpMode: Safe Mode (CDC-only)"); }
    USBDevice.attach();
    return;
  }

  initCommunicationMode();  // Retrieve communication mode from memory (None, USB, Bluetooth)
  initOperatingMode();      // Retrieve operating mode from memory (USB Mouse, Bluetooth Mouse, Gamepad)

  Serial.print(F("BOOT: com-op om="));
  Serial.print(g_operatingMode);
  Serial.print(F(" cm="));
  Serial.println(g_comMode);

  // Cross-validate the loaded pair (4.5.7 review): the setters reject
  // gamepad+BLE, but the boot loaders clamp each value to its own range
  // only — a hand-edited/old settings file with OM=2+CM=2 booted a device
  // whose btFeedbackLoop then hammered the UNINITIALIZED SoftDevice for a
  // full second per poll (the bench-documented 1 s stall), and whose USB
  // recovery ladder never armed (it keys on the com mode, and the gamepad
  // paths never call usbCheckConnection themselves). Force the com mode to
  // USB, mirroring the setter contract, and persist the correction so the
  // file heals.
  if ((g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) && (g_comMode == CONF_COM_MODE_BLE)) {
    g_comMode = CONF_COM_MODE_USB;
    mem.writeInt(CONF_SETTINGS_FILE, "CM", CONF_COM_MODE_USB);
    Serial.println("ERROR: stored gamepad+Bluetooth is not a supported combination -- communication mode reset to USB");
  }

  // Bus up only now: setup() detached at the top, so no host could have
  // locked in a CDC-only enumeration during boot. CDC has been registered
  // since Serial.begin(); the mode interface registers microseconds below.
  // A host needs at least a connect-detect + bus-reset cycle (ms) after
  // attach before it can fetch a descriptor, so every enumeration now sees
  // the complete composite.
  // 4.5.9: USB attach moved INTO the mode branches. USB and gamepad boots
  // keep the bench-validated order (attach, then begin — the host sees the
  // complete composite); BLE boots now begin the SoftDevice stack FIRST and
  // attach after — a fresh diagnostic axis for the BLE boot loop, and the
  // ordering the stock BLE-mouse examples use (no USB composite to build
  // beyond CDC on that path).
  switch (g_operatingMode) {
    case CONF_OPERATING_MODE_MOUSE:
      switch (g_comMode) {
        case CONF_COM_MODE_USB:  // USB Mouse
          USBDevice.attach();
          usbmouse.begin();
          break;
        case CONF_COM_MODE_BLE:  // Bluetooth Mouse
          {
            // 4.5.10: the name is copied to a STATIC buffer and the init is
            // DEFERRED to the 16 KB task (started at the end of setup) —
            // the loop task's 4 KB stack must not carry Bluefruit.begin.
            // 4.5.18: the USB attach is RESTORED TO THE STOCK ORDER —
            // enumerated BEFORE the SoftDevice comes up, exactly like the
            // stock blehid_mouse example and 4.1.0. The 4.5.11 deferral
            // was built on a since-RETRACTED claim (the core's USB/SD
            // coexistence hooks DO exist and begin() runs them itself),
            // and it left our boot attaching into a live SoftDevice — a
            // deviation no working recipe performs.
            USBDevice.attach();
            snprintf(g_bleDeviceName, sizeof(g_bleDeviceName), "LS_%s", g_deviceUID.c_str());
            g_bleInitPending = true;
            Serial.println(F("BOOT: BLE deferred to 16KB task"));
          }
          break;
      }
      break;
    case CONF_OPERATING_MODE_GAMEPAD:  // USB Gamepad
      USBDevice.attach();
      gamepad.begin();
      break;
      // default: (SAFE is handled by the early return above)
  }
  Serial.println(F("BOOT: interfaces up"));


  if (USB_DEBUG) {
    Serial.print("USBDEBUG: g_operatingMode: ");
    Serial.println(g_operatingMode);
  }

}  // end beginComOpMode

//*********************************//
// Input Functions
//*********************************//


//***INITIALIZE INPUTS FUNCTION***//
// Function   : initInput
//
// Description: This function initializes inputs including input buttons and input switches.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initInput() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: Initializing Input"); }
  
  // Hub Input Buttons
  ib.begin();                                                                      // Begin input buttons
  buttonActionSize = sizeof(buttonActionProperty) / sizeof(inputActionStruct);     // Size of total available input button actions
  buttonActionMaxTime = getActionMaxTime(buttonActionSize, buttonActionProperty);  // Maximum button action end time

  // Hub External Switch Inputs
  is.begin();                                                                      // Begin input switches
  switchActionSize = sizeof(switchActionProperty) / sizeof(inputActionStruct);     // Size of total available switch input actions
  switchActionMaxTime = getActionMaxTime(switchActionSize, switchActionProperty);  // Maximum switch action end time
  // Seed the live switch action table from the compiled one — remappable
  // cells (roadmap item 11's #20 follow-up) are overlaid by initSwitchActions()
  memcpy(g_switchActionTable, switchActionProperty, sizeof(switchActionProperty));
}

//***INITIALIZE RESTRICTIONS FUNCTION***//
// Function   : initRestrictions
//
// Description: Loads the input restriction settings (roadmap item 11) from
//              flash memory into the live gates. Runs before any input can
//              fire, so a locked-down configuration is in force from the
//              first tick of the input timers.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initRestrictions() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initRestrictions()"); }
  getSapDisabled(false, false);
  getMenuButtonsOnly(false, false);
  getModeLocked(false, false);
  getCursorHoldDuringSap(false, false);
  // Input timing windows (roadmap item 15) load with the same stroke so the
  // first tick of the input timers already sees the user's windows.
  getInputWindowShort(false, false);
  getInputWindowLong(false, false);
  getInputWindowMax(false, false);
  sanitizeInputWindows();
  // External switch action remaps (#20) join the same stroke — the getters
  // apply any stored mapping onto the live table before the first tick.
  initSwitchActions();
}

//***SANITIZE INPUT WINDOWS FUNCTION***//
// Function   : sanitizeInputWindows
//
// Description: Boot-load validation for the input timing windows (4.5.7
//              review): the getters apply whatever the settings file holds
//              with no range or ordering checks — asymmetric with the
//              switch-remap load path, which validates against its
//              allowlist. A hand-edited, corrupted, or foreign file could
//              load inverted windows (a quick puff firing the MAX row) or
//              out-of-range values. An offending stored value falls back to
//              the compiled default (0) IN MEMORY ONLY — the stored value
//              is untouched, and every boot re-sanitizes consistently (the
//              4.5.5 switch-action precedent).
//
// Parameters : void
//
// Return     : void
//****************************************//
void sanitizeInputWindows() {
  unsigned long shortEff = (g_inputWindowShortMs > 0) ? g_inputWindowShortMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
  if (g_inputWindowShortMs > 0
      && (g_inputWindowShortMs < CONF_INPUT_WINDOW_SHORT_MIN_MS || g_inputWindowShortMs > CONF_INPUT_WINDOW_SHORT_MAX_MS
          || shortEff >= ((g_inputWindowLongMs > 0) ? g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS))) {
    g_inputWindowShortMs = 0;
  }
  unsigned long longEff = (g_inputWindowLongMs > 0) ? g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  shortEff = (g_inputWindowShortMs > 0) ? g_inputWindowShortMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
  if (g_inputWindowLongMs > 0
      && (g_inputWindowLongMs < CONF_INPUT_WINDOW_LONG_MIN_MS || g_inputWindowLongMs > CONF_INPUT_WINDOW_LONG_MAX_MS
          || longEff <= shortEff || longEff >= ((g_inputWindowMaxMs > 0) ? g_inputWindowMaxMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS))) {
    g_inputWindowLongMs = 0;
  }
  unsigned long maxEff = (g_inputWindowMaxMs > 0) ? g_inputWindowMaxMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
  longEff = (g_inputWindowLongMs > 0) ? g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  if (g_inputWindowMaxMs > 0
      && (g_inputWindowMaxMs < CONF_INPUT_WINDOW_MAX_MIN_MS || g_inputWindowMaxMs > CONF_INPUT_WINDOW_MAX_MAX_MS
          || maxEff <= longEff)) {
    g_inputWindowMaxMs = 0;
  }
}

//***INITIALIZE SWITCH ACTIONS FUNCTION***//
// Function   : initSwitchActions
//
// Description: Loads the external-switch action remaps (roadmap item 11's
//              #20 follow-up, firmware 4.5.0) from flash memory onto the
//              live switch action table. Reads go through the SA/SB/SC/GA/
//              GB/GC getters so the boot path and the serial API share one
//              apply implementation; a device with no stored mapping keeps
//              the compiled table exactly.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initSwitchActions() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initSwitchActions()"); }
  getSwitchAction(false, false, "SA");
  getSwitchAction(false, false, "SB");
  getSwitchAction(false, false, "SC");
  getSwitchAction(false, false, "GA");
  getSwitchAction(false, false, "GB");
  getSwitchAction(false, false, "GC");
}

//***SWITCH REMAP HELPER FUNCTIONS***//
// Function   : switchRemapAllowed / switchRemapApplyCell
//
// Description: The validation and apply helpers shared by every remap
//              endpoint. switchRemapAllowed pins a cell value to the
//              caregiver-safe allowlist (LSConfig.h — one accidental switch
//              press must never be able to reset or factory-reset the
//              device), and is COLUMN-AWARE (4.5.7 review): the gamepad
//              columns additionally reject CONF_ACTION_DRAG/SCROLL — mouse
//              hold modes that wedge the whole input path in gamepad mode
//              (see CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN).
//              switchRemapApplyCell writes one remapped cell into
//              the live table; the jack index runs 0-2, the window index
//              0-2 (short/long/max), gamepad selects the action column.
//
// Parameters : actionNumber : int : an output action number
//              gamepad : bool : true = gamepad-mode column
//              jack / window : int : cell coordinates
//
// Return     : bool : allowlist membership
//              void : (apply)
//****************************************//
bool switchRemapAllowed(int actionNumber, bool gamepad) {
  bool allowed = false;
  for (unsigned int i = 0; i < CONF_SWITCH_REMAP_ALLOWED_NUM; i++) {
    if (CONF_SWITCH_REMAP_ALLOWED[i] == actionNumber) { allowed = true; break; }
  }
  if (allowed && gamepad) {
    for (unsigned int i = 0; i < CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN_NUM; i++) {
      if (CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN[i] == actionNumber) { allowed = false; break; }
    }
  }
  return allowed;
}

void switchRemapApplyCell(int jack, int window, bool gamepad, int actionNumber) {
  int row = CONF_SWITCH_REMAP_FIRST_ROW + jack * CONF_SWITCH_REMAP_ROWS_PER_JACK + window;
  if (row < 0 || row >= (int)(sizeof(switchActionProperty) / sizeof(inputActionStruct))) return;
  if (gamepad) {
    g_switchActionTable[row].gamepadOutputActionNumber = (uint8_t)actionNumber;
  } else {
    g_switchActionTable[row].mouseOutputActionNumber = (uint8_t)actionNumber;
  }
}

int switchRemapDefaultCell(int jack, int window, bool gamepad) {
  int row = CONF_SWITCH_REMAP_FIRST_ROW + jack * CONF_SWITCH_REMAP_ROWS_PER_JACK + window;
  if (gamepad) return (int)switchActionProperty[row].gamepadOutputActionNumber;
  return (int)switchActionProperty[row].mouseOutputActionNumber;
}

//***EFFECTIVE WINDOW FUNCTION***//
// Function   : effectiveWindowMs
//
// Description: Maps an action-table row's compiled window boundary
//              (1000 / 3000 / 8000 ms — the shorts/longs/very-longs the
//              tables are written against) onto the user's stored input
//              timing windows (roadmap item 15, upstream #62): a stored 0
//              is the compiled default, so the factory feel is exact.
//              Any other boundary value passes through unchanged.
//
// Parameters : defaultWindowMs : unsigned long : The compiled boundary value
//
// Return     : unsigned long : The boundary in force
//****************************************//
unsigned long effectiveWindowMs(unsigned long defaultWindowMs) {
  switch (defaultWindowMs) {
    case CONF_INPUT_WINDOW_SHORT_DEFAULT_MS:
      return (g_inputWindowShortMs > 0) ? g_inputWindowShortMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
    case CONF_INPUT_WINDOW_LONG_DEFAULT_MS:
      return (g_inputWindowLongMs > 0) ? g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
    case CONF_INPUT_WINDOW_MAX_DEFAULT_MS:
      return (g_inputWindowMaxMs > 0) ? g_inputWindowMaxMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
    default:
      return defaultWindowMs;  // 0 (row inert) or any custom boundary passes through
  }
}

//***FILL USB HID DIAGNOSTICS FUNCTION***//
// Function   : fillUsbHidDiagnostics
//
// Description: HID-layer telemetry for the BH,0:0 bus-health line (bench
//              2026-10-16: the cursor stopped moving on 4.5.0 while the
//              ready page showed, the joystick read fine and serial kept
//              working — the ready page only draws when isReady() held, so
//              the failure hides between "mouseReport never called",
//              "called and refused by TinyUSB" and "accepted but ignored by
//              the host". These seven numbers split exactly those cases).
//
//              The values describe the ACTIVE mode's HID interface (bench
//              2026-10-18, gamepad-mode validation): mouse mode reads
//              usbmouse and gamepad mode reads the gamepad — the old
//              unconditional mouse read made BH report hidReady=0 and
//              attempts=0 forever on a healthy gamepad boot, so health
//              checks and stuck-boot forensics lied exactly when they
//              mattered (LSUSBGamepad now carries the same counters as
//              LSUSBMouse, filled at the same choke points).
//
// Parameters : out : int[9] : filled with [usbMounted, hidReady, hidRetrying,
//              hidTimedOut, centerComplete, reportAttempts, reportSuccesses,
//              nonzeroReports, maxAbsReport]
//
// Return     : void
//****************************************//
void fillUsbHidDiagnostics(int out[9]) {
  const bool gamepadMode = (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD);
  out[0] = USBDevice.mounted() ? 1 : 0;
  out[1] = (gamepadMode ? gamepad.isReady() : usbmouse.isReady()) ? 1 : 0;
  out[2] = (gamepadMode ? gamepad.usbRetrying : usbmouse.usbRetrying) ? 1 : 0;
  out[3] = (gamepadMode ? gamepad.timedOut : usbmouse.timedOut) ? 1 : 0;
  out[4] = g_resetCenterComplete ? 1 : 0;
  // Wrap-safe reads of boot-long counters (the raw uint32 would overflow int)
  out[5] = (int)((gamepadMode ? gamepad.reportAttempts : usbmouse.reportAttempts) % 1000000007);
  out[6] = (int)((gamepadMode ? gamepad.reportSuccesses : usbmouse.reportSuccesses) % 1000000007);
  out[7] = (int)((gamepadMode ? gamepad.nonzeroReports : usbmouse.nonzeroReports) % 1000000007);
  out[8] = gamepadMode ? gamepad.maxAbsReport : usbmouse.maxAbsReport;
}

//***FILL JOYSTICK PIPELINE DIAGNOSTICS FUNCTION***//
// Function   : fillJoystickPipelineDiagnostics
//
// Description: Free-function bridge for LSJoystick::fillPipelineDiagnostics
//              (the .ino cannot call private members directly, but the
//              merged-TU free function in LSApiDeclarations.h's pattern
//              needs a callable from LSAPI.ino's BH handler).
//
// Parameters : out : int[14] : the live pipeline internals
//
// Return     : void
//****************************************//
void fillJoystickPipelineDiagnostics(int out[14]) {
  js.fillPipelineDiagnostics(out);
}

//***PRESSURE THRESHOLD LEVEL HELPERS***//
// Function   : pressThresholdLevelUp / pressThresholdLevelDown / pressThresholdLevelFor
//
// Description: The coarse 10-level threshold scale (roadmap item 15,
//              upstream #22): the Hub menu's Increase/Decrease step through
//              PRESS_THRESHOLD_LEVEL_TABLE instead of ±1 hPa, so a
//              mouthpiece-driven adjustment is practical. At the top/bottom
//              of the scale the step is a no-op (the re-read shows the
//              unchanged value).
//
// Return     : the stepped threshold in hPa (level index for the display)
//****************************************//
float pressThresholdLevelUp(float currentThreshold) {
  for (int level = 0; level < PRESS_THRESHOLD_LEVEL_NUM; level++) {
    if (PRESS_THRESHOLD_LEVEL_TABLE[level] > currentThreshold + 0.01f) return PRESS_THRESHOLD_LEVEL_TABLE[level];
  }
  return currentThreshold;  // already at the top level
}

float pressThresholdLevelDown(float currentThreshold) {
  for (int level = PRESS_THRESHOLD_LEVEL_NUM - 1; level >= 0; level--) {
    if (PRESS_THRESHOLD_LEVEL_TABLE[level] < currentThreshold - 0.01f) return PRESS_THRESHOLD_LEVEL_TABLE[level];
  }
  return currentThreshold;  // already at the bottom level
}

int pressThresholdLevelFor(float currentThreshold) {
  for (int level = 0; level < PRESS_THRESHOLD_LEVEL_NUM; level++) {
    if (PRESS_THRESHOLD_LEVEL_TABLE[level] >= currentThreshold - 0.01f) return level + 1;
  }
  return PRESS_THRESHOLD_LEVEL_NUM;
}

//***APPLY INPUT RESTRICTIONS FUNCTION***//
// Function   : applyInputRestrictions
//
// Description: The single seam where a resolved input action meets the
//              restriction settings (roadmap item 11 — the 4.2.0
//              sensor-health gating occupies the same place in spirit: a
//              gate between "an input happened" and "the device acts").
//              Gated actions become CONF_ACTION_NOTHING, which the caller
//              already treats as "no feedback, no output" — a phantom or
//              forbidden input produces nothing at all, not a half-action.
//
// Parameters : actionIndex : int : The resolved output action number
//              inputClass : int : INPUT_CLASS_SAP / _BUTTON / _SWITCH
//
// Return     : int : The action to actually perform (CONF_ACTION_NOTHING when restricted)
//****************************************//
int applyInputRestrictions(int actionIndex, int inputClass) {
  if (actionIndex == CONF_ACTION_NOTHING) return actionIndex;

  // Mode lock: no input path may change the operating mode. The serial API
  // stays open on purpose — a caregiver must be able to unlock or adjust the
  // device through LipSync Connect even while the device user cannot.
  if (g_modeLocked && actionIndex == CONF_ACTION_CHANGE_MODE) return CONF_ACTION_NOTHING;

  // Menu restricted to hub buttons: sip & puff and external switches can
  // neither open/close the menu nor navigate it. Only the hub buttons
  // (inputClass BUTTON) keep menu control.
  if (g_menuButtonsOnly && inputClass != INPUT_CLASS_BUTTON) {
    switch (actionIndex) {
      case CONF_ACTION_START_MENU:
      case CONF_ACTION_STOP_MENU:
      case CONF_ACTION_NEXT_MENU_ITEM:
      case CONF_ACTION_SELECT_MENU_ITEM:
        return CONF_ACTION_NOTHING;
    }
  }

  return actionIndex;
}

//***INPUT LOOP FUNCTION***//
// Function   : inputLoop
//
// Description: This function handles input button and input switch actions.
//
// Parameters : void
//
// Return     : void
//****************************************//
void inputLoop() {

  //if (USB_DEBUG) { Serial.println("USBDEBUG: inputLoop"); }
  // Read new values
  ib.update();  // update buttons
  is.update();  // update external assistive switch inputs

  // Get the last state change
  buttonState = ib.getInputState();
  switchState = is.getInputState();

  // Evaluate Output Actions
  evaluateOutputAction(buttonState, buttonActionMaxTime, buttonActionSize, buttonActionProperty, INPUT_CLASS_BUTTON);
  evaluateOutputAction(switchState, switchActionMaxTime, switchActionSize, g_switchActionTable, INPUT_CLASS_SWITCH);

}

//*********************************//
// Sip and Puff Functions
//*********************************//

//***INITIALIZE SIP AND PUFF FUNCTION***//
// Function   : initSipAndPuff
//
// Description: This function initializes sip and puff as inputs.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initSipAndPuff() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: Initializing Sip and Puff"); }
  ps.begin();                                                             // Begin sip and puff
  getSipPressureThreshold(false, false);                                  // Get sip  pressure thresholds stored in flash memory
  getPuffPressureThreshold(false, false);                                 // Get puff pressure thresholds stored in flash memory
  sapActionSize = sizeof(sapActionProperty) / sizeof(inputActionStruct);  // Size of total available sip and puff actions
  sapActionMaxTime = getActionMaxTime(sapActionSize, sapActionProperty);  // Maximum end action time
}


//***GET ACTION MAX TIME FUNCTION***//
// Function   : getActionMaxTime
//
// Description: This function finds the maximum end action time
//
// Parameters : actionSize : int : size of available actions
//              actionProperty : const inputActionStruct : array of all possible actions
//
// Return     : actionMaxTime : unsigned long : maximum end action time
//****************************************//
unsigned long getActionMaxTime(int actionSize, const inputActionStruct actionProperty[]) {
  unsigned long actionMaxTime = 0;
  // Loop over all possible outputs
  for (int actionIndex = 0; actionIndex < actionSize; actionIndex++) {
    if (actionMaxTime < actionProperty[actionIndex].inputActionEndTime) {
      actionMaxTime = actionProperty[actionIndex].inputActionEndTime;
    }
  }
  return actionMaxTime;
}

//***PRESSURE LOOP FUNCTION***//
// Function   : pressureLoop
//
// Description: This function handles pressure polling, sip and puff state evaluation.
//
// Parameters : void
//
// Return     : void
//****************************************//
void pressureLoop() {
  //if (USB_DEBUG) { Serial.println("USBDEBUG: pressureLoop()"); }
  ps.update();  // Request new pressure difference from sensor and push it to array

  //pressureValues = ps.getAllPressure();  // Read the pressure object (can be last value from array, average or other algorithms)

  // Get the last state change
  sapActionState = ps.getState();

  // Output action logic — INPUT_CLASS_SAP lets the restriction gates tell a
  // mouthpiece input from a button/switch one (roadmap item 11).
  evaluateOutputAction(sapActionState, sapActionMaxTime, sapActionSize, sapActionProperty, INPUT_CLASS_SAP);
}

//***RELEASE OUTPUT FUNCTION***//
// Function   : releaseOutputAction
//
// Description: This function handles release of usbmouse hold actions.
//
// Parameters : void
//
// Return     : void
//****************************************//
void releaseOutputAction() {
  // Release left click if it's in drag mode and left mouse button is pressed.
  if (outputAction == CONF_ACTION_DRAG && (usbmouse.isPressed(MOUSE_LEFT) || btmouse.isPressed(MOUSE_LEFT))) {
    usbmouse.release(MOUSE_LEFT);
    btmouse.release(MOUSE_LEFT);
  }
  // Set new state of current output action
  outputAction = CONF_ACTION_NOTHING;
  canOutputAction = true;
}

//***EVALUATE OUTPUT ACTION FUNCTION***//
// Function   : evaluateOutputAction
//
// Description: This function evaluates and performs output action
//
// Parameters : actionState : inputStateStruct : Current input state
//              actionMaxEndTime : unsigned long : maximum end action time
//              actionSize : int : size of available actions
//              actionProperty : const inputActionStruct : array of all possible actions
//              inputClass : int : INPUT_CLASS_SAP / _BUTTON / _SWITCH — which
//                          physical input the state came from (restriction
//                          gates, roadmap item 11)
//
// Return     : void
//****************************************//
void evaluateOutputAction(inputStateStruct actionState, unsigned long actionMaxEndTime, int actionSize, const inputActionStruct actionProperty[], int inputClass) {
  g_menuInputHoldMs = 0;  // fresh per pass — see the global's comment
  // Sip & puff disabled (SD): no new sap action may fire. The hold-release
  // and timer bookkeeping below still runs, so a drag/scroll engaged before
  // the restriction was applied still ends cleanly instead of sticking.
  bool canEvaluateAction = (inputClass != INPUT_CLASS_SAP || !g_sapDisabled);

  // Output action logic
  int tempActionIndex = 0;

  // Handle input action when it's in hold state (scroll mode or drag mode)
  if ((actionState.secondaryState == INPUT_SEC_STATE_RELEASED) && (outputAction == CONF_ACTION_SCROLL || outputAction == CONF_ACTION_DRAG)) {
    setLedDefault();  // Set default led feedback
    // Set new state of current output action
    releaseOutputAction();
    canEvaluateAction = false;
  }  // Detected input release after defined time limits.
  else if (actionState.secondaryState == INPUT_SEC_STATE_RELEASED && actionState.elapsedTime > effectiveWindowMs(actionMaxEndTime)) {
    // Set Led color to default
    setLedDefault();
  }

  // Code to switch between joystick controlled scroll and joystick controlled cursor movement
  // 4.5.7 review: the swap is gated on !g_safeModeEnabled — the unconditional
  // form re-enabled the JOYSTICK poll timer toggleSafeMode had deliberately
  // shut down on EVERY sensorful safe boot (the first input tick's
  // outputAction == NOTHING takes the enable-joystick branch), restoring
  // exactly the 20 ms I2C-against-degraded-hardware traffic safe mode
  // exists to avoid. Safe-mode action columns never resolve to
  // SCROLL/DRAG, so there is nothing for the swap to do there.
  if(g_joystickSensorConnected && !g_safeModeEnabled){
    if (outputAction == CONF_ACTION_SCROLL) {
      pollTimer.enable(pollTimerId[CONF_TIMER_SCROLL]);
      pollTimer.disable(pollTimerId[CONF_TIMER_JOYSTICK]);
    } else if (outputAction != CONF_ACTION_SCROLL) {
      pollTimer.enable(pollTimerId[CONF_TIMER_JOYSTICK]);
      pollTimer.disable(pollTimerId[CONF_TIMER_SCROLL]);
    }
  }

  // Loop over all possible outputs
  for (int actionIndex = 0; actionIndex < actionSize && canEvaluateAction && canOutputAction; actionIndex++) {
    // Detected input release in defined time limits. Perform output action based on action index
    if (actionState.mainState == actionProperty[actionIndex].inputActionState
        && actionState.secondaryState == INPUT_SEC_STATE_RELEASED
        && actionState.elapsedTime >= effectiveWindowMs(actionProperty[actionIndex].inputActionStartTime)
        && actionState.elapsedTime < effectiveWindowMs(actionProperty[actionIndex].inputActionEndTime)) {
      
      // Get action index
      if (screen.isMenuActive()) {
        tempActionIndex = actionProperty[actionIndex].menuOutputActionNumber;
      } else {
        switch (g_operatingMode) {
          case CONF_OPERATING_MODE_MOUSE:
            tempActionIndex = actionProperty[actionIndex].mouseOutputActionNumber;
            break;
          case CONF_OPERATING_MODE_GAMEPAD:
            tempActionIndex = actionProperty[actionIndex].gamepadOutputActionNumber;
            break;
          case CONF_OPERATING_MODE_SAFE:
            tempActionIndex = actionProperty[actionIndex].safeModeOutputActionNumber;
            break;
        }
      }

      // Restriction gates (roadmap item 11): a forbidden action resolves to
      // NOTHING here — no LED feedback, no output, no trace of the input.
      tempActionIndex = applyInputRestrictions(tempActionIndex, inputClass);

      // Input-state LED mode (LM=3, roadmap item 16 / upstream #43): the
      // window the hold was in already showed on the LEDs while it was
      // held; the release restores the default LEDs instead of blinking
      // the action — the window was the message. The action itself still
      // fires; a gated (restricted) input shows nothing at all.
      bool inputStateLeds = (tempActionIndex != CONF_ACTION_NOTHING && g_lightMode == CONF_LIGHT_MODE_STATE);

      if (inputStateLeds) {
        setLedDefault();
      } else {
        // Set Led color to default
        //setLedDefault();
        // Set Led state
        setLedState(ledActionProperty[tempActionIndex].ledEndAction,
                    ledActionProperty[tempActionIndex].ledEndColor,
                    ledActionProperty[tempActionIndex].ledNumber,
                    CONF_INPUT_LED_BLINK,
                    CONF_INPUT_LED_DELAY,
                    led.getLedBrightness());
      }
      outputAction = tempActionIndex;

      // Engagement cue for the two hold modes (roadmap item 16, upstream
      // #61): one short tone when scroll or drag takes, so the mode change
      // is audible, not just visible on the LEDs.
      if (tempActionIndex == CONF_ACTION_SCROLL || tempActionIndex == CONF_ACTION_DRAG) {
        buzzer.modeEngageTone(tempActionIndex == CONF_ACTION_SCROLL);
      }

      // Perform led action
      if (!inputStateLeds) {
        performLedAction(ledCurrentState);
      }

      // Perform output action — but NEVER for a restriction-gated NOTHING
      // (4.5.7 review): performOutputAction's gamepad NOTHING case releases
      // all buttons, an observable output that violates the gate's "no
      // output, no trace" contract (bench-visible as a "Released all"
      // burst and a killed in-flight pulse on every LK/MA/SD-refused
      // press). A genuine compiled NOTHING cell also skips: "nothing"
      // means nothing, and the actionTimer's scheduled releaseAll already
      // cleans up gamepad pulses.
      if (tempActionIndex != CONF_ACTION_NOTHING) {
        g_menuInputHoldMs = actionState.elapsedTime;  // the hold-to-confirm gate reads this (see selectMenuItem)
        performOutputAction(tempActionIndex);
      }

      break;
    }  // Detected input start in defined time limits. Perform led action based on action index
    else if (actionState.mainState == actionProperty[actionIndex].inputActionState
             && actionState.secondaryState == INPUT_SEC_STATE_STARTED
             && actionState.elapsedTime >= effectiveWindowMs(actionProperty[actionIndex].inputActionStartTime)
             && actionState.elapsedTime < effectiveWindowMs(actionProperty[actionIndex].inputActionEndTime)) {
      // Get action index
      if (screen.isMenuActive()) {
        tempActionIndex = actionProperty[actionIndex].menuOutputActionNumber;
      } else {
        switch (g_operatingMode) {
          case CONF_OPERATING_MODE_MOUSE:
            tempActionIndex = actionProperty[actionIndex].mouseOutputActionNumber;
            break;
          case CONF_OPERATING_MODE_GAMEPAD:
            tempActionIndex = actionProperty[actionIndex].gamepadOutputActionNumber;
            break;
          case CONF_OPERATING_MODE_SAFE:
            tempActionIndex = actionProperty[actionIndex].safeModeOutputActionNumber;
            break;
        }
      }

      // Restriction gates (roadmap item 11): the started-phase LED cue is
      // suppressed too — a forbidden input must not light anything up.
      tempActionIndex = applyInputRestrictions(tempActionIndex, inputClass);

      // Input-state LED mode (LM=3, roadmap item 16 / upstream #43): the
      // LEDs show the window the hold is in — a DUAL channel (bench design
      // 2026-10-19): the three face LEDs FILL 1 -> 2 -> 3 as the hold
      // crosses each window boundary (they are physically single-color, so
      // a count is the one message they can carry), and the micro LED next
      // to the USB-C port shows the window's COLOR. Actions are not
      // one-to-one with inputs (#43), so the window is the honest signal,
      // and the fill previews what a release will do without memorizing
      // any mapping. Re-evaluated every tick, so the LEDs step up through
      // the windows as the hold crosses each boundary; a gated (restricted)
      // input still shows nothing at all.
      if (tempActionIndex != CONF_ACTION_NOTHING && g_lightMode == CONF_LIGHT_MODE_STATE) {
        unsigned long holdMs = actionState.elapsedTime;
        int stateColor;
        int windowCount;
        if (holdMs < effectiveWindowMs(CONF_INPUT_WINDOW_SHORT_DEFAULT_MS)) {
          stateColor = CONF_INPUT_STATE_LED_SHORT_COLOR;  // green
          windowCount = 1;
        } else if (holdMs < effectiveWindowMs(CONF_INPUT_WINDOW_LONG_DEFAULT_MS)) {
          stateColor = CONF_INPUT_STATE_LED_LONG_COLOR;  // yellow
          windowCount = 2;
        } else {
          stateColor = CONF_INPUT_STATE_LED_MAX_COLOR;  // red
          windowCount = 3;
        }
        led.setInputStateLeds(windowCount, stateColor, led.getLedBrightness());
        break;
      }

      // Set Led color to default
      //setLedDefault();
      // Set Led state
      setLedState(LED_ACTION_ON,
                  ledActionProperty[tempActionIndex].ledStartColor,
                  ledActionProperty[tempActionIndex].ledNumber,
                  0,                     // number of blinks
                  0,                     // blink time
                  led.getLedBrightness());  // brightness
      // Perform led action
      performLedAction(ledCurrentState);

      break;
    }
  }
}

//***PERFORM OUTPUT ACTION FUNCTION***//
// Function   : performOutputAction
//
// Description: This function performs output action
//
// Parameters : action : int : action index number
//
// Return     : void
//****************************************//
void performOutputAction(int action) {
  switch (action) {
    case CONF_ACTION_NOTHING:
      {
        // A matched NOTHING cell in gamepad mode still clears any in-flight
        // button pulse (the actionTimer auto-release below also covers it;
        // this is the immediate path). The debug print is gone (4.5.7
        // review): it fired from loop context on the wire for every
        // matched nothing-cell — and for every LK/MA/SD-gated input, which
        // is why the restriction gates now skip performOutputAction
        // entirely for gated actions (see evaluateOutputAction).
        if (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) {
          gamepadButtonReleaseAll();
        }
        break;
      }
    case CONF_ACTION_LEFT_CLICK:
      {
        cursorLeftClick();
        break;
      }
    case CONF_ACTION_RIGHT_CLICK:
      {
        cursorRightClick();
        break;
      }
    case CONF_ACTION_DRAG:
      {
        cursorDrag();
        break;
      }
    case CONF_ACTION_SCROLL:
      {
        cursorScroll();  // Enter Scroll mode
        break;
      }
    case CONF_ACTION_CURSOR_CENTER:
      {
        screen.centerResetPage();  // Perform cursor center
        //setJoystickInitialization(true,false);
        break;
      }
    case CONF_ACTION_CURSOR_CALIBRATION:
      {
        setJoystickCalibration(true, false);
        break;
      }
    case CONF_ACTION_MIDDLE_CLICK:
      {
        cursorMiddleClick();  // Perform cursor middle click
        break;
      }
    case CONF_ACTION_DEC_SPEED:
      {
        decreaseCursorSpeed(true, false);  // Decrease cursor speed
        break;
      }
    case CONF_ACTION_INC_SPEED:
      {
        increaseCursorSpeed(true, false);  // Increase cursor speed
        break;
      }
    case CONF_ACTION_CHANGE_MODE:
      {
        // Change communication mode
        toggleCommunicationMode(true, false);
        break;
      }
    case CONF_ACTION_START_MENU:
      {
        screen.activateMenu();  // Activate Menu
        break;
      }
    case CONF_ACTION_STOP_MENU:
      {
        screen.closeMenu();  // Deactivate Menu (freeze-aware: see LSScreen::closeMenu)
        break;
      }
    case CONF_ACTION_B1_PRESS:
      {
        gamepadButtonPress(1);
        break;
      }
    case CONF_ACTION_B2_PRESS:
      {
        gamepadButtonPress(2);
        break;
      }
    case CONF_ACTION_B3_PRESS:
      {
        gamepadButtonPress(3);
        break;
      }
    case CONF_ACTION_B4_PRESS:
      {
        gamepadButtonPress(4);
        break;
      }
    case CONF_ACTION_B5_PRESS:
      {
        gamepadButtonPress(5);
        break;
      }
    case CONF_ACTION_B6_PRESS:
      {
        gamepadButtonPress(6);
        break;
      }
    case CONF_ACTION_B7_PRESS:
      {
        gamepadButtonPress(7);
        break;
      }
    case CONF_ACTION_B8_PRESS:
      {
        gamepadButtonPress(8);
        break;
      }
    case CONF_ACTION_NEXT_MENU_ITEM:
      {
        screen.nextMenuItem();  // Move to next menu item
        break;
      }
    case CONF_ACTION_SELECT_MENU_ITEM:
      {
        screen.selectMenuItem();  // Move to next menu item
        break;
      }
    case CONF_ACTION_RESET:
      {
        softwareReset();
        break;
      }
    case CONF_ACTION_FACTORY_RESET:
      {
        doFactoryReset(true, false);  // Perform Factory Reset
        break;
      }
  }
  if (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) {
    //actionTimerId[0] = actionTimer.setTimeout(CONF_BUTTON_PRESS_DELAY, gamepadButtonRelease, (int *)action);
    actionTimerId[0] = actionTimer.setTimeout(CONF_BUTTON_PRESS_DELAY, gamepadButtonReleaseAll);  // TODO: Change this to just release one
    outputAction = CONF_ACTION_NOTHING;
  }

  // Hold-mode bookkeeping (4.5.7 review hardening): the mouse's drag/scroll
  // hold tracker only arms OUTSIDE gamepad mode — in gamepad mode the branch
  // above has already clobbered outputAction to NOTHING, so arming the
  // canOutputAction=false latch here could NEVER be released again (the
  // release match keys on outputAction == DRAG/SCROLL): one rogue gamepad
  // DRAG/SCROLL mapping (now allowlist-blocked twice over) used to wedge
  // every input on the device until a reboot or a serial center flow.
  if ((action == CONF_ACTION_DRAG || action == CONF_ACTION_SCROLL) && g_operatingMode != CONF_OPERATING_MODE_GAMEPAD) {
    canOutputAction = false;
  } else {
    outputAction = CONF_ACTION_NOTHING;
    canOutputAction = true;
  }
}


//*********************************//
// Mouse Functions
//*********************************//

//***CURSOR LEFT CLICK FUNCTION***//
// Function   : cursorLeftClick
//
// Description: This function performs cursor left click action.
//
// Parameters : void
//
// Return     : void
//****************************************//
void cursorLeftClick(void) {
  if (g_comMode == CONF_COM_MODE_USB) {
    usbmouse.click(MOUSE_LEFT);  // USB Mouse left click
  } else if (g_comMode == CONF_COM_MODE_BLE) {
    btmouse.click(MOUSE_LEFT);  // Bluetooth mouse left click
  }
}

//***CURSOR RIGHT CLICK FUNCTION***//
// Function   : cursorRightClick
//
// Description: This function performs cursor right click action.
//
// Parameters : void
//
// Return     : void
//****************************************//
void cursorRightClick(void) {
  if (g_comMode == CONF_COM_MODE_USB) {
    usbmouse.click(MOUSE_RIGHT);  // USB Mouse right click
  } else if (g_comMode == CONF_COM_MODE_BLE) {
    btmouse.click(MOUSE_RIGHT);  // Bluetooth mouse right click
  }
}

//***CURSOR MIDDLE CLICK FUNCTION***//
// Function   : cursorMiddleClick
//
// Description: This function performs cursor middle click action.
//
// Parameters : void
//
// Return     : void
//****************************************//
void cursorMiddleClick(void) {
  if (g_comMode == CONF_COM_MODE_USB) {
    usbmouse.click(MOUSE_MIDDLE);  // USB Mouse Middle click
  } else if (g_comMode == CONF_COM_MODE_BLE) {
    btmouse.click(MOUSE_MIDDLE);  // Bluetooth mouse middle click
  }
}

//***DRAG FUNCTION***//
// Function   : cursorDrag
//
// Description: This function performs cursor drag action.
//
// Parameters : void
//
// Return     : void
//********************//
void cursorDrag(void) {
  if (g_comMode == CONF_COM_MODE_USB) {
    usbmouse.press(MOUSE_LEFT);  // USB Mouse press left
  } else if (g_comMode == CONF_COM_MODE_BLE) {
    btmouse.press(MOUSE_LEFT);  // Bluetooth Mouse press left
  }
}
//***CURSOR SCROLL FUNCTION***//
// Function   : cursorScroll
//
// Description: This function is an operating mode that enables scrolling action.
//
// Parameters : void
//
// Return     : void
//****************************************//
void cursorScroll(void) {
  outputAction = CONF_ACTION_SCROLL;
  g_scrollNumRuns = 0;
}


//*********************************//
// Gamepad Functions
//*********************************//

//***GAMEPAD BUTTON PRESS FUNCTION***//
// Function   : gamepadButtonPress
//
// Description: This function performs button press action.
//
// Parameters : int : buttonNumber
//
// Return     : void
//****************************************//
void gamepadButtonPress(int buttonNumber) {
  if (buttonNumber > 0 && buttonNumber <= 8) {
    gamepad.press(buttonNumber - 1);
    gamepad.send();  // Gamepad button press
  }
}

//***GAMEPAD BUTTON CLICK FUNCTION***//
// Function   : gamepadButtonClick
//
// Description: This function performs button click action.
//
// Parameters : int : buttonNumber
//
// Return     : void
//****************************************//
void gamepadButtonClick(int buttonNumber) {
  if (buttonNumber > 0 && buttonNumber <= 8) {
    gamepad.press(buttonNumber - 1);
    gamepad.send();
    actionTimerId[0] = actionTimer.setTimeout(CONF_BUTTON_PRESS_DELAY, gamepadButtonRelease, (int*)buttonNumber);
  }
}

//***GAMEPAD BUTTON RELEASE FUNCTION***//
// Function   : gamepadButtonRelease
//
// Description: This function performs button release action.
//
// Parameters : int* : args
//
// Return     : void
//****************************************//
void gamepadButtonRelease(int* args) {
  int buttonNumber = (int)args;
  if (buttonNumber > 0 && buttonNumber <= 8) {
    gamepad.release(buttonNumber - 1);
    gamepad.send();
  }
}


//***GAMEPAD BUTTON RELEASE ALL FUNCTION***//
// Function   : gamepadButtonReleaseAll
//
// Description: This function performs button release action.
//
// Parameters : void
//
// Return     : void
//****************************************//
void gamepadButtonReleaseAll() {
  gamepad.releaseAll();  // Release all gamepad buttons
  gamepad.send();
}



//*********************************//
// Joystick Functions
//*********************************//

//***INITIALIZE JOYSTICK FUNCTION***//
// Function   : initJoystick
//
// Description: This function initializes joystick as input.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initJoystick() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initJoystick()"); }
  js.begin();                                                           // Begin joystick
  js.setMagnetDirection(JOY_DIRECTION_DEFAULT, JOY_DIRECTION_INVERSE);  // Set x and y magnet direction
  getJoystickInnerDeadzone(true, false);                               // Get joystick deadzone stored in flash memory
  getJoystickOuterDeadzone(true, false);                                     // Get joystick deadzone stored in flash memory
  getCursorSpeed(true, false);                                          // Get joystick cursor speed stored in flash memory
  g_scrollLevel = getScrollLevel(true, false);                            // Get scroll level stored in flash memory
  setJoystickInitialization(true, false);                               // Perform joystick center initialization
  getJoystickCalibration(true, false);                                  // Get joystick calibration points stored in flash memory
}

//***PERFORM JOYSTICK CENTER FUNCTION***//
// Function   : performJoystickCenter
//
// Description: This function performs joystick center point initialization.
//
// Parameters : * args : int : pointer of step number
//
// Return     : void
//****************************************//
void performJoystickCenter(int* args) {
  // The timer payload carries this chain's initiator alongside the step
  // number (CONF_CENTER_INIT_API_FLAG, 4.5.4): SUCCESS only for a center
  // the HOST requested over the serial API (IN,1:1); MANUAL for every
  // device-initiated center (the boot auto-center, the Hub-menu Center
  // Reset, the calibration's internal step 5) — an unsolicited SUCCESS on
  // the wire is a lie a command waiter can false-match (the bench caught a
  // boot straggler landing between the VN request and its response); MANUAL
  // carries the same data and matches no waiter. Riding the payload
  // instead of the 4.5.2 global means each in-flight chain owns its own
  // initiator: two overlapping centers (an app IN,1:1 racing the boot
  // auto-center) each ack with their own correct prefix instead of sharing
  // one flag that whichever-fires-first consumes.
  bool centerInitFromApi = ((int)args & CONF_CENTER_INIT_API_FLAG) != 0;
  int stepNumber = (int)args & ~CONF_CENTER_INIT_API_FLAG;
  if (USB_DEBUG) { Serial.print("USBDEBUG: performJoystickCenter("); Serial.print(stepNumber); Serial.println(")"); }

  g_resetCenterComplete = false;  // Reset the global flag

  

  // Duration of the center point readings (500 milliseconds)
  unsigned long readingDuration = CONF_JOY_INIT_READING_DELAY * CONF_JOY_INIT_READING_NUMBER;

  // (500 + 150*3) - time until start of current reading.
  unsigned long currentReadingStart = CONF_JOY_INIT_START_DELAY + (CONF_JOY_INIT_STEP_BLINK_DELAY * ((CONF_JOY_INIT_STEP_BLINK * 2) + 1));

  // Time until start of next step. (1450 seconds )
  unsigned long nextStepStart = currentReadingStart + readingDuration;

  pointFloatType centerPoint;

  if (stepNumber == 0)  // STEP 0: Joystick Compensation Center Point
  {
    if (ledActionEnabled) {
      setLedState(LED_ACTION_BLINK, CONF_JOY_INIT_STEP_BLINK_COLOR, CONF_JOY_INIT_LED_NUMBER, CONF_JOY_INIT_STEP_BLINK, CONF_JOY_INIT_STEP_BLINK_DELAY, led.getLedBrightness());
      performLedAction(ledCurrentState);  // LED Feedback to show start of performJoystickCalibrationStep
    }
    // Start timer to get 5 reading every 100 ms. A refused registration
    // (pool full — the failure the 4.5.1 audit made loud) must not leave the
    // cursor output dead: without the reading timer the completion timeout
    // below would still fire and average an empty center buffer, so bail
    // out honestly instead — restore the output-enabled state and keep the
    // previous center (4.5.7 review; unreachable with 16 slots, but the
    // guard is one check).
    calibrationTimerId[1] = calibrationTimer.setTimer(CONF_JOY_INIT_READING_DELAY, currentReadingStart, CONF_JOY_INIT_READING_NUMBER, performJoystickCenterStep, (int*)args);
    if (calibrationTimerId[1] < 0) {
      Serial.println("ERROR: center-reset reading timer refused -- keeping previous center");
      setLedDefault();
      canOutputAction = true;
      g_resetCenterComplete = true;
      return;
    }
    // Carry the initiator into the completion step
    int payload = (stepNumber + 1) | (centerInitFromApi ? CONF_CENTER_INIT_API_FLAG : 0);
    // Start exit step
    calibrationTimerId[0] = calibrationTimer.setTimeout(nextStepStart, performJoystickCenter, (int*)payload);
  } else {
    js.evaluateInputCenter();           // Evaluate the center point using values in the buffer
    js.setMinimumRadius();              // Update minimum radius of operation
    centerPoint = js.getInputCenter();  // Get the new center for API output
    printResponseFloatPoint(true, centerInitFromApi, true, 0, "IN,1", true, centerPoint);
    // No deleteTimer here (4.5.4): the fired RUN_ONCE timeout already
    // deleted itself, and the old blind deleteTimer(0) could kill a LIVE
    // timer — slot 0 is the lowest free slot handed to the next
    // registration, which in an overlapping-centers race is exactly the
    // OTHER flow's pending timeout (a host IN,1:1 arriving during the
    // boot auto-center's reading phase took slot 0 and was silently
    // eaten). Overlapping chains now both complete, each with their own
    // correct ack.
    setLedDefault();                  // Set default led feedback
    canOutputAction = true;
    g_resetCenterComplete = true;

    if (g_startupCenterReset) {  // Checks variable to only play sound and show splash screen on startup
      readyToUse();              // TODO JDMc 2025-Jan-24 Probably want sound to play after every calibration to indicate joystick is ready to be used again.
    }
    if (screen.showCenterResetComplete) {  // Checks variable so center reset complete page only shows if accessed from menu, not on startup or during full calibration
      screen.centerResetCompletePage();
    }
    g_startupCenterReset = false;
  }
}

//***PERFORM JOYSTICK CENTER STEP FUNCTION***//
// Function   : performJoystickCenterStep
//
// Description: This function performs the actual joystick center point initialization step.
//
// Parameters : * args : int : pointer of step number
//
// Return     : void
//****************************************//
void performJoystickCenterStep(int* args) {

  // Turn on and set the second led to orange to indicate start of the process
  if (calibrationTimer.getNumRuns(calibrationTimerId[1]) == 1 && ledActionEnabled) {  // Turn LED's ON when timer is running for first time
    setLedState(LED_ACTION_ON, CONF_JOY_INIT_LED_COLOR, CONF_JOY_INIT_LED_NUMBER, 0, 0, led.getLedBrightness());
    performLedAction(ledCurrentState);
  }
  // Push new center values to be evaluated at the end of the process
  js.updateInputCenterBuffer();

  // Turn off the second led to orange to indicate end of the process
  if (calibrationTimer.getNumRuns(calibrationTimerId[1]) == CONF_JOY_INIT_READING_NUMBER && ledActionEnabled) {  // Turn LED's OFF when timer is running for last time
    setLedState(LED_ACTION_OFF, LED_CLR_NONE, CONF_JOY_INIT_LED_NUMBER, 0, 0, led.getLedBrightness());
    performLedAction(ledCurrentState);
  }
}

//***PERFORM JOYSTICK CALIBRATION FUNCTION***//
// Function   : performJoystickCalibration
//
// Description: This function performs joystick maximum point calibration in a recursive fashion.
//
// Parameters : * args : int : pointer of step number
//
// Return     : void
//****************************************//
void performJoystickCalibration(int* args) {
  g_resetCenterComplete = false;
  g_calibrationError = false;
  // The initiator rides the chain's payload (CONF_CALIB_API_FLAG, 4.5.4
  // audit follow-up): the per-corner CA,n lines print SUCCESS only for a
  // calibration the HOST requested over the serial API (CA,1:1); a
  // Hub-menu-initiated run prints MANUAL with the same data — the 4.5.2
  // initiator contract (an unsolicited SUCCESS on the wire can falsely
  // resolve a pending command waiter).
  bool calibFromApi = ((int)args & CONF_CALIB_API_FLAG) != 0;
  int stepNumber = (int)args & ~CONF_CALIB_API_FLAG;

  // Duration of the max corner reading ( 2 seconds )
  unsigned long readingDuration = CONF_JOY_CALIB_READING_DELAY * CONF_JOY_CALIB_READING_NUMBER;

  // Time until start of current reading
  unsigned long currentReadingStart = CONF_JOY_CALIB_STEP_DELAY + (CONF_JOY_CALIB_STEP_BLINK_DELAY * ((CONF_JOY_CALIB_STEP_BLINK * 2) + 1));

  // Time until start of next step. (1.5 + (3*300) seconds )
  unsigned long nextStepStart = currentReadingStart + readingDuration + CONF_JOY_CALIB_START_DELAY;  // Time until start of next step. ( 2.4 + 2 + 1 seconds )

  if (stepNumber == 0) {                     // STEP 0: Calibration started
    // (g_calibrationRunning was set at chain acceptance — see setJoystickCalibration)
    pollTimer.disable(pollTimerId[CONF_TIMER_JOYSTICK]);  // Temporarily disable joystick data polling timer
    pollTimer.disable(pollTimerId[CONF_TIMER_SCROLL]);    // 4.5.7 review: scroll mode is a latch that survives release — without this, a scroll-latched device kept running joystickLoop through the scroll tick for the whole calibration (sensor I2C traffic during the shutdown, and live scroll events after the step-5 center re-enabled output). Step 6 already re-enables it (normal boots only).
    if (!g_safeModeEnabled) {
      // 4.5.7 review: safe mode never shuts the INPUT poll down (it is the
      // only hardware input there — how the safe menu is navigable), so the
      // calibration must not disable it either: step 6's safe-mode guard
      // skips the re-enable, and the asymmetry left every button/switch
      // dead for the rest of the boot after a CA,1:1 on a sensorful safe
      // unit. The sensor timers are already off in safe mode; disabling
      // them again is a harmless no-op.
      pollTimer.disable(pollTimerId[CONF_TIMER_INPUT]);
    }
    pollTimer.disable(pollTimerId[CONF_TIMER_PRESSURE]);
    // A menu-driven calibration is user-paced at each corner and can exceed
    // the 5-minute menu timeout (4.5.7 review): a mid-run deactivateMenu()
    // blanked the OLED between the step prompts. Safe mode already disables
    // the timeout (toggleSafeMode); the run re-enables it at completion.
    screen.disableTimeout();
    setLedState(LED_ACTION_BLINK, CONF_JOY_CALIB_START_LED_COLOR, CONF_JOY_CALIB_LED_NUMBER, CONF_JOY_CALIB_STEP_BLINK, CONF_JOY_CALIB_STEP_BLINK_DELAY, led.getLedBrightness());
    performLedAction(ledCurrentState);
    ++stepNumber;
    calibrationTimerId[0] = calibrationTimer.setTimeout(currentReadingStart, performJoystickCalibration, (int*)(stepNumber | (calibFromApi ? CONF_CALIB_API_FLAG : 0)));  // Start next step
  } else if (stepNumber < 5) {                                                                                               // STEP 1-4: Joystick Calibration Corner Points
    screen.fullCalibrationPrompt(stepNumber);
    buzzer.calibCornerTone();
    setLedState(LED_ACTION_BLINK, CONF_JOY_CALIB_STEP_BLINK_COLOR, CONF_JOY_CALIB_LED_NUMBER, CONF_JOY_CALIB_STEP_BLINK, CONF_JOY_CALIB_STEP_BLINK_DELAY, led.getLedBrightness());
    performLedAction(ledCurrentState);  // LED Feedback to show start of performJoystickCalibrationStep
    js.zeroInputMax(stepNumber);        // Clear the existing calibration value

    calibrationTimerId[1] = calibrationTimer.setTimer(CONF_JOY_CALIB_READING_DELAY, currentReadingStart, CONF_JOY_CALIB_READING_NUMBER, performJoystickCalibrationStep, (int*)(stepNumber | (calibFromApi ? CONF_CALIB_API_FLAG : 0)));
    ++stepNumber;
    // Set LED's feedback to show step is already started and get the max reading for the quadrant/step

    calibrationTimerId[0] = calibrationTimer.setTimeout(nextStepStart, performJoystickCalibration, (int*)(stepNumber | (calibFromApi ? CONF_CALIB_API_FLAG : 0)));  // Start next step
  } else if (stepNumber == 5) {                                                                                        // STEP 5 : Joystick center point initialization
    screen.fullCalibrationPrompt(stepNumber);
    buzzer.calibCenterTone();
    g_calibrationCenterStep = true;   // The chain's own internal center — bypass the in-flight gate (synchronous: no timer fires between set and clear)
    setJoystickInitialization(false, false);
    g_calibrationCenterStep = false;
    ++stepNumber;
    calibrationTimerId[0] = calibrationTimer.setTimeout(nextStepStart, performJoystickCalibration, (int*)(stepNumber | (calibFromApi ? CONF_CALIB_API_FLAG : 0)));  // Start next step
  }

  else  // STEP 6: Calibration ended
  {
    g_calibrationRunning = false;
    setLedState(LED_ACTION_BLINK, CONF_JOY_CALIB_START_LED_COLOR, CONF_JOY_CALIB_LED_NUMBER, CONF_JOY_CALIB_STEP_BLINK, CONF_JOY_CALIB_STEP_BLINK_DELAY, led.getLedBrightness());  // Turn off Led's
    performLedAction(ledCurrentState);
    js.setMinimumRadius();  // Update the minimum cursor operating radius
    setLedDefault();
    canOutputAction = true;
    g_resetCenterComplete = true;
    // The INPUT poll comes back unconditionally (4.5.7 review): safe mode
    // never disabled it (see step 0), and it is the only hardware input a
    // safe boot has.
    pollTimer.enable(pollTimerId[CONF_TIMER_INPUT]);
    if (!g_safeModeEnabled) {
      // The sensor poll timers and the menu timeout stay inside the
      // safe-mode guard: the API is not filtered by safe mode, and
      // re-enabling these on a safe boot would run 20 ms I2C traffic
      // against whatever hardware toggleSafeMode shut down, and re-arm the
      // 5-minute timeout the safe menu deliberately disables.
      screen.enableTimeout();  // The run may have been menu-driven — see step 0
      pollTimer.enable(pollTimerId[CONF_TIMER_JOYSTICK]);     // Re-Enable joystick data polling
      pollTimer.enable(pollTimerId[CONF_TIMER_SCROLL]);       // Re-enable scroll data polling
      pollTimer.enable(pollTimerId[CONF_TIMER_PRESSURE]);
    }
    screen.fullCalibrationPrompt(stepNumber);  // update
    g_calibrationError = false;
  }
}
//***PERFORM JOYSTICK CALIBRATION STEP FUNCTION***//
// Function   : performJoystickCalibrationStep
//
// Description: This function performs the actual joystick maximum point calibration step in a recursive fashion.
//
// Parameters : * args : int : pointer of step number
//
// Return     : void
//****************************************//
void performJoystickCalibrationStep(int* args) {
  // See performJoystickCalibration(): the payload carries this chain's
  // initiator alongside the step number — the corner line is an ack (SUCCESS)
  // only when the serial API started the calibration, MANUAL otherwise.
  bool calibFromApi = ((int)args & CONF_CALIB_API_FLAG) != 0;
  int stepNumber = (int)args & ~CONF_CALIB_API_FLAG;
  String stepKey = "CA" + String(stepNumber);       // Key to write new calibration point to Flash memory
  String stepCommand = "CA," + String(stepNumber);  // Command to output calibration point via serial
  pointFloatType maxPoint;
  int magnetZDirection = js.getMagnetZDirection();
  // A FAULT latch (0) must not zero the fallback corners (4.5.7 review):
  // the default-corner geometry multiplies by this direction, and a 0 here
  // replaced every fallback corner with {0,0} — a stored corner that then
  // failed setMinimumRadius's validity check and left the radius at the
  // full 30 mT default (a stiff, barely-responsive cursor). Same
  // DEFAULT-polarity fallback the boot path uses (setMagnetDirection): a
  // briefly wrong axis beats a degenerate corner.
  if (magnetZDirection == JOY_DIRECTION_FAULT) magnetZDirection = JOY_DIRECTION_DEFAULT;

  //Serial.print("Step started: ");   // TODO: Remove these print statements
  //Serial.print(stepNumber);
  //Serial.print(" numRuns: ");
  //Serial.println(calibrationTimer.getNumRuns(calibrationTimerId[1]));

  // Turn on and set all leds to orange to indicate start of the process // TODO Jake update to non-neopixel LEDS
  if (calibrationTimer.getNumRuns(calibrationTimerId[1]) == 1) {  // Turn LLED's ON when timer is running for first time
    setLedState(LED_ACTION_ON, CONF_JOY_CALIB_LED_COLOR, CONF_JOY_CALIB_LED_NUMBER, 0, 0, led.getLedBrightness());
    performLedAction(ledCurrentState);
  }

  maxPoint = js.getInputMax(stepNumber);  // Get maximum x and y for the step number

  // Check for calibration errors
  if ((abs(maxPoint.x) < CONF_JOY_CALIB_CORNER_MIN) || (abs(maxPoint.y) < CONF_JOY_CALIB_CORNER_MIN)) {
    pointFloatType tempDefaultPoint = { 0, 0 };
    switch (stepNumber) {
      case 1:  // Top left corner
        tempDefaultPoint = { float(CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection, float(-CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection };
        break;
      case 2:  // Top right corner
        tempDefaultPoint = { float(-CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection, float(-CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection };
        break;
      case 3:  // Bottom right corner
        tempDefaultPoint = { float(-CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection, float(CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection };
        break;
      case 4:  // Bottom left corner
        tempDefaultPoint = { float(CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection, float(CONF_JOY_CALIB_CORNER_DEFAULT) * magnetZDirection };
        break;
    }
    maxPoint = tempDefaultPoint;
    js.setInputMax(stepNumber, maxPoint);
    g_calibrationError = true;
  }

  // Turn off all the LEDs to orange to indicate end of the process
  if (calibrationTimer.getNumRuns(calibrationTimerId[1]) == CONF_JOY_CALIB_READING_NUMBER) {  // Turn LED's OFF when timer is running for last time
    // The settings-size budget guard can refuse the store (LSMemory): the
    // corner response must then report FAIL,3, not SUCCESS — otherwise the
    // app's calibration readout would show a corner the next boot forgets.
    bool cornerStored = mem.writePoint(CONF_SETTINGS_FILE, stepKey, maxPoint);
    setLedState(LED_ACTION_OFF, LED_CLR_NONE, CONF_JOY_CALIB_LED_NUMBER, 0, 0, led.getLedBrightness());
    performLedAction(ledCurrentState);
    printResponseFloatPoint(true, calibFromApi, cornerStored, cornerStored ? 0 : 3, stepCommand, true, maxPoint);
    if (g_calibrationError) {
      screen.fullCalibrationPrompt(CONF_JOY_CALIB_ERROR);
      // Feed the watchdog through the deliberate dwell (4.5.7 review):
      // upstream's own 3 s blocking hold (its TODO still asks "Why the
      // delay?") sat exactly at the TIMER3 hang witness's 3 s threshold —
      // a tick landing in the overshoot fired a false "hang witnessed"
      // SREQ mid-calibration. The dwell UX is kept, but the loop keeps the
      // watchdog (and the witness's un-fed-seconds counter) alive.
      unsigned long errorDwellStart = millis();
      while (millis() - errorDwellStart < 3000) {
        if (g_watchdogArmed) {
          watchdogLoop();
        }
        delay(10);
      }
      g_calibrationError = false;
    }
  }
}

//***JOYSTICK LOOP FUNCTION***//
// Function   : joystickLoop
//
// Description: This function handles joystick move and scroll actions.
//
// Parameters : void
//
// Return     : void
//****************************************//
void joystickLoop() {
  //if (USB_DEBUG) { Serial.println("USBDEBUG: joystickLoop"); }
  js.update();  // Request new values

  pointIntType joyOutPoint = js.getXYOut();  // Read the filtered values

  // Cursor hold during sip & puff (CF, roadmap item 11 / upstream #44): the
  // cursor freezes while a mouthpiece input is in progress, so deflecting
  // the stick mid-sip cannot drag the pointer around. The old
  // !g_sapDisabled term is gone (4.5.7 review): the pressure state machine
  // keeps tracking sips with SD set (SD gates ACTIONS, not sensing), and a
  // user physically sipping — head motion perturbing the stick — still
  // deserves CF's protection; the premise "with SD there is no mouthpiece
  // input at all" was false.
  bool sapInProgress = g_freezeCursorDuringSap && (ps.getState().mainState != PRESS_SAP_MAIN_STATE_NONE);

  // Bus-crawl backoff suppresses cursor output entirely (4.5.7 review):
  // during the self-measured backoff the last computed output stays on the
  // buffers, and performJoystick replayed it as relative movement every
  // tick — a user holding a corner when the shared bus wedged got a cursor
  // that flew to and ground against the screen edge for up to 60 s with NO
  // input able to stop it (returning the stick to center changed nothing:
  // the stale output was what replayed). Zero motion until real readings
  // return.
  bool outputSuppressed = js.isPollBackedOff();

  if (g_resetCenterComplete && !sapInProgress && !outputSuppressed) {  // Don't output joystick movement until the center position has been reset
    performJoystick(joyOutPoint);                 // Perform joystick move action
  }

  //if (USB_DEBUG) { Serial.println("USBDEBUG: End of joystickLoop"); }
}

//***PERFORM JOYSTICK FUNCTION***//
// Function   : performJoystick
//
// Description: This function performs joystick move and scroll actions.
//
// Parameters : inputPoint : pointIntType : The output cursor x and y
//
// Return     : void
//****************************************//
void performJoystick(pointIntType inputPoint) {
  pointIntType outputPoint = {0,0};
  
  if (g_operatingMode == CONF_OPERATING_MODE_MOUSE) {
    int maxMouse = js.getMouseSpeedRange();
    outputPoint.x = js.mapRoundInt(inputPoint.x, -CONF_JOY_OUTPUT_XY_MAX, CONF_JOY_OUTPUT_XY_MAX ,-maxMouse, maxMouse);
    outputPoint.y = js.mapRoundInt(inputPoint.y, -CONF_JOY_OUTPUT_XY_MAX, CONF_JOY_OUTPUT_XY_MAX ,-maxMouse, maxMouse);
    // Deflection-based acceleration (roadmap item 9): reshapes the mapped
    // cursor curve per axis; full deflection keeps the full speed of the
    // current speed level, and level 0 (the default) is a no-op.
    outputPoint.x = accelerationModifier(outputPoint.x, maxMouse, acceleration);
    outputPoint.y = accelerationModifier(outputPoint.y, maxMouse, acceleration);
    // 0 = None , 1 = USB , 2 = Wireless
    if (g_comMode == CONF_COM_MODE_USB) {
      (outputAction == CONF_ACTION_SCROLL) ? usbmouse.scroll(scrollModifier(round(inputPoint.y), CONF_JOY_OUTPUT_XY_MAX, g_scrollLevel)) : usbmouse.move(outputPoint.x, outputPoint.y);

    } else if (g_comMode == CONF_COM_MODE_BLE) {
      (outputAction == CONF_ACTION_SCROLL) ? btmouse.scroll(scrollModifier(round(inputPoint.y), CONF_JOY_OUTPUT_XY_MAX, g_scrollLevel)) : btmouse.move(outputPoint.x, outputPoint.y);
    }
  } else if (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) {
    // Gamepad is USB only, if wireless gamepad functionality is added, add that here
    outputPoint.x = js.mapRoundInt(inputPoint.x, -CONF_JOY_OUTPUT_XY_MAX, CONF_JOY_OUTPUT_XY_MAX ,-CONF_JOY_OUTPUT_XY_MAX_GAMEPAD, CONF_JOY_OUTPUT_XY_MAX_GAMEPAD);
    outputPoint.y = js.mapRoundInt(inputPoint.y, -CONF_JOY_OUTPUT_XY_MAX, CONF_JOY_OUTPUT_XY_MAX ,-CONF_JOY_OUTPUT_XY_MAX_GAMEPAD, CONF_JOY_OUTPUT_XY_MAX_GAMEPAD);
    gamepad.move(outputPoint.x, outputPoint.y);
    gamepad.send();
  }
}

//***SCROLL MOVEMENT MODIFIER FUNCTION***//
// Function   : scrollModifier
//
// Description: This function converts y cursor movements to y scroll movements based on y cursor value and scroll speed level.
//
// Parameters : cursorValue : const int : y cursor value.
//              cursorMaxValue : const int : maximum y cursor value.
//              scrollLevelValue : const int : scroll speed level value.
//
// Return     : cursorOutput : int : The modified scroll value.
//****************************************//
int scrollModifier(const int cursorValue, const int cursorMaxValue, const int scrollLevelValue) {
  int scrollOutput = 0;

  //int scrollMaxSpeed = round((1.0 * pow(CONF_SCROLL_MOVE_MAX, scrollLevelValue / 10.0)) + CONF_SCROLL_MOVE_BASE);
  //int scrollMaxSpeed = round((1.0 * pow(CONF_SCROLL_MOVE_MAX, scrollLevelValue / CONF_SCROLL_LEVEL_MAX)) + CONF_SCROLL_MOVE_BASE);
  int scrollMaxSpeed = round((1.0 * CONF_SCROLL_MOVE_MAX * scrollLevelValue/CONF_SCROLL_LEVEL_MAX) + CONF_SCROLL_MOVE_BASE); // Max scroll speed at a given scroll level

  //scrollOutput = map(cursorValue, 0, cursorMaxValue, 0, scrollMaxSpeed);
  //scrollOutput = -1 * constrain(scrollOutput, -1 * scrollMaxSpeed, scrollMaxSpeed);

  scrollOutput = round(float(cursorValue) * float(scrollMaxSpeed) / float(cursorMaxValue));     // Actual scroll output based on amount of joystick movement
  scrollOutput = -1 * constrain(scrollOutput, -1 * scrollMaxSpeed, scrollMaxSpeed);


  //Serial.print(cursorValue);  Serial.print("\t");
  //Serial.print(cursorMaxValue); Serial.print("\t");
  //Serial.println(scrollOutput);  

  // 4.5.7 review: at scroll level 10 the max speed is 17, so a strong-but-
  // not-full deflection (|y| in ~[934, 993] of 1024) computed
  // |scrollOutput| == 16 and this modulus became X % 0 — one pulse, then
  // dead scroll while the deflection stayed in that band (and a HardFault
  // on any future core with DIV_0_TRP). Clamp the period to >= 1: full
  // pulse rate at the top of the band.
  int scrollPulsePeriod = CONF_SCROLL_MOVE_MAX - abs(scrollOutput);
  if (scrollPulsePeriod < 1) scrollPulsePeriod = 1;
  if (g_scrollNumRuns % scrollPulsePeriod == 0){
    if (cursorValue < 0) {
      scrollOutput = 1;
    } else if (cursorValue > 0){
      scrollOutput = -1;
    } else { 
      scrollOutput = 0;
    }
    g_scrollNumRuns = 0;
  } else {
    scrollOutput = 0;
  }

  g_scrollNumRuns++;
  return scrollOutput;
}


//***ACCELERATION MOVEMENT MODIFIER FUNCTION***//
// Function   : accelerationModifier
//
// Description: Deflection-based cursor response curve (roadmap item 9,
//              upstream #86). The stored level (-10..10, default 0) bends
//              the deflection-to-speed curve while leaving both endpoints
//              fixed: zero deflection stays zero, and FULL deflection
//              always maps to the full speed of the current cursor speed
//              level — acceleration trades mid-range precision for speed,
//              never top speed.
//                level  0  → linear (identical to the pre-acceleration output)
//                level 10  → exponent 0.0: any deflection moves at ~full speed
//                level -10 → exponent 2.0: gentle deflections move very slowly
//              The exponent moves 0.1 per level:
//                out = sign(v) · max · (|v| / max) ^ (1 − level·0.1)
//              Host-side acceleration varies per device and affects other
//              mice when tuned for the LipSync — this keeps the curve on the
//              device where the speed level already lives.
//
// Parameters : cursorValue : const int : The mapped cursor axis value (-max..max).
//              cursorMaxValue : const int : The maximum cursor axis value.
//              accelerationValue : const int : The acceleration level (-10..10).
//
// Return     : cursorOutput : int : The reshaped cursor axis value.
//****************************************//
int accelerationModifier(const int cursorValue, const int cursorMaxValue, const int accelerationValue) {
  if (cursorValue == 0 || cursorMaxValue <= 0 || accelerationValue == 0) {
    return cursorValue;  // Zero maps to zero; level 0 is the identity curve
  }

  float exponent = 1.0f - (accelerationValue * 0.1f);
  if (exponent < 0.0f) exponent = 0.0f;
  if (exponent > 2.0f) exponent = 2.0f;

  float magnitude = fabs((float)cursorValue);
  float scaled = pow(magnitude / (float)cursorMaxValue, exponent) * (float)cursorMaxValue;
  int accelerationOutput = (int)round(scaled);
  return (cursorValue < 0) ? -accelerationOutput : accelerationOutput;
}


//*********************************//
// Debug Functions
//*********************************//

//***INITIALIZE DEBUG FUNCTION***//
// Function   : initDebug
//
// Description: This function initializes debug mode and debug state.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initDebug() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initDebug()"); }
  g_debugMode = getDebugMode(false, false);  // Get debug mode number stored in flash memory
  setDebugState(g_debugMode);                // Set debug operation state based on the debug mode
}

//***DEBUG LOOP FUNCTION***//
// Function   : debugLoop
//
// Description: This function outputs debug data based on debug mode.
//
// Parameters : void
//
// Return     : void
//****************************************//
void debugLoop() {
  //if (USB_DEBUG) { Serial.println("USBDEBUG: debugLoop"); }
  // Debug mode is off if the debug mode is #0
  if (g_debugMode == CONF_DEBUG_MODE_JOYSTICK) {  // Debug #1
    js.update();                                  // Request new values from joystick class
    pointFloatType debugJoystickArray[3];
    debugJoystickArray[0] = js.getXYRaw();                                       // Read the raw values
    debugJoystickArray[1] = { (float)js.getXYIn().x, (float)js.getXYIn().y };    // Read the filtered values
    debugJoystickArray[2] = { (float)js.getXYOut().x, (float)js.getXYOut().y };  // Read the output values
    printResponseFloatPointArray(true, true, true, 0, "DEBUG,1", true, "", 3, ',', debugJoystickArray);
  } else if (g_debugMode == CONF_DEBUG_MODE_PRESSURE) {  // Debug #2
    ps.update();                                       // Request new pressure difference from sensor and push it to array
    float debugPressureArray[4];
    debugPressureArray[0] = ps.getSapPressureAbs();   // Read the main pressure
    debugPressureArray[1] = ps.getAmbientPressure();  // Read the ref pressure
    debugPressureArray[2] = ps.getSapPressure();      // Read the diff pressure
    debugPressureArray[3] = ps.getOffsetPressure();      // Read the diff pressure    
    printResponseFloatArray(true, true, true, 0, "DEBUG,2", true, "", 4, ',', debugPressureArray);
  } else if (g_debugMode == CONF_DEBUG_MODE_BUTTON) {  // Debug #3
    int debugButtonArray[3];
    debugButtonArray[0] = buttonState.mainState;         // Read the main state
    debugButtonArray[1] = buttonState.secondaryState;    // Read the secondary state
    debugButtonArray[2] = (int)buttonState.elapsedTime;  // Read the Elapsed Time
    printResponseIntArray(true, true, true, 0, "DEBUG,3", true, "", 3, ',', debugButtonArray);
  } else if (g_debugMode == CONF_DEBUG_MODE_SWITCH) {  // Debug #4
    int debugSwitchArray[3];
    debugSwitchArray[0] = switchState.mainState;         // Read the main state
    debugSwitchArray[1] = switchState.secondaryState;    // Read the secondary state
    debugSwitchArray[2] = (int)switchState.elapsedTime;  // Read the Elapsed Time
    printResponseIntArray(true, true, true, 0, "DEBUG,4", true, "", 3, ',', debugSwitchArray);
  } else if (g_debugMode == CONF_DEBUG_MODE_SAP) {  // Debug #5
    int debugSapArray[3];
    debugSapArray[0] = sapActionState.mainState;         // Read the main state
    debugSapArray[1] = sapActionState.secondaryState;    // Read the secondary state
    debugSapArray[2] = (int)sapActionState.elapsedTime;  // Read the Elapsed Time
    printResponseIntArray(true, true, true, 0, "DEBUG,5", true, "", 3, ',', debugSapArray);
  }
}

//***SET DEBUG STATE FUNCTION***//
// Function   : setDebugState
//
// Description: This function configures the state of operation based on the debug mode
//
// Parameters : inputDebugMode : int : The current debug mode
//
// Return     : void
//****************************************//
void setDebugState(int inputDebugMode) {
  if (USB_DEBUG) {
    Serial.print("USBDEBUG: setDebugState( ");
    Serial.print(inputDebugMode);
    Serial.println(" )");
  }

  if (inputDebugMode == CONF_DEBUG_MODE_NONE) {
    pollTimer.disable(pollTimerId[CONF_TIMER_DEBUG]);    // Disable debug data polling
    // 4.5.7 review: the sensor-poll re-enables are safe-mode-gated — the
    // serial API is deliberately not filtered by safe mode, and a DM,1:0 on
    // a sensorful safe boot used to restore the 20 ms/30 ms joystick/scroll
    // I2C polls toggleSafeMode shut down, for the rest of the boot.
    if (!g_safeModeEnabled) {
      pollTimer.enable(pollTimerId[CONF_TIMER_JOYSTICK]);  // Enable joystick data polling
      pollTimer.enable(pollTimerId[CONF_TIMER_SCROLL]);    // Enable scroll data polling
    }

  } else if (inputDebugMode == CONF_DEBUG_MODE_JOYSTICK) {
    pollTimer.disable(pollTimerId[CONF_TIMER_JOYSTICK]);  // Disable joystick data polling
    pollTimer.disable(pollTimerId[CONF_TIMER_SCROLL]);    // Disable scroll data polling
    pollTimer.enable(pollTimerId[CONF_TIMER_DEBUG]);      // Enable debug data polling
  } else {
    pollTimer.enable(pollTimerId[CONF_TIMER_DEBUG]);  // Enable debug data polling
  }
}


//***CHECK I2C FUNCTION***//
// Function   : checkI2C
//
// Description: This function checks to ensure the necessary i2C devices are connected.
//
// Parameters : void
//
// Return     : void
//****************************************//
void checkI2C() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: checkI2C"); }

  Wire.begin();

  Wire.beginTransmission(I2CADDR_DISPLAY);
  byte error_display;
  error_display = Wire.endTransmission();

  if (error_display == 0) {
    g_displayConnected = true;  // Display found
  } else {
    Serial.println("ERROR: Display: Not found");
    g_displayConnected = false;
  }

  // Scan for Ambient Pressure Sensor
  Wire.beginTransmission(I2CADDR_LPS22);
  byte error_LPS22;
  error_LPS22 = Wire.endTransmission();

  if (error_LPS22 == 0) {
    g_ambientPressureSensorConnected = true;  // Ambient pressure sensor found
  } else {
    Serial.println("ERROR: Ambient Pressure Sensor: Not found");
    g_ambientPressureSensorConnected = false;
  }

  // Scan for Sip and Puff Sensor
  Wire.beginTransmission(I2CADDR_LPS35HW);
  byte error_LPS35HW;
  error_LPS35HW = Wire.endTransmission();

  if (error_LPS35HW == 0) {
    g_mouthpiecePressureSensorConnected = true;  // Sip and Puff Sensor Found
  } else {
    Serial.println("ERROR: Mouthpiece Pressure Sensor: Not found");
    g_mouthpiecePressureSensorConnected = false;
  }

  // Scan for Joystick Sensor
  Wire.beginTransmission(I2CADDR_TLV493D);
  byte error_TLV493D;
  error_TLV493D = Wire.endTransmission();

  if (error_TLV493D == 0) {
    g_joystickSensorConnected = true;  // Joystick sensor found
  } else {
    Serial.println("ERROR: Joystick Sensor: Not found");
    g_joystickSensorConnected = false;
  }
  // No Wire.end() here (4.5.4): this used to disable the TWIM peripheral
  // behind it — scanner-example residue with no consumer — and the bus only
  // worked afterwards because the display's and the pressure sensors'
  // begin()s happen to call Wire.begin() as a side effect. The XENSIV
  // migration deliberately stopped the joystick library from re-beginning
  // the shared bus (executeInit=false), which left the joystick-only
  // degraded boot reading against a dead controller until the vendored
  // Wire's bounded recovery re-enabled it. One owner instead: the bus comes
  // up at the Wire.begin() above and stays up for every consumer.
}


//*********************************//
// LED Functions
//*********************************//

//***INITIALIZE LED FUNCTION***//
// Function   : initLed
//
// Description: This function initializes LED output as feedback method.
//
// Parameters : void
//
// Return     : void
//****************************************//
void initLed() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: Initializing LEDs"); }
  led.begin();
  *ledCurrentState = { 0, 0, 0, 0, 0, 0 };  // Initialize pointer ledAction, ledColorNumber, ledBlinkNumber, ledBlinkTime, ledBrightness
  turnLedAllOff();

  led.setLightModeLevel(getLightMode(false, false));
  led.setLedBrightnessLevel(getLightBrightnessLevel(false, false)); 
}


//***LED WAIT FEEDBACK FUNCTION***//
// Function   : ledWaitFeedback
//
// Description: This function sets the LEDS to cue the user to wait for initialization.
//
// Parameters : void
//
// Return     : void
//****************************************//
void ledWaitFeedback() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: All LEDS on"); }
  setLedState(LED_ACTION_ON, LED_CLR_RED, CONF_LED_ALL, 0, 0, led.getLedBrightness());  // Turn on all LEDS
  performLedAction(ledCurrentState);
}

//***LED READY FEEDBACK FUNCTION***//
// Function   : ledReadyFeedback
//
// Description: This function sets the LEDS to cue the user to wait for initialization.
//
// Parameters : void
//
// Return     : void
//****************************************//
void ledReadyFeedback() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: ledReadyFeedback"); }
  setLedState(LED_ACTION_OFF, LED_CLR_NONE, CONF_LED_ALL, 0, 0, led.getLedBrightness());    // Turn off all LEDS
  setLedState(LED_ACTION_ON, LED_CLR_GREEN, CONF_LED_MICRO, 0, 0, led.getLedBrightness());  // Turn micro LED green
  performLedAction(ledCurrentState);
}

//***STARTUP INITIALIZATION LED FUNCTION***//
// Function   : startupFeedback
//
// Description: This function performs the startup initialization feedback.
//
// Parameters : void
//
// Return     : void
//****************************************//
void startupFeedback() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: startupFeedback"); }
  ledTimerId[CONF_TIMER_LED_STARTUP] = ledStateTimer.setTimeout(ledCurrentState->ledBlinkTime, ledStartupEffect, ledCurrentState);

  //ledTimerId[CONF_TIMER_LED_ERROR] = ledStateTimer.setTimer(CONF_ERROR_LED_BLINK_DELAY, blinkStartDelay, ((errorBlinks * 2) + 1), ledErrorEffect);
}

//***SET LED STATE FUNCTION***//
// Function   : setLedState
//
// Description: This function sets LED states based on led number, blink number, color number, blink delay , and brightness.
//
// Parameters : ledAction      : int : led action ( NOTHING , ON , OFF , BLINK )
//              ledColorNumber : int : led color number
//              ledNumber      : int : led number ( 1, 2, 3, 4 or all )
//              ledBlinkNumber : int : led blink number
//              ledBlinkTime : unsigned long : led blink delay time
//              ledBrightness : int : led brightness
//
// Return     : void
//****************************************//
void setLedState(int ledAction, int ledColorNumber, int ledNumber, int ledBlinkNumber, unsigned long ledBlinkTime, int ledBrightness) {
  if (ledNumber <= OUTPUT_LED_NUM + 1) {  // Valid ledNumber
    ledCurrentState->ledAction = ledAction;
    ledCurrentState->ledColorNumber = ledColorNumber;
    ledCurrentState->ledNumber = ledNumber;
    ledCurrentState->ledBlinkNumber = ledBlinkNumber;
    ledCurrentState->ledBlinkTime = ledBlinkTime;
    ledCurrentState->ledBrightness = ledBrightness;
  }
}


//***LED STARTUP EFFECT FUNCTION***//
// Function   : ledStartupEffect
//
// Description: This function performs the Startup LED effect based on step number.
//
// Parameters : args : ledStateStruct* : It includes step number.
//
// Return     : void
//****************************************//
void ledStartupEffect(ledStateStruct* args) {



  // setLedState(LED_ACTION_ON,
  //             ledActionProperty[tempActionIndex].ledStartColor,
  //             ledActionProperty[tempActionIndex].ledNumber,
  //             0,                     // number of blinks
  //             0,                     // blink time
  //             CONF_LED_BRIGHTNESS);  // brightness
  // // Perform led action
  // performLedAction(ledCurrentState);

  // if (args->ledColorNumber < MAX_LED_COLORS) {
  //   led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
  //   setLedState(args->ledAction, (args->ledColorNumber) + 1, args->ledNumber, args->ledBlinkNumber, (args->ledBlinkTime), args->ledBrightness);
  //   ledTimerId[CONF_TIMER_LED_STARTUP] = ledStateTimer.setTimeout(ledCurrentState->ledBlinkTime, ledStartupEffect, ledCurrentState);
  // } else if (args->ledColorNumber == MAX_LED_COLORS) {
  //   ledActionEnabled = true;
  //   enablePoll(true);
  // }
}

//***LED IBM EFFECT FUNCTION***//
// Function   : ledIBMEffect
//
// Description: This function performs the IBM LED effect based on step number.
//
// Parameters : args : ledStateStruct* : It includes step number.
//
// Return     : void
//****************************************//
void ledIBMEffect(ledStateStruct* args) {
  const int MAX_LED_COLORS = 7;
  if (args->ledColorNumber < MAX_LED_COLORS) {
    led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
    setLedState(args->ledAction, (args->ledColorNumber) + 1, args->ledNumber, args->ledBlinkNumber, (args->ledBlinkTime), args->ledBrightness);
    ledTimerId[CONF_TIMER_LED_IBM] = ledStateTimer.setTimeout(ledCurrentState->ledBlinkTime, ledIBMEffect, ledCurrentState);
  } else if (args->ledColorNumber == MAX_LED_COLORS) {
    ledActionEnabled = true;
    enablePoll(true);
  }
}

//***LED BLINK EFFECT FUNCTION***//
// Function   : ledBlinkEffect
//
// Description: This function performs the blink LED effect based on step number, led number passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes step number, LED number.
//
// Return     : void
//****************************************//
void ledBlinkEffect(ledStateStruct* args) {
  if (ledStateTimer.getNumRuns(ledTimerId[CONF_TIMER_LED_BLINK]) % 2) {
    led.setLedColor(args->ledNumber, 0, args->ledBrightness);
  } else {
    led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
  }

  if (ledStateTimer.getNumRuns(ledTimerId[CONF_TIMER_LED_BLINK]) == ((args->ledBlinkNumber) * 2) + 1) {
    setLedDefault();  // return all LEDs to default after blink finished
    // 4.5.7 review: the blink self-deleted its slot — clear the id so the
    // stale name can never make a later blinkLed's delete-before-rearm hit
    // a RECYCLED slot (e.g. the BT scan timer that took the freed slot in
    // BLE-disconnected mode).
    ledTimerId[CONF_TIMER_LED_BLINK] = -1;
  }
}

//***LED BLUETOOTH SCAN EFFECT FUNCTION***//
// Function   : ledBtScanEffect
//
// Description: This function performs bluetooth scan blink LED effect.
//
// Return     : void
//****************************************//
void ledBtScanEffect() {
  if (ledStateTimer.getNumRuns(ledTimerId[CONF_TIMER_LED_BT]) % 2) {
    led.setLedColor(CONF_BT_LED_NUMBER, LED_CLR_NONE, CONF_BT_LED_BRIGHTNESS);

  } else {
    led.setLedColor(CONF_BT_LED_NUMBER, CONF_BT_LED_COLOR, CONF_BT_LED_BRIGHTNESS);
  }
}

//***LED BLUETOOTH FAILED EFFECT FUNCTION***//
// Function   : ledBtFailedEffect
//
// Description: This function performs the BLE-init-failed blink (red) —
// btFeedbackLoop re-arms it per poll while the init task has reported
// failure, the logless user's "Bluetooth is broken" cue (bench 2026-10-26:
// the user watched a fully dark device and had no way to know the stack
// had never come up). Reuses the CONF_TIMER_LED_BT slot — it can never
// coexist with the scan blink above: the scan blink requires
// needsInitialization false, this one requires it true.
//
// Return     : void
//****************************************//
void ledBtFailedEffect() {
  if (ledStateTimer.getNumRuns(ledTimerId[CONF_TIMER_LED_BT]) % 2) {
    led.setLedColor(CONF_BT_LED_NUMBER, LED_CLR_NONE, CONF_BT_LED_BRIGHTNESS);

  } else {
    led.setLedColor(CONF_BT_LED_NUMBER, LED_CLR_RED, CONF_BT_LED_BRIGHTNESS);
  }
}

//***LED ERROR EFFECT FUNCTION***//
// Function   : ledErrorEffect
//
// Description: This function performs LED error ffect.
//
// Return     : void
//****************************************//
void ledErrorEffect() {
  if (ledStateTimer.getNumRuns(ledTimerId[CONF_TIMER_LED_ERROR]) % 2) {
    led.setLedColor(CONF_ERROR_LED_NUMBER, LED_CLR_NONE, CONF_ERROR_LED_BRIGHTNESS);

  } else {
    led.setLedColor(CONF_ERROR_LED_NUMBER, CONF_ERROR_LED_COLOR, CONF_ERROR_LED_BRIGHTNESS);
  }
}

//***LED ERROR FUNCTION***//
// Function   : ledError
//
// Description: This function displays LED sequence for hardware error
//
// Return     : void
//****************************************//
void ledError(int errorBlinks) {
  const int blinkStartDelay = 0;
  ledTimerId[CONF_TIMER_LED_ERROR] = ledStateTimer.setTimer(CONF_ERROR_LED_BLINK_DELAY, blinkStartDelay, ((errorBlinks * 2) + 1), ledErrorEffect);
}

//***TURN ALL LEDS OFF FUNCTION***//
// Function   : turnLedAllOff
//
// Description: This function turns off all the LEDs.
//
// Parameters : void
//
// Return     : void
//****************************************//
void turnLedAllOff() {
  led.clearLedAll();
}

//***TURN LED OFF FUNCTION***//
// Function   : turnLedOff
//
// Description: This function turns off the specified LED based on led number passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number.
//
// Return     : void
//****************************************//
void turnLedOff(ledStateStruct* args) {
  led.clearLed(args->ledNumber);
}

//***TURN LED ON FUNCTION***//
// Function   : turnLedOff
//
// Description: This function turns on the specified LED based on led number passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number.
//
// Return     : void
//****************************************//
void turnLedOn(ledStateStruct* args) {
  led.setLedColor(args->ledNumber, args->ledColorNumber, args->ledBrightness);
}


//***BLINK LED FUNCTION***//
// Function   : blinkLed
//
// Description: This function blinks the specified LED based on led number and number of blinks passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number and number of blinks.
//
// Return     : void
//****************************************//
static ledStateStruct blinkStateSnapshot;  // 4.5.7 review: the running blink reads ITS OWN captured parameters — see blinkLed

void blinkLed(ledStateStruct* args) {
  int blinkOffset = 0;  // number of milliseconds to delay start of blink
  // 4.5.7 review: delete the superseded blink before re-arming — the id was
  // simply overwritten, so a still-pending long blink (the 3 x 1000 ms
  // com-mode/brightness cues) kept firing after a newer, shorter blink
  // self-deleted its slot: ledBlinkEffect then read getNumRuns of the FREED
  // slot (0), took the else branch, lit the stale action's LED, and never
  // reached its setLedDefault cleanup — an LED stuck ON until an unrelated
  // event. The new blink's own final run performs the default restore, so
  // deleting the old tail is the correct supersede semantics. (The BT scan
  // blink deliberately re-arms per poll and must NOT do this — deletion
  // there would reset its cadence every second.)
  if (ledTimerId[CONF_TIMER_LED_BLINK] >= 0) {
    ledStateTimer.deleteTimer(ledTimerId[CONF_TIMER_LED_BLINK]);
  }
  // Snapshot the parameters (4.5.7 review): the effect used to dereference
  // the SHARED ledCurrentState on every fire, so any later setLedState (a
  // hold's 20 ms re-assert, a mode cue) hijacked the pending blink's
  // number/color/LED mid-run and misrouted its cleanup. The blink now runs
  // on its own captured copy; only one blink is ever live (the
  // delete-before-rearm above supersedes).
  blinkStateSnapshot = *args;
  ledTimerId[CONF_TIMER_LED_BLINK] = ledStateTimer.setTimer(blinkStateSnapshot.ledBlinkTime, blinkOffset, ((blinkStateSnapshot.ledBlinkNumber) * 2) + 1, ledBlinkEffect, &blinkStateSnapshot);
}

//***SET DEFAULT LED EFFECT FUNCTION***//
// Function   : setLedDefault
//
// Description: This function sets the LED to default operating color based on led number and color passed by arguments.
//
// Parameters : args : ledStateStruct* : It includes led number and color.
//
// Return     : void
//****************************************//
void setLedDefault() {
  led.clearLedAll();  // Clear all LEDs

  switch (g_operatingMode) {
    case CONF_OPERATING_MODE_MOUSE:
      {
        if (g_comMode == CONF_COM_MODE_USB) {
          led.setLedColor(CONF_LED_MICRO, LED_CLR_PURPLE, led.getLedBrightness());  // USB: micro LED purple (stock v4.1 visual, kept — the v4.1 comment claimed this was the BLE indication, but the branch conditions and their comments were swapped)
        } else if (g_comMode == CONF_COM_MODE_BLE && btmouse.isConnected()) {    // BLE connected: micro LED solid blue — while DISCONNECTED btFeedbackLoop blinks it blue when advertising is up, red when init failed, dark while init is in flight
          led.setLedColor(CONF_BT_LED_NUMBER, LED_CLR_BLUE, led.getLedBrightness());
        }
        break;
      }
    case CONF_OPERATING_MODE_GAMEPAD:
      {
        led.setLedColor(CONF_LED_MICRO, LED_CLR_YELLOW, led.getLedBrightness());
        break;
      }
    case CONF_OPERATING_MODE_SAFE:
      {
       led.setLedColor(CONF_LED_ALL, LED_CLR_RED, led.getLedBrightness());
       break;
      }
  }
}

//***BLUETOOTH SCAN AND LED FEEDBACK LOOP FUNCTION***//
// Function   : btFeedbackLoop
//
// Description: This function performs the default LED effects to indicate the device is connected.
//
// Parameters : void
//
// Return     : void
//****************************************//
void btFeedbackLoop() {

  //if (USB_DEBUG) { Serial.println("USBDEBUG: btFeedbackLoop"); }

  // The BLE connection poll only matters in Bluetooth mode. In USB mode the
  // call is wasted — and on safe-mode boots the Bluefruit stack may never
  // have been initialized, where btmouse.isConnected() blocks the SoftDevice
  // for a full second (observed live as a flat 1 s main-loop tax).
  if (g_comMode != CONF_COM_MODE_BLE) return;
  // needsInitialization: the SoftDevice must actually have been begun (4.5.7
  // review) — a stored/live state with CM=BLE but no BLE begin() blocks 1 s
  // per poll on isConnected(). 4.5.13: if the init task has FINISHED while
  // needsInitialization is still true, the stack never came up (ble=3) —
  // blink the BT LED red per poll so a user without a log sees the degraded
  // state instead of a dark device; re-armed without deleting exactly like
  // the scan blink below (a delete would reset the cadence every second).
  // While the init is still in flight (g_bleInitDone false) the state is
  // transient — stay dark and return.
  if (needsInitialization) {
    if (g_bleInitPending && g_bleInitDone) {
      ledTimerId[CONF_TIMER_LED_BT] = ledStateTimer.setTimer(CONF_BT_SCAN_BLINK_DELAY, 0, ((CONF_BT_SCAN_BLINK_NUMBER * 2) + 1), ledBtFailedEffect);
    }
    return;
  }

  // Get the current bluetooth connection state
  bool tempIsConnected = btmouse.isConnected();

  //if (USB_DEBUG) { Serial.println(tempIsConnected); }
  // Perform Bluetooth LED blinking if Bluetooth is not connected and wasn't connected before
  if (g_comMode == CONF_COM_MODE_BLE && tempIsConnected == false && tempIsConnected == g_btIsConnected) {
    g_btIsConnected = false;
    const int blinkStartDelay = 0;
    ledTimerId[CONF_TIMER_LED_BT] = ledStateTimer.setTimer(CONF_BT_SCAN_BLINK_DELAY, blinkStartDelay, ((CONF_BT_SCAN_BLINK_NUMBER * 2) + 1), ledBtScanEffect);

  }  // Set the default LED effect if bluetooth connection state is changed
  else if (g_comMode == CONF_COM_MODE_BLE && tempIsConnected != g_btIsConnected) {
    g_btIsConnected = tempIsConnected;
    setLedDefault();
  }

  //if (USB_DEBUG) { Serial.println("USBDEBUG: end of btFeedbackLoop"); }
}

//***PERFORM LED ACTION FUNCTION***//
// Function   : performLedAction
//
// Description: This function performs LED actions based on led number, blink number, color number, blink delay, and brightness.
//
// Parameters : args : ledStateStruct* : It includes LED number, blink number, color number, blink delay, and brightness.
//
// Return     : void
//****************************************//
void performLedAction(ledStateStruct* args) {
  bootTrailStamp(4);  // witness v7: an LED action is running
  switch (args->ledAction)  // Get LED action
  {
    case LED_ACTION_NONE:
      {
        //setLedDefault(); // TODO Check to see if this should be uncommented
        break;
      }
    case LED_ACTION_OFF:
      {
        turnLedOff(args);
        break;
      }
    case LED_ACTION_ON:
      {
        turnLedOn(args);
        break;
      }
    case LED_ACTION_BLINK:
      {
        blinkLed(args);
        break;
      }

      /* case LED_ACTION_BLINKFAST:  // TODO 2025-Feb-21 Fix or remove blinkfast leds
    {
      blinkLedFast(args);
      break;
    }
    */
  }
}


//***SOFTWARE RESET FUNCTION***//
// Function   : softwareReset
//
// Description: This function initiates a software reset.
//
// Parameters :  none
//
// Return     : none
//******************************************//
void softwareReset() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: softwareReset()"); }
  g_shutdownInProgress = true;
  bootTrailStamp(5);  // witness v7: a software reset is starting — the next boot's pt=5 names this path
  screen.restartPage();
  buzzer.playShutdownSound();

  releaseOutputAction();
  usbmouse.end();
  gamepad.end();
  btmouse.end();

  delay(3000);
  screen.clear();
  
  NVIC_SystemReset();
  
}

