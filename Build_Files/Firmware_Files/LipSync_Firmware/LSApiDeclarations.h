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

// Hub menu, diagnostics and device actions
void controlHubMenu(bool responseEnabled, bool apiEnabled, int inputMenuControl);
void controlHubMenu(bool responseEnabled, bool apiEnabled, String optionalParameter);
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

// ---- Cross-tab free functions (defined in LipSync_Firmware.ino) ----

// Device lifecycle
void softwareReset();
void setDebugState(int inputDebugMode);
void readyToUse();
void errorScreen();

// Buzzer mode helpers called by the Hub menu (LSScreen.h)
void buzzerSoundOn();
void buzzerSoundOff();

// Screen page-transition continuations passed to LSScreen::scheduleTransition()
void clearSplashScreen();
void screenExitTransitionDone();
void screenScrollTransitionDone();
void screenChangeModeTransitionDone();
void screenReturnToMenuTransitionDone();
void screenFullCalibConfirmTransitionDone();
void screenFactoryResetConfirm2TransitionDone();

// Calibration state machine callbacks (timer continuations, referenced by
// the calibration timers and by setJoystickCalibration / setJoystickInitialization)
void performJoystickCenter(int* args);
void performJoystickCenterStep(int* args);
void performJoystickCalibration(int* args);
void performJoystickCalibrationStep(int* args);

#endif // _LSAPIDECLARATIONS_H
