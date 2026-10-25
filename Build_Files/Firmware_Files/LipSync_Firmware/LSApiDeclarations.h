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
void setOperatingMode(bool responseEnabled, bool apiEnabled, int inputOperatingMode);
void setOperatingMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getCommunicationMode(bool responseEnabled, bool apiEnabled);
void setCommunicationMode(bool responseEnabled, bool apiEnabled, int inputCommunicationMode);
void setCommunicationMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getSoundMode(bool responseEnabled, bool apiEnabled);
void setSoundMode(bool responseEnabled, bool apiEnabled, int inputSoundMode);
void setSoundMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getLightMode(bool responseEnabled, bool apiEnabled);
void setLightMode(bool responseEnabled, bool apiEnabled, int inputLightMode);
void setLightMode(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Cursor, scroll and acceleration
int getCursorSpeed(bool responseEnabled, bool apiEnabled);
void setCursorSpeed(bool responseEnabled, bool apiEnabled, int inputSpeedLevel);
void setCursorSpeed(bool responseEnabled, bool apiEnabled, String optionalParameter);
void increaseCursorSpeed(bool responseEnabled, bool apiEnabled);
void decreaseCursorSpeed(bool responseEnabled, bool apiEnabled);
int getScrollLevel(bool responseEnabled, bool apiEnabled);
void setScrollLevel(bool responseEnabled, bool apiEnabled, int inputScrollLevel);
void setScrollLevel(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getJoystickAcceleration(bool responseEnabled, bool apiEnabled);
void setJoystickAcceleration(bool responseEnabled, bool apiEnabled, int inputAccelerationLevel);
void setJoystickAcceleration(bool responseEnabled, bool apiEnabled, String optionalParameter);
void increaseJoystickAcceleration(bool responseEnabled, bool apiEnabled);
void decreaseJoystickAcceleration(bool responseEnabled, bool apiEnabled);

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
void setSipPressureThreshold(bool responseEnabled, bool apiEnabled, float inputSipThreshold);
void setSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);
float getPuffPressureThreshold(bool responseEnabled, bool apiEnabled);
void setPuffPressureThreshold(bool responseEnabled, bool apiEnabled, float inputPuffThreshold);
void setPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter);

// Feedback levels
int getLightBrightnessLevel(bool responseEnabled, bool apiEnabled);
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

// Live Hub-menu state read (CH,0:0, firmware 4.5.1+): [active, pageId, row]
// from LSScreen::fillMenuState — the device-truth source the app's remote
// re-syncs its mirror against after every press (refused selects and
// frozen-window drops both ack SUCCESS, so the state line is the only
// honest answer to "where is the Hub now?").
void getHubMenuState(bool responseEnabled, bool apiEnabled);
void getHubMenuState(bool responseEnabled, bool apiEnabled, String optionalParameter);
int getDebugMode(bool responseEnabled, bool apiEnabled);
void setDebugMode(bool responseEnabled, bool apiEnabled, int inputDebugMode);
void setDebugMode(bool responseEnabled, bool apiEnabled, String optionalParameter);
void exportSettings(bool responseEnabled, bool apiEnabled);
void exportSettings(bool responseEnabled, bool apiEnabled, String optionalParameter);
void runTest(bool responseEnabled, bool apiEnabled, int inputTest);
void runTest(bool responseEnabled, bool apiEnabled, String optionalParameter);
void softReset(bool responseEnabled, bool apiEnabled);
void softReset(bool responseEnabled, bool apiEnabled, String optionalParameter);
void resetSettings(bool responseEnabled, bool apiEnabled);
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

// ---- Cross-tab free functions (defined in LipSync_Firmware.ino) ----

// Device lifecycle
void softwareReset();
void setDebugState(int inputDebugMode);
void readyToUse();
void errorScreen();

// Buzzer mode helpers called by the Hub menu (LSScreen.h)
void buzzerSoundOn();
void buzzerSoundOff();
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
bool switchRemapAllowed(int actionNumber, bool gamepad);  // (4.5.7 review) column-aware: the gamepad cells reject the mouse hold modes
void sanitizeInputWindows();                              // (4.5.7 review) boot-load validation for WS/WL/WM
void switchRemapApplyCell(int jack, int window, bool gamepad, int actionNumber);
int switchRemapDefaultCell(int jack, int window, bool gamepad);

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

// Calibration state machine callbacks (timer continuations, referenced by
// the calibration timers and by setJoystickCalibration / setJoystickInitialization)
void performJoystickCenter(int* args);
void performJoystickCenterStep(int* args);
void performJoystickCalibration(int* args);
void performJoystickCalibrationStep(int* args);

#endif // _LSAPIDECLARATIONS_H
