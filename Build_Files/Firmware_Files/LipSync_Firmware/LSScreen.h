/* 
* File: LSScreen.h
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

// Header definition
#ifndef _LSSCREEN_H
#define _LSSCREEN_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "LSApiDeclarations.h"

#define CONF_SCREEN_WIDTH 128  // OLED display width, in pixels
#define CONF_SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define MAIN_MENU 0
#define EXIT_MENU 1
#define CALIB_MENU 2
#define MODE_MENU 3
#define CURSOR_SP_MENU 4
#define MORE_MENU 5
#define SAFEMODE_MENU 6

// Calibration pages
#define CENTER_RESET_PAGE 21

// Informational dwell pages (4.5.7 review — the CH,0:0 honesty fix): these
// draw-only pages previously left _currentMenu at the page they
// interrupted, so a remote polling after its press saw the OLD page for
// the whole dwell (the exact drift class the endpoint exists to prevent).
// Each names itself for its window, exactly as MODE_LOCKED_PAGE does; the
// continuation restores the real page id.
#define EXITING_PAGE 11          // "Exiting" (0.5 s) -> finishExitMenu
#define CHANGING_MODE_PAGE 33    // "Changing mode." (2 s) -> finishChangeMode (softwareReset)
#define FULL_CALIB_WARN_PAGE 552   // "Incorrect full calib. may cause drift." (3 s) -> fullCalibrationConfirmMenu
#define FACTORY_RESET_WARN_PAGE 572 // "This will erase all custom settings" (2 s) -> factoryResetConfirm2Menu

// Mode pages
#define CONFIRM_MODE_CHANGE 31
// The LK refusal interstitial (bench follow-up 2026-10-18, CH,0:0): the
// locked page previously left _currentMenu at MODE_MENU, so a remote could
// not tell "refused, showing why" from "still on the Mode page" — exactly
// the state the menu-sync endpoint exists to disambiguate. Input stays
// frozen for the page's dwell; finishModeLocked's modeMenu() restores
// MODE_MENU.
#define MODE_LOCKED_PAGE 32

// More Menus
#define SOUND_MENU 51
#define LIGHT_BRIGHT_MENU 52
#define SCROLL_SP_MENU 53
#define SIP_PUFF_MENU 54
#define SIP_THRESH_MENU 541
#define PUFF_THRESH_MENU 542
#define FULL_CALIB_PAGE 55
#define FULL_CALIB_CONFIRM_PAGE 551
#define RESTART_PAGE 56
#define FACTORY_RESET_PAGE 57
#define FACTORY_RESET_CONFIRM2_PAGE 571
#define INFO_MENU 58
#define ACCEL_MENU 59
#define RESTRICTIONS_MENU 60
#define TIMING_MENU 61
#define TIMING_ADJUST_MENU 611

#define SCROLL_DELAY_MILLIS 100 // [ms] This controls the scroll speed of long menu items //TODO 2025-Feb-28 Make this user adjustable

#define _MODE_MOUSE_USB 1
#define _MODE_MOUSE_BT 2
#define _MODE_GAMEPAD_USB 3

const int CHAR_PIXEL_HEIGHT_S1 = 8;    // The height of a character on the screen, in pixels, for size 1 text
const int CHAR_PIXEL_WIDTH_S1 = 6;     // The width of a character on the screen, in pixels, for size 1 text

const int CHAR_PIXEL_HEIGHT_S2 = CHAR_PIXEL_HEIGHT_S1 * 2;    // The height of a character on the screen, in pixels, for size 2 text
const int CHAR_PIXEL_WIDTH_S2  = CHAR_PIXEL_WIDTH_S1 * 2;     // The width of a character on the screen, in pixels, for size 2 text

const int TEXT_ROWS = CONF_SCREEN_HEIGHT / CHAR_PIXEL_HEIGHT_S2;

int scrollPixelsPerLoop = 4;

// Define globals
extern bool g_displayConnected;                   // Display connection state
extern bool g_joystickSensorConnected;            // Joystick sensor connection state
extern bool g_mouthpiecePressureSensorConnected;  // Mouthpiece pressure sensor connection state
extern bool g_ambientPressureSensorConnected;     // Ambient pressure sensor connection state
extern int g_safeModeReason;                      // Reason safe mode is triggered.
extern bool g_modeLocked;                         // Operating-mode lock (roadmap item 11, LK setting)
extern bool g_calibrationRunning;                  // Full calibration accepted and in flight
extern bool g_centerRunning;                       // Center sample/timer flow in flight
extern String g_deviceUID;                        // Unique Device ID

class LSScreen {

public:
  LSScreen();
  void begin();
  void update();
  void clear();
  void show();

  void setupDisplay();
  void splashScreen();
  void splashScreen2();

  void nextMenuItem();
  void selectMenuItem();
  bool isMenuActive();
  void activateMenu();
  void deactivateMenu();
  // The user-facing menu CLOSE (serial CH,1:4 and the STOP_MENU input
  // action), frozen-window-aware like activateMenu/nextMenuItem/
  // selectMenuItem: a close arriving while a scheduled page transition or a
  // boot dwell page owns the display is dropped — executing it early would
  // flip _isActive off before the pending continuation registers its menu,
  // leaving a menu that is drawn and navigable but reported inactive (and
  // outside the 5-minute timeout). deactivateMenu itself stays unguarded:
  // the screen's own continuations and the timeout must always be able to
  // run their state machine.
  void closeMenu();

  // Live menu-state read for the serial menu-sync endpoint (CH,0:0, bench
  // follow-up 2026-10-18): fills [active, currentPageId, currentSelection].
  // The remote's drift problem was that a refused select (LK locked → the
  // ~2 s locked page) or a press dropped in a scheduled transition's frozen
  // window acked SUCCESS while the real menu went somewhere the client-side
  // model did not predict — device truth is the only cure. Selection is the
  // SELECTABLE-row index (the same index selectMenuItem() switches on); add
  // the page's _cursorStart offset for the text-array row.
  void fillMenuState(int out[3]);

  // Two-way Hub remote (firmware side of the CH,5 push, LipSync_Connect's
  // protocol proposal now realized): report the live menu state on the wire
  // whenever it changes, so the app's remote mirror follows PHYSICAL
  // navigation too — the one menu fact that never rode the wire unprompted
  // (settings changes already push MANUAL lines; app-driven presses are
  // followed by the app's own CH,0:0 poll). See reportMenuState for the wire
  // contract; the change-detect is the rate limiter.
  void reportMenuState();

  // Non-blocking page transitions (see scheduleTransition): the continuations
  // run on the screen timer when the informational page's dwell time ends,
  // instead of delay() freezing the whole main loop for 0.2-3 s.
  void scheduleTransition(unsigned long delayMs, void (*transitionFn)());
  void finishExitMenu();
  void beginLongTextScroll();
  void returnToMenu();
  // A serial-API-triggered calibration (#36) must not end by parking the Hub
  // on a main menu nothing opened: when set, the calibration end transition
  // (and the center-reset complete page) closes the menu instead of
  // returning to it. Set by setJoystickCalibration on the API path; the
  // menu-driven path clears it.
  void calibrationEndsByClosingMenu(bool enable);
  void finishChangeMode();
  void fullCalibrationConfirmMenu();
  void factoryResetConfirm2Menu();
  void modeLockedPage();
  void finishModeLocked();

  // The USB-retry loop's error page: drawn at most once per error episode, and
  // only when there is a display on an answerable I2C bus. The old code redrew
  // the full framebuffer on every retry tick (~2 s apart) without either check
  // — against held I2C hardware each draw cost seconds of bounded transaction
  // timeouts, so the retry loop alone dragged every serial exchange to ~2 s.
  void drawUsbErrorPageOnce();
  bool resetUsbErrorPage();

  // Display-health gate state for the bus-health diagnostic endpoint (BH):
  // false means the last framebuffer push wedged or crawled and the display
  // is being re-probed on an exponential backoff instead of trusted.
  bool isDisplayHealthy();

  void centerResetPage();
  void centerResetCompletePage();
  void centerResetFailedPage();
  void fullCalibrationPrompt(int stepNum);
  void calibrationAbortedPage();
  void infoMenu();
  void testPage(unsigned int usbConnectDelay);
  void noUsbPage();
  void updateModePage();
  void warningUSBDebugOn();
  void connectionTimingPage(unsigned long, unsigned long);
  void restartPage();
  void safeModePage(int);
  void safeModeMenu();
  void print4LineString(String s1, String s2, String s3, String s4);
  void disableTimeout();
  void enableTimeout();

  bool showCenterResetComplete = false;

private:
  Adafruit_SSD1306 _display = Adafruit_SSD1306(CONF_SCREEN_WIDTH, CONF_SCREEN_HEIGHT, &Wire, OLED_RESET);

  LSTimer<void> _screenStateTimer;  // Timer with a callback function
  int _screenStateTimerId = -1;     // Timer ID for screen (-1 = none pending; read by the USB-error-page splash-clear cancel)
  int _screenTimeoutTimerId;        // Timer ID to trigger next screen based on time
  unsigned long _inputFrozenUntil = 0;  // While millis() is before this, menu input is frozen (a scheduled page transition owns the menu)
  int _transitionTimerId = -1;           // Pending scheduled page transition (LSTimer id), -1 = none
  bool _usbErrorPageShown = false;       // The no-USB page has been drawn for the current error episode
  bool _usbErrorPagePainted = false;     // The no-USB page actually reached the OLED this episode (the latch
                                         // can be set on a headless unit where nothing was painted)
  bool _calibrationClosesMenu = false;   // The running calibration was started over the serial API — its end pages close the menu (#36)
  bool _displayHealthy = true;               // Display answers on the I2C bus (mirrors the pressure-sensor health gate)
  bool _begun = false;                       // SSD1306 framebuffer allocated; no draw call is legal before this
  unsigned long _lastDisplayProbeMs = 0;     // Last cheap re-probe while the display was marked unhealthy
  unsigned long _displayProbeIntervalMs = 2000;  // Re-probe cadence; doubles after each wedged push (flaky buses
                                                 // ACK a probe during momentary releases, and every re-trusted
                                                 // full push that then wedges costs seconds of bounded timeouts)
  void flushDisplay();                   // The single choke point for framebuffer pushes to the I2C bus
  void finishSplashClear();              // The splash/ready page's scheduled self-clear (see its definition)

  bool _isActive = false;
  bool _menuTimeoutEnabled = true;
  int _currentMenu = 0;
  int _prevMenu = -1; // keep track of previous menu
  int _prevSelection = -1; // Keep track of previous menu selection
  int _currentSelection = 0;
  int _selectedLine;
  int _cursorPos;

  // Two-way Hub remote (CH,5 push): the last [active, page, row] triple
  // reported on the wire, for reportMenuState's change-detect. Boot state
  // {0,0,0} means the closed-menu triple never prints by itself — the first
  // line is the first real menu event.
  int _lastReportedMenuState[3] = {0, 0, 0};

  int _operatingMode;
  int _tempOperatingMode;
  int _communicationMode;
  int _tempCommunicationMode;
  int _cursorSpeedLevel;
  int _scrollSpeedLevel;
  int _accelLevel;
  int _lightBrightLevel;
  bool _soundOn = true;
  int _soundMode;

  bool _scrollOn = false;
  unsigned long _scrollDelayTimer = millis();
  int _scrollPos = 12;
  const unsigned int _maxCharPerLine = 10;

  int _cursorStart = 0;
  int _countMenuScroll = 0;

  int _testScreenAttempt = 0;

  int _currentMenuLength;
  String *_currentMenuText = nullptr;  // null until a menu page registers (the boot dwell pages set _isActive first); read by the input/CH,5 guards
  String _selectedText;

  float _sipPressThresh;
  float _puffPressThresh;
  int _timingWindowSel = 0;  // Timing adjust page target: 0 = short (WS), 1 = long (WL), 2 = max (WM)

  unsigned long _lastActivityMillis;

  uint8_t _hardwareErrorCode = 0; 

  bool inputIsFrozen();  // (4.5.7 review) wrap/stale-safe menu-input freeze check
  void displayMenu();
  void displayCursor();
  void scrollLongText();
  void drawCentreString(const String &buf, int y);
  void modeMenuHighlight();

  void mainMenu();
  void exitConfirmMenu();
  void calibMenu();
  void modeMenu();
  void confirmModeChange();
  void changeMode();
  void cursorSpeedMenu();
  void scrollSpeedMenu();
  void bluetoothMenu();
  void moreMenu();
  void fullCalibrationPage();
  void fullCalibrationConfirmPage();
  void centerReset();
  void soundMenu();
  void lightBrightMenu();
  void sipPuffThreshMenu();
  void adjustSipThreshMenu();
  void adjustPuffThreshMenu();
  void accelMenu();
  void restrictionsMenu();
  void timingMenu();
  void adjustTimingMenu();
  void restartConfirmPage();
  void factoryResetConfirm1Page();
  void factoryResetConfirm2Page();
  void factoryResetPage();

  String _safeModeReasonText = "<Reason>";

  String _mainMenuText[5] = { "Exit Menu", "Center Reset", "Mode", "Cursor Speed", "More" };
  String _exitConfirmText[4] = { "Exit", "settings?", "Confirm", "... Back" };
  String _calibMenuText[4] = { "Center Reset", "... Back", " ", " " };
  String _modeMenuText[4] = { "MOUSE USB", "MOUSE BT", "GAMEPAD ", "... Back" };
  String _modeConfirmText[4] = { "Change", "mode?", "Confirm", "... Back" };
  String _cursorSpMenuText[4] = { "Speed: ", "Increase", "Decrease", "... Back" };
  String _moreMenuText[11] = { "Sound",  "Light Brightness", "Scroll Speed",   "Sip & Puff",  "Full Calibration",   "Restart LipSync",  "Factory Reset",  "Info", "Acceleration", "Restrictions", "... Back",  };
  String _soundMenuText[4] = { "Sound:", "<>", "Turn <>", "... Back" };
  String _lightBrightMenuText[4] = { "Lights: ", "Increase", "Decrease", "... Back" };
  String _scrollSpMenuText[4] = { "Speed: ", "Increase", "Decrease", "... Back" };
  String _accelMenuText[4] = { "Accel: ", "Increase", "Decrease", "... Back" };
  String _sipPuffThreshMenuText[4] = { "Sip Threshold", "Puff Threshold", "Timing", "... Back" };
  String _adjustSipThreshMenuText[4] = { "Sip: ", "Increase", "Decrease", "... Back" };
  String _adjustPuffThreshMenuText[4] = { "Puff: ", "Increase", "Decrease", "... Back" };
  // Input timing (roadmap item 15, upstream #62): the three window
  // boundaries, each adjusted on its own page in 250 ms steps.
  String _timingMenuText[4] = { "Short: ", "Long: ", "Max: ", "... Back" };
  String _timingAdjustMenuText[4] = { "Short: ", "Increase", "Decrease", "... Back" };
  // Input restrictions (roadmap item 11): one line per toggle, state shown
  // inline; selecting a line flips that restriction.
  String _restrictionsMenuText[5] = { "Sip&Puff: On", "Menu: All", "Mode lock: Off", "Cursor hold: Off", "... Back" };
  String _restartConfirmText[4] = { "Restart", "LipSync?", "Confirm", "... Back" };
  String _factoryResetConfirm1Text[4] = { "Reset to", "defaults?", "Confirm", "... Back" };
  String _factoryResetConfirm2Text[4] = { "Are you", "sure?", "Confirm", "... Back" };
  String _fullCalibrationConfirmText[4] = { "Are you", "sure?", "Confirm", "... Back" };
  String _infoMenuText[4] = {"Info", "Version","ID","... Back"};
  String _safeModeMenuText[4] = {"SAFE MODE", "Options:", "Restart", "Factory Reset"};

  // Number of selectable options in each menu
  const int _mainMenuLen = 5;
  const int _exitConfirmLen = 2;
  const int _calibMenuLen = 2;
  const int _modeMenuLen = 4;
  const int _cursorSpMenuLen = 3;
  const int _moreMenuLen = 11;
  const int _soundMenuLen = 2;
  const int _lightBrightMenuLen = 3;
  const int _scrollSpMenuLen = 3;
  const int _accelMenuLen = 3;
  const int _sipPuffThreshMenuLen = 4;
  const int _adjustSipThreshMenuLen = 3;
  const int _adjustPuffThreshMenuLen = 3;
  const int _restrictionsMenuLen = 5;
  const int _timingMenuLen = 4;
  const int _timingAdjustMenuLen = 3;
  const int _restartConfirmLen = 2;
  const int _factoryResetConfirm1Len = 2;
  const int _factoryResetConfirm2Len = 2;
  const int _fullCalibrationConfirmLen = 2;
  const int _safeModeMenuLen = 2;
  const int _infoMenuLen = 1;
};


//*********************************//
// Function   : LSScreen
//
// Description: Class for display object
//
// Arguments :  void
//
// Return     : void
//*********************************//
LSScreen::LSScreen() {
}

//*********************************//
// Function   : begin
//
// Description: Activates and clears display
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::begin() {

  if (_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    _begun = true;
    g_displayConnected = true;
  } else {
    Serial.println(F("ERROR: Display SSD1306 allocation failed"));

    g_displayConnected = false;
    _displayHealthy = false;
    _begun = false;
    return;  // Adafruit_SSD1306 draw calls dereference a null framebuffer after allocation failure
  }

  setupDisplay();  // Clear screen
  _display.setTextWrap(false);
  flushDisplay();

  _operatingMode = getOperatingMode(false, false);  // TODO JDMc 2025-Jan-24 These should be moved to update function so they are updated if changed through serial API
  _communicationMode = getCommunicationMode(false, false);
  _soundMode = getSoundMode(false, false);

  _lastActivityMillis = millis();
}

//*********************************//
// Function   : clear
//
// Description: Clears the display.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::clear() {
  if (!_begun) return;
  //_display.clearDisplay();
  setupDisplay();
}


//*********************************//
// Function   : update
//
// Description: Refreshes the display and scrolls text if necessary
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::update() {
  _screenStateTimer.run();

  // Loop for screen functions
  if (_scrollOn) {
    scrollLongText();
  }

  if (((millis() - _lastActivityMillis) > CONF_MENU_TIMEOUT) && _isActive && _menuTimeoutEnabled) {
    deactivateMenu();
  }
}

//*********************************//
// Function   : inputIsFrozen
//
// Description: True while a scheduled page transition or boot dwell owns the
//              menu and input must be dropped. Wrap-safe AND stale-safe
//              (4.5.7 review): a genuine freeze is always a short window in
//              the FUTURE, so the remaining time is a small positive. The
//              bare `(long)(millis() - _inputFrozenUntil) < 0` idiom flipped
//              to "frozen" for the entire millis() window [T+2^31, T+2^32)
//              — ~24.9 days — whenever _inputFrozenUntil was 0 (never set on
//              a normal boot) or left far in the past, silently jamming the
//              Hub menu for a 25-day stretch of a long-uptime device. Bound
//              the honored window to well above the longest real freeze
//              (the ~3 s dwell/transition/center windows) so a stale
//              timestamp can never read as frozen.
//
// Arguments :  void
//
// Return     : bool : true = input frozen
//*********************************//
bool LSScreen::inputIsFrozen() {
  long remaining = (long)(_inputFrozenUntil - millis());
  return remaining > 0 && remaining <= (long)CONF_SCREEN_INPUT_FREEZE_MAX_MS;
}

//*********************************//
// Function   : activateMenu
//
// Description: Changes display mode to active and loads main menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::activateMenu() {
  if (!_begun || !g_displayConnected) return;
  if (inputIsFrozen()) return;  // A scheduled page transition owns the menu right now (wrap/stale-safe — see inputIsFrozen)
  if (!Wire.busHealthy()) return;  // Held I2C bus — the menu could not draw anyway; don't feed the timeout tax

  _lastActivityMillis = millis();
  _isActive = true;
  _countMenuScroll = 0;
  _operatingMode = getOperatingMode(false, false);
  _communicationMode = getCommunicationMode(false, false);  // 4.5.7 review: a live CM,1 switch (no reset) left the Mode page highlighting the stale com mode until reboot

  mainMenu();
}

//*********************************//
// Function   : deactivateMenu
//
// Description: Changes display mode to inactive and clears display.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::deactivateMenu() {
  _isActive = false;
  clear();
  flushDisplay();
}

//*********************************//
// Function   : closeMenu
//
// Description: The user-facing menu close (serial CH,1:4, the STOP_MENU
//              input action): drops the close while a scheduled page
//              transition or a boot dwell page owns the display, so a
//              pending continuation can never register its menu onto an
//              _isActive the close already turned off (a menu that draws
//              and navigates but reports inactive, outside the timeout —
//              the same frozen-window contract activateMenu /
//              nextMenuItem / selectMenuItem already enforce).
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::closeMenu() {
  if (inputIsFrozen()) return;  // A scheduled page transition / boot dwell page owns the display right now (wrap/stale-safe — see inputIsFrozen)
  deactivateMenu();
}

//*********************************//
// Function   : scheduleTransition
//
// Description: Shows an informational page, then runs `transitionFn` after
//              `delayMs` on the screen timer. Replaces the delay() calls that
//              froze the entire main loop (serial API, joystick, pressure)
//              for the dwell time of each page. Menu input is frozen for the
//              same window so button presses are dropped exactly as they were
//              under the old blocking wait — the rest of the device keeps
//              running. Only one transition may be pending; a new one
//              replaces it (and unfreezes input for its own window).
//
// Arguments :  delayMs : unsigned long : page dwell time in milliseconds
//              transitionFn : void (*)() : free-function continuation (defined
//                            in LipSync_Firmware.ino)
//
// Return     : void
//*********************************//
void LSScreen::scheduleTransition(unsigned long delayMs, void (*transitionFn)()) {
  _inputFrozenUntil = millis() + delayMs;
  if (_transitionTimerId >= 0) {
    _screenStateTimer.deleteTimer(_transitionTimerId);
  }
  _transitionTimerId = _screenStateTimer.setTimeout(delayMs, transitionFn);
  if (_transitionTimerId < 0) {
    _inputFrozenUntil = 0;
    if (transitionFn != nullptr) transitionFn();
  }
}

// Each continuation first clears the pending-transition id (the timer just
// fired), so a later scheduleTransition cannot delete a recycled timer slot.

void LSScreen::finishExitMenu() {
  _transitionTimerId = -1;
  deactivateMenu();
}

void LSScreen::beginLongTextScroll() {
  _transitionTimerId = -1;
  scrollLongText();
}

void LSScreen::returnToMenu() {
  _transitionTimerId = -1;
  if (_calibrationClosesMenu) {
    // #36: this calibration run was started over the serial API — the menu
    // was not how the flow started, so it must not be how it ends. Close it,
    // then show the ready page: a merely blank screen reads as "dead" and
    // invites a long wake-puff — which is START_MENU (3-8 s) and puts the
    // menu right back (observed on hardware, 2026-10-14). The ready page
    // tells the user the device is finished and usable, then clears itself.
    _calibrationClosesMenu = false;
    _isActive = false;
    clear();
    flushDisplay();
    if (g_displayConnected && Wire.busHealthy()) {
      splashScreen2();
    }
    return;
  }
  if (_isActive) {
    mainMenu();
  } else {
    deactivateMenu();
  }
}

void LSScreen::calibrationEndsByClosingMenu(bool enable) {
  _calibrationClosesMenu = enable;
}

void LSScreen::finishChangeMode() {
  _transitionTimerId = -1;

  // Persist both halves before the single reset. Calling the public setters
  // sequentially is not atomic: CM resets immediately on change, so a
  // Bluetooth-mouse -> USB-gamepad menu change rebooted after CM and never
  // stored OM. Direct validated writes make the pair land together from the
  // menu's already constrained choices.
  bool stored = storeModePair(_tempOperatingMode, _tempCommunicationMode);
  if (!stored) {
    buzzerErrorCue();
    modeMenu();
    return;
  }

  _communicationMode = _tempCommunicationMode;
  _operatingMode = _tempOperatingMode;
  softwareReset();
}

void LSScreen::fullCalibrationConfirmMenu() {
  _transitionTimerId = -1;
  _currentMenu = FULL_CALIB_CONFIRM_PAGE;
  _currentMenuLength = _fullCalibrationConfirmLen;
  _currentMenuText = _fullCalibrationConfirmText;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

void LSScreen::factoryResetConfirm2Menu() {
  _transitionTimerId = -1;
  _currentMenu = FACTORY_RESET_CONFIRM2_PAGE;
  _currentMenuLength = _factoryResetConfirm2Len;
  _currentMenuText = _factoryResetConfirm2Text;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;  // 4.5.7 review: stale parent-menu scroll offset indexed past this 4-element array

  displayMenu();  //  Print items in current menu
}

void LSScreen::drawUsbErrorPageOnce() {
  if (_usbErrorPageShown) return;
  if (!g_displayConnected) {  // Nothing to draw on
    _usbErrorPageShown = true;
    return;
  }
  if (!Wire.busHealthy()) return;  // Bus is held — a draw would only feed the timeout tax; retried on the next connection check
  if (!_isActive) {
    // Cancel the ready page's pending 10 s self-clear: it was armed by the
    // splash this episode interrupted, and if it fires while the No-USB page
    // is on the OLED it wipes it — and the once-per-episode latch this
    // function sets would then keep the page from ever redrawing (a blank
    // screen for the rest of the outage, bench-audit finding). The error
    // page owns the display until the episode clears. Safe against
    // recycled slots because every continuation that fires from a
    // _screenStateTimerId registration resets it (see finishSplashClear and
    // the boot-page chain) — the id only ever names a genuinely pending
    // timer, never a fired-and-reused slot.
    if (_screenStateTimerId >= 0) {
      _screenStateTimer.deleteTimer(_screenStateTimerId);
      _screenStateTimerId = -1;
    }
    noUsbPage();
    _usbErrorPagePainted = true;
  }
  _usbErrorPageShown = true;
}

bool LSScreen::resetUsbErrorPage() {
  // #63 residual, round 2 (bench 2026-10-16, reproduced twice): the clear is
  // ONE-SHOT — usbCheckConnection's healthy branch never re-arms — and it
  // cleared the episode latch unconditionally while gating the actual draw
  // behind bus/display health. One healthy tick landing while the bus was in
  // its 1 s post-recovery window (or the display in a re-probe backoff)
  // consumed the only clear: latch gone, timer dead, and the No-USB page
  // stayed on the OLED for the rest of the boot on a fully working device —
  // the user had to restart by hand. The clear now succeeds only when the
  // ready page can actually land; the caller re-arms while it cannot.
  if (!_usbErrorPageShown) return true;
  if (!_usbErrorPagePainted) {
    // The latch was set but nothing ever reached the OLED (headless unit, or
    // every draw was gated) — nothing on screen needs fixing.
    _usbErrorPageShown = false;
    return true;
  }
  if (_isActive || !g_displayConnected || !Wire.busHealthy() || !_displayHealthy) return false;
  _usbErrorPageShown = false;
  _usbErrorPagePainted = false;
  splashScreen2();
  return true;
}

//*********************************//
// Function   : finishSplashClear
//
// Description: The splash/ready page's scheduled self-clear, as a member so
//              it can keep _screenStateTimerId honest (the 4.5.5 follow-up
//              audit): every continuation that fires from a
//              _screenStateTimerId slot resets the id, exactly the way the
//              scheduleTransition continuations reset _transitionTimerId
//              (see the invariant documented there) — a fired RUN_ONCE
//              timeout frees its slot, and the pool hands that slot to the
//              NEXT registration, so a stale id read later could delete a
//              live, unrelated timer. The only reader is the USB-error
//              page's splash-clear cancel, which must stay able to kill a
//              genuinely pending splash clear without any chance of hitting
//              a recycled slot.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::finishSplashClear() {
  _screenStateTimerId = -1;  // The timer just fired — its slot is free again
  if (!_isActive) {
    // Same body the clearSplashScreen free function had: blank the display
    // once the splash's dwell ends and no menu took it over.
    deactivateMenu();
  }
}

bool LSScreen::isDisplayHealthy() {
  return _begun && _displayHealthy;
}

//*********************************//
// Function   : flushDisplay
//
// Description: The single choke point for pushing the framebuffer to the
//              display. Text and clear calls only touch the RAM buffer;
//              display() is what drives the I2C bus — and against a held
//              bus (a failing display or sensor) each push costs seconds of
//              bounded transaction timeouts. Skipping the push while the
//              bus is wedged keeps boot sequences, the safe-mode menu and
//              every page transition cheap; the page state machines still
//              advance, and the next flush after the bus recovers catches
//              the drawing up. Without this, chained boot-time draws could
//              outrun the 30 s watchdog and reset the device in a loop.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::flushDisplay() {
  if (!_begun || !g_displayConnected) return;
  // Display health gate (mirrors the pressure sensors'): a full framebuffer
  // push against a held or absent display costs seconds of bounded
  // transaction timeouts — and a flaky bus ACKs probes during momentary
  // releases, so every re-trusted push can wedge all over again. One wedged
  // push demotes the display; re-probes are a single cheap address ACK on an
  // exponentially backing-off cadence (2 s doubling to 60 s, reset by a clean
  // push). This keeps boot sequences, readyToUse() pages and the safe-mode
  // menu from stalling the main loop — or outrunning the 30 s watchdog at
  // boot — when the display side of the shared I2C bus is dead or flaky.
  if (!_displayHealthy) {
    if (millis() - _lastDisplayProbeMs < _displayProbeIntervalMs) return;
    _lastDisplayProbeMs = millis();
    Wire.beginTransmission(SCREEN_ADDRESS);
    if (Wire.endTransmission() != 0) return;  // Still not answering
    _displayHealthy = true;                   // ACKed — the next flush pushes for real
  }

  if (!Wire.busHealthy()) return;  // Bus held by something else — retry on a later flush

  uint32_t recoveriesBefore = Wire.recoveryCount();
  unsigned long pushStartMs = millis();
  _display.display();
  unsigned long pushMs = millis() - pushStartMs;
  if (Wire.recoveryCount() != recoveriesBefore || pushMs > 100) {
    // This push wedged the bus, or crawled through it: a flaky device can
    // drag every transfer until it completes slowly without ever tripping a
    // timeout — a healthy push takes ~5-10 ms, anything past 100 ms means
    // the shared bus is being dragged. One strike, and back off.
    _displayHealthy = false;
    _displayProbeIntervalMs = _displayProbeIntervalMs * 2;
    if (_displayProbeIntervalMs > 60000) _displayProbeIntervalMs = 60000;
  } else {
    _displayProbeIntervalMs = 2000;  // Clean push — restore the base re-probe cadence
  }

  // Two-way Hub remote (see reportMenuState): every menu redraw ends here,
  // so this single hook turns a menu state change into a wire push — open,
  // navigation, page transitions, the exit/timeout close all flow through
  // it. It runs only when the framebuffer actually reached the bus: while
  // the display is demoted the menu's internal state advances but nothing
  // is shown, and the app's CH,0:0 poll still reads the same truth.
  reportMenuState();
}

//*********************************//
// Function   : isMenuActive
//
// Description: Returns a boolean representing state of menu
//
// Arguments :  void
//
// Return     : bool : _isActive : true = menu activated
//*********************************//
bool LSScreen::isMenuActive() {

  return _isActive;
}

//*********************************//
// Function   : splashScreen
//
// Description: Displays a screen with the device name and version. To be used when the device starts up.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::splashScreen() {
  if (!_begun) return;
  setupDisplay();

  _display.setTextSize(2);
  drawCentreString("LipSync", 12);

  _display.setTextSize(1);
  drawCentreString(CONF_LIPSYNC_VERSION_STRING, 32);
  drawCentreString("Makers Making Change", 54);

  flushDisplay();
}

//*********************************//
// Function   : splashScreen2
//
// Description: Displays a screen indicating it is ready to use (center reset complete) and indicating the mode.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::splashScreen2() {
  if (!_begun) return;
  setupDisplay();

  _display.setTextSize(2);
  drawCentreString("Ready to", 0);
  drawCentreString("use", 16);
  _display.setTextSize(1);
  drawCentreString("Mode:", 40);
  _display.setTextSize(2);


  _display.setTextSize(2);

  switch (_operatingMode) {
    case CONF_OPERATING_MODE_MOUSE:
      switch (_communicationMode) {
        case CONF_COM_MODE_USB:
          drawCentreString("USB Mouse", 48);
          break;
        case CONF_COM_MODE_BLE:
          drawCentreString("BT Mouse", 48);
          break;
      }
      break;
    case CONF_OPERATING_MODE_GAMEPAD:
      _display.setCursor(1, 48);
      _display.print("USB");
      _display.setTextSize(1);
      _display.print(" ");
      _display.setTextSize(2);
      _display.print("Gamepad");  // text size changed for space so it would all fit on one line
      //_display.print("USB"); _display.setTextSize(1); _display.print(" "); _display.setTextSize(2); _display.print("Gamepad"); // text size changed for space so it would all fit on one line
      //drawCentreString("Gamepad", 48);
      break;
    case CONF_OPERATING_MODE_SAFE:
      // Currently bypassed since safe mode menus are called directly
      break;
    default:
      _display.println("Error");
  }

  flushDisplay();

  if (USB_DEBUG){
    //delay(2000);  //TODO - 2025-FEB-21 Why is this delay here?
    warningUSBDebugOn();
  } else {
    // Delete-before-rearm (4.5.7 review): every writer of _screenStateTimerId
    // first deletes any genuinely pending predecessor — the id only ever
    // names a live timer (the continuations reset it when they fire), so a
    // second arm could never leak an orphaned splash-clear/safe-mode timer
    // whose late firing would blank whatever page owns the display then.
    // The same guard applies at warningUSBDebugOn/restartPage/
    // factoryResetPage/safeModePage.
    if (_screenStateTimerId >= 0) {
      _screenStateTimer.deleteTimer(_screenStateTimerId);
    }
    _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::finishSplashClear>(CONF_SPLASH_SCREEN_DURATION, this);
  }

  _lastActivityMillis = millis();
}

//****************************************//
// Functions called from inputs
//****************************************//

//*********************************//
// Function   : nextMenuItem
//
// Description: Cycles to the next item based on current selection and menu.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::nextMenuItem() {
  if (!_begun) return;
  if (inputIsFrozen()) return;  // A scheduled page transition owns the menu right now (wrap/stale-safe — see inputIsFrozen)
  if (!_isActive) return;  // No open menu (4.5.7 review): the serial CH,1 vector used to keep driving the STALE page after a close — deactivateMenu never nulls _currentMenuText, so a NEXT painted a cursor over the ready splash and a SELECT executed the stale page's switch (a stale RESTART_PAGE row reset the device; a stale MORE walk reached the factory reset) while CH,0:0 truthfully answered "no menu". Physical inputs were already gated on isMenuActive(); the serial vector now gets the same gate.
  if (_currentMenuText == nullptr) return;  // No menu page is registered yet (the safe-mode/error boot pages set _isActive before safeModeMenu registers state)

  _lastActivityMillis = millis();

  if (_scrollOn) {
    _display.setCursor(0, _selectedLine * 16);
    _display.print("                                   ");
    _display.setCursor(12, _selectedLine * 16);
    _display.print(_selectedText);
  }

  _currentSelection++;  // Increment current selection by one 
  
  if (_currentSelection >= _currentMenuLength) { // if 
    _currentSelection = 0;
    _countMenuScroll = 0;
    displayMenu();  //  Print items in current menu and display cursor
  } else if (_currentSelection + _cursorStart > TEXT_ROWS - 1) {
    _countMenuScroll++;
    displayMenu();  //  Print items in current menu and display cursor
  } else {
    displayCursor();
  }

}



//*********************************//
// Function   : selectMenuItem
//
// Description: Activates the appropriate action when the current selection is selected.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::selectMenuItem() {
  if (!_begun) return;
  if (inputIsFrozen()) return;  // A scheduled page transition owns the menu right now (wrap/stale-safe — see inputIsFrozen)
  if (!_isActive) return;  // No open menu (4.5.7 review) — see nextMenuItem(): a closed menu's stale page must not execute from the serial CH,1 vector
  if (_currentMenuText == nullptr) return;  // No menu page is registered yet (the safe-mode/error boot pages set _isActive before safeModeMenu registers state)

  _lastActivityMillis = millis();

  // Destructive safe-menu and confirmation selections require a deliberate
  // hold. Short electrical glitches previously walked and confirmed these
  // rows; serial CH presses carry UINT32_MAX and intentionally bypass because
  // the wire has no hold-duration concept.
  bool destructiveSelection =
      (_currentMenu == SAFEMODE_MENU && (_currentSelection == 0 || _currentSelection == 1)) ||
      (_currentMenu == RESTART_PAGE && _currentSelection == 0) ||
      (_currentMenu == FACTORY_RESET_PAGE && _currentSelection == 0) ||
      (_currentMenu == FACTORY_RESET_CONFIRM2_PAGE && _currentSelection == 0);
  unsigned long confirmHoldMs = effectiveWindowMs(CONF_INPUT_WINDOW_SHORT_DEFAULT_MS) / 2;
  if (confirmHoldMs < CONF_MENU_CONFIRM_HOLD_MIN_MS) confirmHoldMs = CONF_MENU_CONFIRM_HOLD_MIN_MS;
  if (destructiveSelection && g_menuInputHoldMs != 0xFFFFFFFFUL && g_menuInputHoldMs < confirmHoldMs) {
    buzzerErrorCue();
    return;
  }

  _countMenuScroll = 0;
  switch (_currentMenu) {
    case MAIN_MENU:
      _prevMenu = MAIN_MENU;
      _currentMenu = _currentSelection + 1;
      switch (_currentMenu) {
        case MAIN_MENU:
          mainMenu();
          break;
        case EXIT_MENU:
          exitConfirmMenu();
          break;
        case CALIB_MENU:
          calibMenu();
          break;
        case MODE_MENU:
          modeMenu();
          break;
        case CURSOR_SP_MENU:
          cursorSpeedMenu();
          break;
        case MORE_MENU:
          moreMenu();
          break;
      }
      break;
    case EXIT_MENU:
      if (_currentSelection == 1) {
        _currentSelection = 0;
        mainMenu();
      } else {
        _currentMenu = EXITING_PAGE;  // Name the dwell (4.5.7 review): a CH,0:0 poll during the 0.5 s window sees the page actually shown, not the confirm page it interrupted
        setupDisplay();
        _display.println("Exiting");
        flushDisplay();
        scheduleTransition(500, screenExitTransitionDone);  // was delay(500) — the menu closes on the next ticks
      }
      break;
    case CALIB_MENU:
      if (_currentSelection == 0) {
        _currentMenu = CENTER_RESET_PAGE;
        centerResetPage();
      } else if (_currentSelection == 1) {
        _currentMenu = MAIN_MENU;
        mainMenu();
      }
      break;
    case MODE_MENU:
      // Operating-mode lock (roadmap item 11, upstream #68): the three mode
      // rows refuse with the short error cue — no confirm page — while the
      // serial API (and so LipSync Connect) can still change the mode
      // deliberately. "... Back" stays live: the first pass gated the whole
      // page and trapped the user inside it (bench 2026-10-16) — a lockout
      // must never strand the person it protects.
      if (_currentSelection < (_modeMenuLen - 1) && g_modeLocked) {
        // Say why, not just beep (bench feedback 2026-10-16): a bare error
        // cue is ambiguous — dead button, generic error, or locked? The
        // locked page names the reason and the unlock path, then returns to
        // the Mode page.
        buzzerErrorCue();
        modeLockedPage();
        break;
      }
      if (_currentSelection < (_modeMenuLen - 1)) {
        // Confirm mode change
        switch (_currentSelection + 1) {
          case _MODE_MOUSE_USB:
            _tempOperatingMode = CONF_OPERATING_MODE_MOUSE;
            _tempCommunicationMode = CONF_COM_MODE_USB;
            break;
          case _MODE_MOUSE_BT:
            _tempOperatingMode = CONF_OPERATING_MODE_MOUSE;
            _tempCommunicationMode = CONF_COM_MODE_BLE;
            break;
          case _MODE_GAMEPAD_USB:
            _tempOperatingMode = CONF_OPERATING_MODE_GAMEPAD;
            _tempCommunicationMode = CONF_COM_MODE_USB;
            break;
        }
        //_tempOperatingMode = _currentSelection;
        if ((_tempOperatingMode != _operatingMode) || (_tempCommunicationMode != _communicationMode)) {
          confirmModeChange();
        }
      } else if (_currentSelection == (_modeMenuLen - 1)) {
        mainMenu();
      }
      break;
    case CONFIRM_MODE_CHANGE:
      if (_currentSelection == 1) {
        _currentSelection = 0;
        modeMenu();
      } else {
        changeMode();
      }
      break;
    case CURSOR_SP_MENU:
      switch (_currentSelection) {
        case 0:  // Increase
          increaseCursorSpeed(true, false);
          _cursorSpeedLevel = getCursorSpeed(true, false);
          _cursorSpMenuText[0] = "Speed: " + String(_cursorSpeedLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_cursorSpMenuText[0]);
          flushDisplay();
          break;
        case 1:  // Decrease
          decreaseCursorSpeed(true, false);
          _cursorSpeedLevel = getCursorSpeed(true, false);
          _cursorSpMenuText[0] = "Speed: " + String(_cursorSpeedLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_cursorSpMenuText[0]);
          flushDisplay();
          break;
        case 2:  // Back
          _currentMenu = MAIN_MENU;
          mainMenu();
          break;
      }
      break;
    case MORE_MENU:
      _prevMenu = MORE_MENU;

      switch(_currentSelection) {
        case 0: // Sound
          soundMenu();
          break;
        case 1:
          lightBrightMenu();
          break;
        case 2:
          scrollSpeedMenu();
          break;
        case 3: // Sip puff
          sipPuffThreshMenu();
          break;
        case 4: // Full Calibration
          fullCalibrationConfirmPage();
          break;
        case 5: // Restart
          restartConfirmPage();
          break;
        case 6: // Factory Reset
          factoryResetConfirm1Page();
          break;
        case 7: // Info
          infoMenu();
          break;
        case 8: // Acceleration (roadmap item 9, upstream #86)
          accelMenu();
          break;
        case 9: // Restrictions (roadmap item 11)
          restrictionsMenu();
          break;
        case 10: // Back
          mainMenu();
          break;
      }
      break;

    case SOUND_MENU:
      switch (_currentSelection) {
        case 0:  // Toggle sound mode
          if (_soundMode == CONF_SOUND_MODE_OFF) {
            buzzerSoundOn();
            _soundMode = CONF_SOUND_MODE_BASIC;
            setSoundMode(false, false, _soundMode);  // TODO: change menu to add Advanced Sound to menu
          } else {
            buzzerSoundOff();
            _soundMode = CONF_SOUND_MODE_OFF;
            setSoundMode(false, false, _soundMode);
          }
          soundMenu();
          break;
        case 1:  // Back
          _currentMenu = MORE_MENU;
          moreMenu();
      }
      break;

    case LIGHT_BRIGHT_MENU:
      switch (_currentSelection) {
        case 0:  // Increase
          _lightBrightLevel = getLightBrightnessLevel(false, false);
          _lightBrightLevel++;
          setLightBrightnessLevel(false, false, _lightBrightLevel);                         
          _lightBrightLevel = getLightBrightnessLevel(false, false);
          _lightBrightMenuText[0] = "Lights: " + String(_lightBrightLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_lightBrightMenuText[0]);
          flushDisplay();
          break;
        case 1:  // Decrease
          _lightBrightLevel = getLightBrightnessLevel(false, false);
          _lightBrightLevel--;
          setLightBrightnessLevel(false, false, _lightBrightLevel);                    
          _lightBrightLevel = getLightBrightnessLevel(false, false);
          _lightBrightMenuText[0] = "Lights: " + String(_lightBrightLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_lightBrightMenuText[0]);
          flushDisplay();
          break;
        case 2:  // Back
          _currentMenu = MORE_MENU;
          moreMenu();
          break;
      }

      break;

    case SCROLL_SP_MENU:
      switch (_currentSelection) {
        case 0:  // Increase
          _scrollSpeedLevel = getScrollLevel(false, false);
          _scrollSpeedLevel++;
          setScrollLevel(true, false, _scrollSpeedLevel);
          _scrollSpeedLevel = getScrollLevel(false, false);
          _scrollSpMenuText[0] = "Speed: " + String(_scrollSpeedLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_scrollSpMenuText[0]);
          flushDisplay();
          break;
        case 1:  // Decrease
          _scrollSpeedLevel = getScrollLevel(false, false);
          _scrollSpeedLevel--;
          setScrollLevel(true, false, _scrollSpeedLevel);
          _scrollSpeedLevel = getScrollLevel(false, false);
          _scrollSpMenuText[0] = "Speed: " + String(_scrollSpeedLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_scrollSpMenuText[0]);
          flushDisplay();
          break;
        case 2:  // Back
          _currentMenu = MORE_MENU;
          moreMenu();
          break;
      }

      break;

    case SIP_PUFF_MENU:
      switch (_currentSelection) {
        case 0:  // Sip
          adjustSipThreshMenu();
          break;
        case 1:  // Puff
          adjustPuffThreshMenu();
          break;
        case 2:  // Timing (roadmap item 15, upstream #62)
          timingMenu();
          break;
        case 3:
          _currentMenu = MORE_MENU;
          moreMenu();
          break;
      }
      break;

    case ACCEL_MENU:
      // Cursor acceleration level (roadmap item 9): -10 (slowest/most
      // precise) .. 10 (fastest); 0 is the linear default. The API set
      // clamps out-of-range steps, and the re-read keeps the label truthful.
      switch (_currentSelection) {
        case 0: {  // Increase
          _accelLevel = getJoystickAcceleration(false, false);
          _accelLevel++;
          setJoystickAcceleration(true, false, _accelLevel);
          _accelLevel = getJoystickAcceleration(false, false);
          _accelMenuText[0] = "Accel: " + String(_accelLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_accelMenuText[0]);
          flushDisplay();
          break;
        }
        case 1: {  // Decrease
          _accelLevel = getJoystickAcceleration(false, false);
          _accelLevel--;
          setJoystickAcceleration(true, false, _accelLevel);
          _accelLevel = getJoystickAcceleration(false, false);
          _accelMenuText[0] = "Accel: " + String(_accelLevel) + " ";
          _display.setCursor(0, 0);
          _display.print(_accelMenuText[0]);
          flushDisplay();
          break;
        }
        case 2:  // Back
          _currentMenu = MORE_MENU;
          moreMenu();
          break;
      }
      break;

    case RESTRICTIONS_MENU:
      // Input restrictions (roadmap item 11): each line toggles one
      // restriction and re-renders its state label. The re-read keeps the
      // label truthful even when the setter clamps or the stored value
      // changed underneath us.
      switch (_currentSelection) {
        case 0: {  // Sip & puff enable
          int sapDisabled = getSapDisabled(false, false);
          setSapDisabled(true, false, sapDisabled == 1 ? 0 : 1);
          restrictionsMenu();
          break;
        }
        case 1: {  // Menu access: all inputs / hub buttons only
          int buttonsOnly = getMenuButtonsOnly(false, false);
          setMenuButtonsOnly(true, false, buttonsOnly == 1 ? 0 : 1);
          restrictionsMenu();
          break;
        }
        case 2: {  // Operating-mode lock
          int modeLocked = getModeLocked(false, false);
          setModeLocked(true, false, modeLocked == 1 ? 0 : 1);
          restrictionsMenu();
          break;
        }
        case 3: {  // Cursor hold during sip & puff
          int cursorHold = getCursorHoldDuringSap(false, false);
          setCursorHoldDuringSap(true, false, cursorHold == 1 ? 0 : 1);
          restrictionsMenu();
          break;
        }
        case 4:  // Back
          _currentMenu = MORE_MENU;
          moreMenu();
          break;
      }
      break;

    case SIP_THRESH_MENU:
      // The Hub menu steps through the coarse 10-level threshold scale
      // (roadmap item 15, upstream #22) instead of ±1 hPa — the fine hPa
      // granularity stays on the serial API and in LipSync Connect.
      switch (_currentSelection) {
        case 0: {  // Increase
          _sipPressThresh = getSipPressureThreshold(false, false);
          setSipPressureThreshold(false, false, pressThresholdLevelUp(_sipPressThresh));
          _sipPressThresh = getSipPressureThreshold(false, false);
          _adjustSipThreshMenuText[0] =
            "Sip: L" + String(pressThresholdLevelFor(_sipPressThresh)) + " " + String(_sipPressThresh) + " ";
          _display.setCursor(0, 0);
          _display.print(_adjustSipThreshMenuText[0]);
          flushDisplay();
          break;
        }
        case 1: {  // Decrease
          _sipPressThresh = getSipPressureThreshold(false, false);
          setSipPressureThreshold(false, false, pressThresholdLevelDown(_sipPressThresh));
          _sipPressThresh = getSipPressureThreshold(false, false);
          _adjustSipThreshMenuText[0] =
            "Sip: L" + String(pressThresholdLevelFor(_sipPressThresh)) + " " + String(_sipPressThresh) + " ";
          _display.setCursor(0, 0);
          _display.print(_adjustSipThreshMenuText[0]);
          flushDisplay();
          break;
        }
        case 2:  // Back
          _currentMenu = SIP_PUFF_MENU;
          sipPuffThreshMenu();
          break;
      }
      break;

    case PUFF_THRESH_MENU:
      // Level scale, same as the sip page (roadmap item 15, upstream #22).
      switch (_currentSelection) {
        case 0: {  // Increase
          _puffPressThresh = getPuffPressureThreshold(false, false);
          setPuffPressureThreshold(false, false, pressThresholdLevelUp(_puffPressThresh));
          _puffPressThresh = getPuffPressureThreshold(false, false);
          _adjustPuffThreshMenuText[0] =
            "Puff: L" + String(pressThresholdLevelFor(_puffPressThresh)) + " " + String(_puffPressThresh) + " ";
          _display.setCursor(0, 0);
          _display.print(_adjustPuffThreshMenuText[0]);
          flushDisplay();
          break;
        }
        case 1: {  // Decrease
          _puffPressThresh = getPuffPressureThreshold(false, false);
          setPuffPressureThreshold(false, false, pressThresholdLevelDown(_puffPressThresh));
          _puffPressThresh = getPuffPressureThreshold(false, false);
          _adjustPuffThreshMenuText[0] =
            "Puff: L" + String(pressThresholdLevelFor(_puffPressThresh)) + " " + String(_puffPressThresh) + " ";
          _display.setCursor(0, 0);
          _display.print(_adjustPuffThreshMenuText[0]);
          flushDisplay();
          break;
        }
        case 2:  // Back
          _currentMenu = SIP_PUFF_MENU;
          sipPuffThreshMenu();
          break;
      }
      break;

    case TIMING_MENU:
      // Input timing windows (roadmap item 15, upstream #62): one page per
      // boundary; selecting a line opens its adjust page.
      switch (_currentSelection) {
        case 0:  // Short window (WS)
        case 1:  // Long window (WL)
        case 2:  // Max window (WM)
          _timingWindowSel = _currentSelection;
          adjustTimingMenu();
          break;
        case 3:  // Back
          _currentMenu = SIP_PUFF_MENU;
          sipPuffThreshMenu();
          break;
      }
      break;

    case TIMING_ADJUST_MENU: {
      // 250 ms steps; the API setter clamps range and keeps short < long <
      // max ordering, and the re-read keeps the label truthful.
      if (_currentSelection == 2) {  // Back
        _currentMenu = TIMING_MENU;
        timingMenu();
        break;
      }
      const int stepMs = 250;
      int currentMs = 0;
      switch (_timingWindowSel) {
        case 0: {
          currentMs = getInputWindowShort(false, false);
          int nextMs = (_currentSelection == 0) ? currentMs + stepMs : currentMs - stepMs;
          setInputWindowShort(true, false, (unsigned long)nextMs);
          currentMs = getInputWindowShort(false, false);
          break;
        }
        case 1: {
          currentMs = getInputWindowLong(false, false);
          int nextMs = (_currentSelection == 0) ? currentMs + stepMs : currentMs - stepMs;
          setInputWindowLong(true, false, (unsigned long)nextMs);
          currentMs = getInputWindowLong(false, false);
          break;
        }
        default: {
          currentMs = getInputWindowMax(false, false);
          int nextMs = (_currentSelection == 0) ? currentMs + stepMs : currentMs - stepMs;
          setInputWindowMax(true, false, (unsigned long)nextMs);
          currentMs = getInputWindowMax(false, false);
          break;
        }
      }
      _timingAdjustMenuText[0] = String(_timingWindowSel == 0 ? "Short: " : _timingWindowSel == 1 ? "Long: " : "Max: ") + String(currentMs) + "ms ";
      _display.setCursor(0, 0);
      _display.print(_timingAdjustMenuText[0]);
      flushDisplay();
      break;
    }

    case FULL_CALIB_CONFIRM_PAGE:
      switch (_currentSelection) {
        case 0:  // Perform full calibration
          _currentMenu = FULL_CALIB_PAGE;
          fullCalibrationPage();
          break;
        case 1:  // Back
          _currentMenu = MORE_MENU;
          moreMenu();
          break;
      }
      break;

    case RESTART_PAGE:
      switch (_currentSelection) {
        case 0:  // Perform restart
          softwareReset(); // Call a software reset
          break;
        case 1:  // Back
          if (_prevMenu == MORE_MENU) {        
            moreMenu();
            break;
          } else if (_prevMenu == SAFEMODE_MENU) {
            safeModeMenu();
            break;
          } else {
            break;
          }
          
          //_currentMenu = MAIN_MENU;
          //mainMenu();
          
      }
      break;
    case FACTORY_RESET_PAGE:
      switch (_currentSelection) {
        case 0:  // Perform factory reset
          _currentMenu = FACTORY_RESET_CONFIRM2_PAGE;
          factoryResetConfirm2Page();
          break;
        case 1:  // Back
          if(_prevMenu == MORE_MENU) {
            moreMenu();
          }
          else if (_prevMenu == SAFEMODE_MENU ) {
            safeModeMenu();
          } 
          break;
      }
      break;
    case FACTORY_RESET_CONFIRM2_PAGE:
      switch (_currentSelection) {
        case 0:  // Perform factory reset
          factoryResetPage();
          doFactoryReset(false, false);
          break;
        case 1:  // Back
          if(_prevMenu == MORE_MENU) {
            moreMenu();
          }
          else if (_prevMenu == SAFEMODE_MENU ) {
            safeModeMenu();
          } 
          break;
      }
      break;
    case INFO_MENU:
      switch(_currentSelection) {
        case 0: // Back
          _currentMenu = MORE_MENU;
          moreMenu();

          break;
      }
      break;
    case SAFEMODE_MENU:
      _prevMenu = SAFEMODE_MENU;
      switch(_currentSelection) {
        case 0: // Restart
          // Confirm first (bench 2026-10-23): this row reset the device the
          // INSTANT any select landed on it — the safe menu's default row
          // was a hair trigger (a real select press on this row showed as a
          // mystery rst=0x4 "idle" reset with no wire trace, because the
          // reset fires inside the select, before any CH,5 push prints).
          // The normal menu's Restart goes through the confirm page; the
          // safe menu's does too now — same RESTART_PAGE confirm, and its
          // "... Back" row already returns to the safe menu.
          restartConfirmPage();
          break;
        case 1: 
          factoryResetConfirm2Page();
          break;
      }
  } // switch (_currentMenu)
} // selectMenuItem


//****************************************//
// Menu displaying functions
//****************************************//


//*********************************//
// Function   : setupDisplay
//
// Description: Clears the display, sets text size and color
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::setupDisplay() {
  if (!_begun) return;
  _display.clearDisplay();
  _scrollOn = false;

  _display.setTextSize(2);                              // 2x scale text
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // Draw white text on solid black background

  _display.setCursor(0, 0);
}


//*********************************//
// Function   : displayMenu
//
// Description: Prints each item in current menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::displayMenu() {
  if (!_begun || _currentMenuText == nullptr) return;
  setupDisplay();

  const int textLength = _cursorStart + _currentMenuLength;
  for (int i = 0; i < TEXT_ROWS; i++) {
    int textIndex = (i >= _cursorStart) ? (i + _countMenuScroll) : i;
    if (textIndex < 0 || textIndex >= textLength) break;
    if (i >= _cursorStart) {
      _display.print(" "), _display.println(_currentMenuText[textIndex]);
    } else {
      _display.println(_currentMenuText[textIndex]);
    }
  }

  if (_currentMenu == MODE_MENU) {
    modeMenuHighlight();
  }

  flushDisplay();

  //_currentSelection = 0;
  displayCursor();
}


//*********************************//
// Function   : displayCursor
//
// Description: Displays the selection cursor next to selected menu item
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::displayCursor() {
  if (!_begun) return;

  if (_currentSelection + _cursorStart > TEXT_ROWS - 1) {
    _cursorPos = TEXT_ROWS - 1;
  } else {
    _cursorPos = _currentSelection + _cursorStart;
  }

  _display.setTextSize(2);                              // 2x scale text
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // Draw white text on solid black background

  // Show cursor on text line of selection index, erase previous cursor
  //_display.setCursor(0, 16 * _cursorStart);           // Start at the line where the cursor should start 
  _display.setCursor(0, 0);           // Start at position 0,0
  for (int i = 0; i < TEXT_ROWS; i++) {
    if (i == _cursorPos) {
      _display.println(">");
    } else if (i < _cursorStart){
      _display.println("");
    } else {
      _display.println(" ");
    }
  }

  flushDisplay();

  _selectedLine = _cursorStart + _currentSelection;
  if (_currentMenuText == nullptr || _selectedLine < 0 || _selectedLine >= (_cursorStart + _currentMenuLength)) {
    _selectedText = "";
  } else {
    _selectedText = _currentMenuText[_selectedLine];
  }

  if (_selectedText.length() > (_maxCharPerLine - 1)) {
    //Serial.println("Long text");
    _scrollOn = true;
    _scrollPos = 12;
    scheduleTransition(200, screenScrollTransitionDone);  // was delay(200); scrollLongText() — fires once, 200 ms from now
  } else {
    _scrollOn = false;
  }
}


//*********************************//
// Function   : scrollLongText
//
// Description: Format and Display Long Menu Items by scrolling
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::scrollLongText() {
  if (!_begun) return;
  int minPos = -CHAR_PIXEL_WIDTH_S2 * _selectedText.length();

  _display.setTextSize(2);                              // 2x scale text
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // Draw white text on solid black background
  _display.setTextWrap(false);

  if (millis() - _scrollDelayTimer >= SCROLL_DELAY_MILLIS) {
    _scrollDelayTimer = millis();

    // Clear previous text by writing over it with blank text
    _display.setCursor(0, _cursorPos * CHAR_PIXEL_HEIGHT_S2);
    _display.print("                                   ");

    // Display text in new position to simulate scrolling
    _display.setCursor(_scrollPos, _cursorPos * CHAR_PIXEL_HEIGHT_S2);
    _display.print(_selectedText);

    _display.setCursor(0, _cursorPos * CHAR_PIXEL_HEIGHT_S2);
    _display.print(">");
    flushDisplay();

    _scrollPos = _scrollPos - scrollPixelsPerLoop;
    
    if (_scrollPos < minPos) {
      _scrollPos = _display.width();
    }
  }
}

//*********************************//
// Function   : drawCentreString
//
// Description: Format and Display Main Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::drawCentreString(const String &buf, int y) {
  if (!_begun) return;
  int16_t x1, y1;
  uint16_t w, h;
  int x = 64;
  _display.getTextBounds(buf, x, y, &x1, &y1, &w, &h);  // Calculate width of new string
  _display.setCursor(x - w / 2, y);
  _display.print(buf);
}

//********** MENUS **********//

//*********************************//
// Function   : mainMenu
//
// Description: Format and Display Main Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::mainMenu(void) {
  _currentMenu = MAIN_MENU;

  //if new menu selection
  //if (_prevMenu != _currentMenu) {
  _currentMenuLength = _mainMenuLen;
  _currentMenuText = _mainMenuText;
  _cursorStart = 0;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
  //}
}

//*********************************//
// Function   : exitConfirmMenu
//
// Description: Format and Exit Confirm Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::exitConfirmMenu() {
  _prevMenu = _currentMenu;
  _currentMenu = EXIT_MENU;
  _currentMenuText = _exitConfirmText;
  _currentMenuLength = 2;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;
  displayMenu();  //  Print items in current menu
}

void LSScreen::calibMenu(void) {
  _currentMenu = CALIB_MENU;
  if (_prevMenu != _currentMenu) {
    _currentMenuLength = _calibMenuLen;
    _currentMenuText = _calibMenuText;
    _cursorStart = 0;
    _currentSelection = 0;
    _countMenuScroll = 0;

    displayMenu();  //  Print items in current menu
  }
}


//*********************************//
// Function   : modeMenu
//
// Description: Format and display Operating Mode Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::modeMenu(void) {
  _currentMenu = MODE_MENU;

  _currentMenuLength = _modeMenuLen;
  _currentMenuText = _modeMenuText;
  _cursorStart = 0;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu

  modeMenuHighlight();
}


//*********************************//
// Function   : modeMenuHighlight
//
// Description: Format and display Operating Mode Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::modeMenuHighlight() {
  if (!_begun) return;

  _display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // Draw 'inverse' coloured text

  switch (_operatingMode) {
    case CONF_OPERATING_MODE_MOUSE:
      switch (_communicationMode) {
        case CONF_COM_MODE_USB:
          _display.setCursor(12, 0);
          _display.print(_modeMenuText[_MODE_MOUSE_USB - 1]);
          break;
        case CONF_COM_MODE_BLE:
          _display.setCursor(12, 16);
          //display.print(" MOUSE BLUETOOTH ");
          _display.print(_modeMenuText[_MODE_MOUSE_BT - 1]);
          break;
      }
      break;
    case CONF_OPERATING_MODE_GAMEPAD:
      _display.setCursor(12, 16 * 2);
      //display.print(" GAMEPAD ");
      _display.print(_modeMenuText[_MODE_GAMEPAD_USB - 1]);
      break;
  }

  flushDisplay();
  _display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // Reset text colour to white on black
}

//*********************************//
// Function   : confirmModeChange
//
// Description: Format and display Change Mode Confirm Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::confirmModeChange() {
  _currentMenu = CONFIRM_MODE_CHANGE;
  _currentMenuText = _modeConfirmText;
  _currentMenuLength = 2;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;
  displayMenu();  //  Print items in current menu
}


//*********************************//
// Function   : changeMode
//
// Description: Format and display Change Mode Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::changeMode() {
  if (!_begun) return;
  _currentMenu = CHANGING_MODE_PAGE;  // Name the dwell (4.5.7 review): the remote's post-press poll sees "Changing mode." while the 2 s window runs — the device resets at its end, so no restore is needed
  setupDisplay();
  _display.println("Changing");
  _display.println("mode.");
  _display.println("Release");
  _display.println("joystick.");
  flushDisplay();
  scheduleTransition(2000, screenChangeModeTransitionDone);  // was delay(2000) — finishChangeMode runs on the timer
}


//*********************************//
// Function   : cursorSpeedMenu
//
// Description: Format and display Cursor Speed Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::cursorSpeedMenu(void) {
  _currentMenu = CURSOR_SP_MENU;
  _cursorSpeedLevel = getCursorSpeed(false, false);  // (4.5.7 review) a read, not a change — no MANUAL wire line per visit

  _cursorSpMenuText[0] = "Speed: " + String(_cursorSpeedLevel);

  _currentMenuLength = _cursorSpMenuLen;
  _currentMenuText = _cursorSpMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : scrollSpeedMenu
//
// Description: Format and display Cursor Speed Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::scrollSpeedMenu(void) {
  _currentMenu = SCROLL_SP_MENU;
  _scrollSpeedLevel = getScrollLevel(false, false);  // (4.5.7 review) a read, not a change — no MANUAL wire line per visit

  _scrollSpMenuText[0] = "Speed: " + String(_scrollSpeedLevel);

  _currentMenuLength = _scrollSpMenuLen;
  _currentMenuText = _scrollSpMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;  // 4.5.7 review: stale More-menu scroll offset indexed past this 4-element array

  displayMenu();
}

//*********************************//
// Function   : accelMenu
//
// Description: Format and display the Cursor Acceleration Menu (roadmap
//              item 9): the stored level (-10..10, 0 = linear) with the
//              same Increase/Decrease/Back shape as the speed menus.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::accelMenu(void) {
  _currentMenu = ACCEL_MENU;
  _accelLevel = getJoystickAcceleration(false, false);  // (4.5.7 review) a read, not a change — no MANUAL wire line per visit

  _accelMenuText[0] = "Accel: " + String(_accelLevel);

  _currentMenuLength = _accelMenuLen;
  _currentMenuText = _accelMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;  // 4.5.7 review: stale More-menu scroll offset indexed past this 4-element array

  displayMenu();
}

//*********************************//
// Function   : restrictionsMenu
//
// Description: Format and display the input restrictions menu (roadmap
//              item 11). One line per restriction, state shown inline —
//              the line both reads and toggles.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::restrictionsMenu(void) {
  _currentMenu = RESTRICTIONS_MENU;

  _restrictionsMenuText[0] = String("Sip&Puff: ") + ((getSapDisabled(false, false) == 1) ? "Off" : "On");
  _restrictionsMenuText[1] = String("Menu: ") + ((getMenuButtonsOnly(false, false) == 1) ? "Buttons" : "All");
  _restrictionsMenuText[2] = String("Mode lock: ") + ((getModeLocked(false, false) == 1) ? "On" : "Off");
  _restrictionsMenuText[3] = String("Cursor hold: ") + ((getCursorHoldDuringSap(false, false) == 1) ? "On" : "Off");

  _currentMenuLength = _restrictionsMenuLen;
  _currentMenuText = _restrictionsMenuText;
  _cursorStart = 0;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : timingMenu
//
// Description: Format and display the input timing windows menu (roadmap
//              item 15, upstream #62): one line per window boundary,
//              current value in ms shown inline.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::timingMenu(void) {
  _currentMenu = TIMING_MENU;

  _timingMenuText[0] = String("Short: ") + String(getInputWindowShort(false, false)) + "ms";
  _timingMenuText[1] = String("Long: ") + String(getInputWindowLong(false, false)) + "ms";
  _timingMenuText[2] = String("Max: ") + String(getInputWindowMax(false, false)) + "ms";

  _currentMenuLength = _timingMenuLen;
  _currentMenuText = _timingMenuText;
  _cursorStart = 0;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : adjustTimingMenu
//
// Description: Format and display the adjust page for the selected input
//              timing window (_timingWindowSel: 0 = short/WS, 1 = long/WL,
//              2 = max/WM). Increase/Decrease step 250 ms; the API setter
//              clamps range and ordering, so a refused step re-reads as the
//              unchanged value.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::adjustTimingMenu(void) {
  _currentMenu = TIMING_ADJUST_MENU;

  int currentMs = 0;
  if (_timingWindowSel == 0) currentMs = getInputWindowShort(false, false);
  else if (_timingWindowSel == 1) currentMs = getInputWindowLong(false, false);
  else currentMs = getInputWindowMax(false, false);

  _timingAdjustMenuText[0] =
    String(_timingWindowSel == 0 ? "Short: " : _timingWindowSel == 1 ? "Long: " : "Max: ") + String(currentMs) + "ms ";

  _currentMenuLength = _timingAdjustMenuLen;
  _currentMenuText = _timingAdjustMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : moreMenu
//
// Description: Format and display More Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::moreMenu() {
  _currentMenu = MORE_MENU;

  _currentMenuLength = _moreMenuLen;
  _currentMenuText = _moreMenuText;
  _cursorStart = 0;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

// ----- CALIBRATION PAGES ----- //

//*********************************//
// Function   : centerResetPage
//
// Description: Format and display Center Reset Start Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::centerResetPage(void) {
  if (!_begun) return;
  // Freeze menu input for the flow's duration (4.5.7 review): the ~2.6 s
  // timer-driven center used to leave the page live to the input poll — a
  // Next redrew the CALIB menu over the "do not move joystick"
  // instruction, and a Select fell through selectMenuItem's switch with
  // no case for CENTER_RESET_PAGE (silently swallowed). Every comparable
  // informational page freezes its window (modeLockedPage, the confirm
  // dwells); the center flow's own timer chain is the continuation, so
  // only the freeze is needed here. The complete page redraws when the
  // chain ends.
  _inputFrozenUntil = millis() + CONF_CENTER_RESET_FREEZE_MS;
  setupDisplay();

  _display.println("Center");
  _display.println("reset, do");
  _display.println("not move");
  _display.println("joystick");

  flushDisplay();

  // 4.5.7 review: the flow's continuation lives on calibrationTimer, which
  // only runs when the joystick sensor is connected — on a safe boot
  // without one (the menu is serial-openable), this page registered no
  // continuation and parked forever after its 3 s freeze. Without the
  // sensor there is nothing to center: schedule the return instead.
  if (!g_joystickSensorConnected) {
    showCenterResetComplete = false;
    scheduleTransition(2000, screenReturnToMenuTransitionDone);
    return;
  }

  // Perform cursor center
  showCenterResetComplete = true;
  setJoystickInitialization(true, false);
  if (!g_centerRunning) {
    showCenterResetComplete = false;
    scheduleTransition(1000, screenReturnToMenuTransitionDone);
  }
}

//*********************************//
// Function   : centerResetCompletePage
//
// Description: Format and display Center Reset Complete Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::centerResetCompletePage(void) {
  showCenterResetComplete = false;
  if (!_begun) return;

  _display.clearDisplay();
  _display.setCursor(0, 0);
  _display.println("Center");
  _display.println("reset");
  _display.println("complete");

  flushDisplay();

  scheduleTransition(2000, screenReturnToMenuTransitionDone);  // was delay(2000) — returnToMenu runs on the timer
}

void LSScreen::centerResetFailedPage(void) {
  showCenterResetComplete = false;
  if (!_begun) {
    returnToMenu();
    return;
  }
  setupDisplay();
  _display.println("Center");
  _display.println("reset");
  _display.println("failed");
  _display.setTextSize(1);
  _display.println("Keep prior center");
  flushDisplay();
  scheduleTransition(2000, screenReturnToMenuTransitionDone);
}

//*********************************//
// Function   : fullCalibrationPage
//
// Description: Format and display Full Calibration Start Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::fullCalibrationPage(void) {
  if (!_begun) return;
  _currentMenu = FULL_CALIB_PAGE;  // Name the run (4.5.7 review): CH,0:0 answers the page actually shown for a mid-run poll; a mid-run Select has no case here (dropped) until the prompts/completion re-register real menus
  // 4.5.7 review: mirror centerResetPage's sensor guard — without the
  // joystick sensor the flow's chain never runs (calibrationTimer is only
  // pumped when the sensor is connected), and this page would sit
  // indefinitely (no continuation, and the safe-mode menu timeout is
  // disabled). The serial CA,1:1 path still gets its honest FAIL,3 from
  // setJoystickCalibration's own guard; the menu path returns to the menu.
  if (!g_joystickSensorConnected) {
    scheduleTransition(2000, screenReturnToMenuTransitionDone);
    return;
  }
  setupDisplay();

  _display.println("Follow");
  _display.println("on screen");
  _display.println("prompts");

  flushDisplay();

  setJoystickCalibration(false, false);
  if (!g_calibrationRunning) {
    // A center/calibration flow was already active or the timer registration
    // failed. Do not strand the menu on a prompt whose continuation will
    // never run.
    scheduleTransition(1000, screenReturnToMenuTransitionDone);
  }
}

//*********************************//
// Function   : fullCalibrationPrompt
//
// Description: Format and display a  page for each step of the full calibration.
//
// Arguments :  stepNum : int : The number of the calibration step
//
// Return     : void
//*********************************//
void LSScreen::fullCalibrationPrompt(int stepNum) {
  if (!_begun) {
    if (stepNum == 6) returnToMenu();
    return;
  }
  // 4.5.7 review: roll the input freeze across the run's steps (each
  // prompt re-arms a window that covers the step) — a mid-run NEXT used to
  // repaint the confirm page's selection column over the instruction
  // prompt, because the run registered no freeze. Step 6 (complete)
  // schedules its own return-to-menu transition and must NOT freeze past
  // it.
  if (stepNum >= 1 && stepNum <= 5) {
    _inputFrozenUntil = millis() + CONF_FULL_CALIB_STEP_FREEZE_MS;
  }
  setupDisplay();

  switch (stepNum) {
    case 1:  // Corner 1
      _display.println("Hold");
      _display.println("joystick");
      _display.println("top left");
      break;
    case 2:  // Corner 2
      _display.println("Hold");
      _display.println("joystick");
      _display.println("top right");
      break;
    case 3:  // Corner 3
      _display.println("Hold");
      _display.println("joystick");
      _display.println("bottom");
      _display.println("right");
      break;
    case 4:  // Corner 4
      _display.println("Hold");
      _display.println("joystick");
      _display.println("bottom");
      _display.println("left");
      break;
    case 5:  // Center
      _display.println("Release,");
      _display.println("do not");
      _display.println("move");
      _display.println("joystick");
      break;
    case 6:  // Complete
      _display.println("Joystick");
      _display.println("calibrated");
      flushDisplay();

      scheduleTransition(1500, screenReturnToMenuTransitionDone);  // was delay(1500) — returnToMenu runs on the timer
      break;
    case CONF_JOY_CALIB_ERROR:  // One or more values were too low to be replaced, flag error message
      _display.println("Movement");
      _display.println("too small.");
      _display.println("Returned");
      _display.println("to default.");
  }

  flushDisplay();
}

void LSScreen::calibrationAbortedPage(void) {
  if (!_begun) {
    returnToMenu();
    return;
  }
  setupDisplay();
  _display.println("Calibration");
  _display.println("failed");
  _display.setTextSize(1);
  _display.println("Previous values kept");
  flushDisplay();
  scheduleTransition(2000, screenReturnToMenuTransitionDone);
}

// ----- MORE SETTINGS MENUS ----- //

//*********************************//
// Function   : soundMenu
//
// Description: Format and display Sound Menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::soundMenu(void) {
  _currentMenu = SOUND_MENU;

  if (_soundMode != CONF_SOUND_MODE_OFF) {
    _soundMenuText[1] = "ON";
    _soundMenuText[2] = "Turn off";
  } else {
    _soundMenuText[1] = "OFF";
    _soundMenuText[2] = "Turn on";
  }

  _currentMenuLength = _soundMenuLen;
  _currentMenuText = _soundMenuText;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;  // 4.5.7 review: stale More-menu scroll offset indexed past this 4-element array

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : lightBrightnessMenu
//
// Description: Format and display light brightness menu
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::lightBrightMenu(void) {
  _currentMenu = LIGHT_BRIGHT_MENU;
  _lightBrightLevel = getLightBrightnessLevel(false, false);

  _lightBrightMenuText[0] = "Lights: " + String(_lightBrightLevel);

  _currentMenuLength = _lightBrightMenuLen;
  _currentMenuText = _lightBrightMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : sipPuffThreshMenu
//
// Description: Format and display Sip and Puff Menu Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::sipPuffThreshMenu(void) {
  _currentMenu = SIP_PUFF_MENU;

  _currentMenuLength = _sipPuffThreshMenuLen;
  _currentMenuText = _sipPuffThreshMenuText;
  _cursorStart = 0;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : adjustSipThreshMenu
//
// Description: Format and display Sip Threshold Adjustment Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::adjustSipThreshMenu(void) {
  _currentMenu = SIP_THRESH_MENU;
  _sipPressThresh = getSipPressureThreshold(false, false);

  _adjustSipThreshMenuText[0] =
    "Sip: L" + String(pressThresholdLevelFor(_sipPressThresh)) + " " + String(_sipPressThresh) + " ";

  _currentMenuLength = _adjustSipThreshMenuLen;
  _currentMenuText = _adjustSipThreshMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : adjustPuffThreshMenu
//
// Description: Format and display Puff Threshold Adjustment Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::adjustPuffThreshMenu(void) {
  _currentMenu = PUFF_THRESH_MENU;
  _puffPressThresh = getPuffPressureThreshold(false, false);

  _adjustPuffThreshMenuText[0] =
    "Puff: L" + String(pressThresholdLevelFor(_puffPressThresh)) + " " + String(_puffPressThresh) + " ";

  _currentMenuLength = _adjustPuffThreshMenuLen;
  _currentMenuText = _adjustPuffThreshMenuText;
  _cursorStart = 1;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : fullCalibrationConfirmPage
//
// Description: Format and display Full Calibration Confirmation Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::fullCalibrationConfirmPage(void) {
  if (!_begun) return;
  _currentMenu = FULL_CALIB_WARN_PAGE;  // Name the dwell (4.5.7 review): the 3 s warning window reads as itself on CH,0:0; the continuation restores FULL_CALIB_CONFIRM_PAGE
  setupDisplay();
  _display.println("Incorrect");
  _display.println("full calib.");
  _display.println("may cause");
  _display.println("drift.");
  flushDisplay();
  scheduleTransition(3000, screenFullCalibConfirmTransitionDone);  // was delay(3000) — the confirm menu registers on the timer
}

//*********************************//
// Function   : restartConfirmPage
//
// Description: Format and display Restart Confirmation Page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::restartConfirmPage(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::restartConfirmPage()"); }
 
  _currentMenu = RESTART_PAGE;
  _currentMenuLength = _restartConfirmLen;
  _currentMenuText = _restartConfirmText;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}


//*********************************//
// Function   : factoryResetConfirm1Page
//
// Description: Format and display Factory Reset Confirmation Page 1
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::factoryResetConfirm1Page(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::factoryResetConfirm1Page()"); }

  _currentMenu = FACTORY_RESET_PAGE;
  _currentMenuLength = _factoryResetConfirm1Len;
  _currentMenuText = _factoryResetConfirm1Text;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : factoryResetConfirm2Page
//
// Description: Format and display Factory Reset Confirmation Page 2
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::factoryResetConfirm2Page(void) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::factoryResetConfirm2Page()"); }

  _currentMenu = FACTORY_RESET_WARN_PAGE;  // Name the dwell (4.5.7 review): the 2 s warning window reads as itself on CH,0:0; the continuation restores FACTORY_RESET_CONFIRM2_PAGE
  setupDisplay();
  _display.println("This will");
  _display.println("erase all");
  _display.println("custom");
  _display.println("settings");
  flushDisplay();
  scheduleTransition(2000, screenFactoryResetConfirm2TransitionDone);  // was delay(2000) — the confirm menu registers on the timer
}

//*********************************//
// Function   : fillMenuState
//
// Description: Reports the live menu state for the CH,0:0 menu-sync
//              endpoint: menu active flag, current page id (the
//              MAIN_MENU/…/TIMING_ADJUST_MENU defines), and the current
//              row selection — the SELECTABLE-row index (the same index
//              selectMenuItem() switches on; add the page's _cursorStart
//              offset for the text-array row). Read-only, so
//              it is safe mid-transition: during a scheduled page's frozen
//              window the values name the page the Hub is actually showing
//              (4.5.7 review: the draw-only informational dwells now NAME
//              themselves — EXITING_PAGE, CHANGING_MODE_PAGE,
//              FULL_CALIB_WARN_PAGE, FACTORY_RESET_WARN_PAGE — exactly as
//              MODE_LOCKED_PAGE already did; the continuation restores the
//              real page id).
//
// Arguments :  out : int[3] : filled with [active, pageId, row]
//
// Return     : void
//*********************************//
void LSScreen::fillMenuState(int out[3]) {
  // No menu page registered yet (the safe-mode/error boot dwell pages set
  // _isActive before their continuation registers the menu): the boot pages
  // are not menu truth — answer "no menu", the same honesty the CH,5 push's
  // null gate enforces (4.5.5 follow-up: the app polls CH,0:0 after every
  // press, and a press dropped in the dwell window would otherwise sync its
  // mirror onto a "menu active, MAIN page" that does not exist).
  out[0] = (_isActive && _currentMenuText != nullptr) ? 1 : 0;
  out[1] = _currentMenu;
  out[2] = _currentSelection;
}

//*********************************//
// Function   : reportMenuState
//
// Description: The push half of Hub remote control (the CH,5 menu-state
//              push, two-way remote): whenever the live [active, page, row]
//              triple changes, print MANUAL,0:CH,5:<active>,<page>,<row> —
//              the same triple the CH,0:0 read answers — so the app's
//              remote mirror follows PHYSICAL navigation (Next/Select on
//              the Hub, joystick-driven menu steps, the inactivity
//              timeout) instead of drifting until the next app press.
//              Called from the flushDisplay choke point, where every menu
//              redraw ends; the change-detect below is the entire rate
//              limiter — value screens re-render on every adjust step and
//              the long-text scroll flushes every 100 ms, and neither
//              changes the triple, so neither prints.
//
//              Wire safety is the 4.5.2 initiator contract: MANUAL + seq 0
//              is device-initiated and matches no command waiter (the boot
//              re-center precedent), printed unconditionally like every
//              unsolicited line this firmware emits (the CDC stack drops
//              writes with no host listening). The triple comes from
//              fillMenuState verbatim, so the poll and the push can never
//              disagree.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::reportMenuState() {
  if (_currentMenuText == nullptr) return;  // No menu page registered yet (safe-mode/error boot dwell): the boot pages are not menu truth, and pushing their (1, MAIN_MENU, 0) triple would tell the app a menu that does not exist yet

  int menuState[3];
  fillMenuState(menuState);
  if (menuState[0] == _lastReportedMenuState[0] &&
      menuState[1] == _lastReportedMenuState[1] &&
      menuState[2] == _lastReportedMenuState[2]) {
    return;
  }
  _lastReportedMenuState[0] = menuState[0];
  _lastReportedMenuState[1] = menuState[1];
  _lastReportedMenuState[2] = menuState[2];
  printResponseIntArray(true, false, true, 0, "CH,5", true, "", 3, ',', menuState);
}

//*********************************//
// Function   : modeLockedPage
//
// Description: The operating-mode lock's refusal page: names the reason and
//              the unlock path instead of answering a mode selection with
//              only an error cue (bench feedback 2026-10-16 — a bare beep
//              reads as a dead button or generic failure). Shown for ~2 s
//              with menu input frozen, then the continuation redraws the
//              Mode page where "... Back" is always live.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::modeLockedPage(void) {
  if (!_begun) return;
  // Name the page in the menu state (CH,0:0 reads it): a remote pressing
  // a locked mode row must be able to tell "refused, reason showing" apart
  // from "still on the Mode options" — this is the drift case that built
  // the endpoint. finishModeLocked → modeMenu() restores MODE_MENU.
  _currentMenu = MODE_LOCKED_PAGE;
  setupDisplay();
  _display.println("Mode is");
  _display.println("locked.");
  _display.println("Unlock via");
  _display.println("the app");
  flushDisplay();
  scheduleTransition(2000, screenModeLockedTransitionDone);
}

//*********************************//
// Function   : finishModeLocked
//
// Description: Scheduled end of the locked page: redraw the Mode options.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::finishModeLocked(void) {
  _transitionTimerId = -1;  // The scheduled transition just fired — its slot is free; keep the id honest (the one continuation that broke this invariant before the 4.5.7 review)
  modeMenu();
}

//*********************************//
// Function   : infoMenu
//
// Description: Format and display an information page
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::infoMenu(void) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::infoPage()"); }

  _infoMenuText[1] = "V"+CONF_LIPSYNC_VERSION_STRING;
  // Clip the ID line (4.5.7 review): "LS_" + the 8-hex-char UID is 11
  // characters = 132 px at size 2 against the 128-px panel — the tail of
  // the UID was cut off mid-glyph. Truncate to the 10 characters that fit.
  String idLine = "LS_" + g_deviceUID;
  if (idLine.length() > 10) idLine = idLine.substring(0, 10);
  _infoMenuText[2] = idLine;

  _currentMenu = INFO_MENU;
  _currentMenuLength = _infoMenuLen;
  _currentMenuText = _infoMenuText;
  _cursorStart = 3;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : testPage
//
// Description: Format and display a test page that shows the operating mode and number of times this function has been called
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::testPage(unsigned int usbConnectDelay) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::testPage()"); }

  setupDisplay();
  _testScreenAttempt++;

  switch (_operatingMode) {
    case CONF_OPERATING_MODE_MOUSE:
      switch (_communicationMode) {
        case CONF_COM_MODE_USB:
          _display.println("USB Mouse");
          break;
        case CONF_COM_MODE_BLE:
          _display.println("BT Mouse");
          break;
      }
      break;
    case CONF_OPERATING_MODE_GAMEPAD:
      _display.println("Gamepad");
      break;
    default:
      _display.println("Error");
  }
  _display.print("Attempt:");
  _display.println(_testScreenAttempt);

  _display.println(usbConnectDelay);
  flushDisplay();
}

//*********************************//
// Function   : noUsbPage
//
// Description: Format and display an error page when the USB has not successfully connected.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::noUsbPage(void) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::noUsbPage()"); }

  setupDisplay();
  _display.println("No USB.");
  _display.println("Use menu");
  _display.println("to change");
  _display.println("modes.");
  flushDisplay();
}

//*********************************//
// Function   : updateModePage
//
// Description: The deliberate update-mode splash (firmware-update
//              improvements item 1): the stock bootloader never drives the
//              Hub's OLED, so whatever the app firmware drew last stays
//              frozen for the whole flash. Drawing this page as the final
//              frame before the jump replaces the confusing frozen
//              "Restarting" message with the truth — the device is fine
//              and waiting for the flash. The OLED is unpowered-fresh: it
//              keeps this frame without power until the new app boots.
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::updateModePage(void) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::updateModePage()"); }

  setupDisplay();
  _display.setTextSize(2);
  drawCentreString("Update", 0);
  drawCentreString("mode", 16);
  _display.setTextSize(1);
  drawCentreString("Use the LipSync", 40);
  drawCentreString("Connect app", 52);
  flushDisplay();
}

// (4.5.6) The hardware-error page CHAIN (hardwareErrorPage -> errorPageI2C /
// errorPageCable -> safeMenu) was collapsed into safeModePage's HARDWARE case:
// one synchronous draw of the same error list plus a single +3 s continuation
// to the safe menu. The chain's three page functions were dead code from that
// moment and are removed per the af91082 discipline; the safeModePage comment
// documents why the chain had to go.

//*********************************//
// Function   : warningUSBDebugOn
//
// Description: Format and display an error page when USB_DEBUG is set to 1 (on)
//
// Arguments :  void
//
// Return     : void
//*********************************//
void LSScreen::warningUSBDebugOn(void) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::warningUSBDebugOn()"); }

  setupDisplay();
  _display.println("Warning:");
  _display.println("USB_DEBUG=1");
  _display.println("Set to 0");
  _display.println("for user.");
  flushDisplay();

  if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
    _screenStateTimer.deleteTimer(_screenStateTimerId);
  }
  _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::finishSplashClear>(CONF_SPLASH_SCREEN_DURATION, this);
}

//*********************************//
// Function   : restartPage
//
// Description: Format and display an page showing the device is going to be reset
//
// Arguments :  void
//
// Return     : void
//*********************************//

void LSScreen::restartPage() {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::restartPage()"); }

  setupDisplay();
  _display.println("");
  _display.println("RESTARTING...");
  _display.println("");

  flushDisplay();

  const int RESTART_TIMEOUT = 3000;
  if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
    _screenStateTimer.deleteTimer(_screenStateTimerId);
  }
  _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::finishSplashClear>(RESTART_TIMEOUT, this);
}

//*********************************//
// Function   : factoryResetPage
//
// Description: Format and display an page showing the device is going to be reset
//
// Arguments :  void
//
// Return     : void
//*********************************//

void LSScreen::factoryResetPage() {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::factoryResetPage()"); }

  setupDisplay();
  _display.println("FACTORY");
  _display.println("RESET...");
  _display.println("");

  flushDisplay();
  const int RESET_TIMEOUT = 3000;
  if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
    _screenStateTimer.deleteTimer(_screenStateTimerId);
  }
  _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::finishSplashClear>(RESET_TIMEOUT, this);
}

//*********************************//
// Function   : connectionTimingPage
//
// Description: Format and display an page showing the timestamps related to connecting to USB
//
// Arguments :  void
//
// Return     : void
//*********************************//

void LSScreen::connectionTimingPage(unsigned long before, unsigned long after) {
  if (!_begun) return;
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::connectionTimingPage()"); }

  setupDisplay();
  _display.setTextSize(1);
  _display.println("Before beginComOpMode");
  _display.setTextSize(2);
  _display.println(before);

  _display.setTextSize(1);
  _display.println("After beginComOpMode");
  _display.setTextSize(2);
  _display.println(after);

  flushDisplay();
}


//*********************************//
// Function   : safeModePage
//
// Description: Format and display an page showing that device is in safe boot mode
//
// Arguments :  int safeModeReason
//
// Return     : void
//*********************************//

void LSScreen::safeModePage(int safeModeReason) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::safeModePage()"); }
  bootTrailStamp(2);  // witness v7: a safe/degraded boot's dwell window begins

  if (!_begun) {
    // A missing/failed display has no framebuffer; every Adafruit_GFX draw
    // dereferences it. Safe mode remains fully recoverable over serial, but
    // there is no honest visual menu to activate on a headless unit.
    _isActive = false;
    _currentMenuText = nullptr;
    return;
  }

  _isActive = true;
  // Freeze menu input for the dwell (the safe-mode menu registers on the
  // continuation): the 4.2.0 non-blocking conversion made this window live
  // to the input poll and the serial API, and a Next/Select arriving before
  // safeModeMenu() registers state hit a null menu — the frozen window drops
  // those presses exactly as the old blocking pages did.
  _inputFrozenUntil = millis() + CONF_SAFEMODE_MENU_TIMEOUT;
  setupDisplay();

  _display.println("SAFE MODE: ");
 
  switch(safeModeReason) {
        case CONF_SAFE_MODE_REASON_INPUT:
        {
          _display.println(" Hub");
          if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
            _screenStateTimer.deleteTimer(_screenStateTimerId);
          }
          _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::safeModeMenu>(CONF_SAFEMODE_MENU_TIMEOUT, this);
          _safeModeReasonText = "Hub";
          
          break;
        }
        case CONF_SAFE_MODE_REASON_WATCHDOG:
        {
          _display.println(" Watchdog");
          if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
            _screenStateTimer.deleteTimer(_screenStateTimerId);
          }
          _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::safeModeMenu>(CONF_SAFEMODE_MENU_TIMEOUT, this);
          _safeModeReasonText = "Watchdog";
          break;
        }
        case CONF_SAFE_MODE_REASON_HARDWARE:
        {
          // THE FREEZE FIX (bench 2026-10-23, five witness rounds): every
          // freeze on this hardware landed inside the hardware-error dwell
          // CHAIN's timer windows (safeModePage -> +3 s hardwareErrorPage ->
          // +3 s errorPageI2C/Cable -> +3 s safeModeMenu, ms 0.5-9.5) — the
          // witness caught the screen pool's continuations dying mid-chain
          // (trail 21/22 with the next page's stamp due and never landing)
          // while the serial API kept answering, then the loop. The chain's
          // content is informational only, so it collapses to ONE
          // synchronous draw here plus a single +3 s continuation straight
          // to the safe menu — same error list, same error code, same
          // recovery path, none of the intermediate pages' timer windows.
          _display.println(" Hardware");
          _hardwareErrorCode = 0;
          if (!g_joystickSensorConnected) {
            _display.println(" JOYSTICK");
            _hardwareErrorCode |= 1 << 0;
          }
          if (!g_mouthpiecePressureSensorConnected) {
            _display.println(" PRESSURE");
            _hardwareErrorCode |= 1 << 1;
          }
          if (!g_ambientPressureSensorConnected) {
            _display.println(" AMBIENT");
            _hardwareErrorCode |= 1 << 2;
          }
          if (!g_displayConnected) {
            _hardwareErrorCode |= 1 << 3;
          }
          {
            // snprintf (4.5.7 review): the 4-bit code's worst case
            // "ERROR-015" + NUL is exactly 10 — a fifth error bit would
            // have overflowed the fixed buffer.
            char buffer[10];
            snprintf(buffer, sizeof(buffer), "ERROR-%03u", _hardwareErrorCode);
            _safeModeReasonText = String(buffer);
          }
          if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
            _screenStateTimer.deleteTimer(_screenStateTimerId);
          }
          _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::safeModeMenu>(CONF_SAFEMODE_MENU_TIMEOUT, this);
          break;
        }
        case CONF_SAFE_MODE_REASON_HANG:
        {
          // 4.5.7 review: repeated witnessed loop hangs settled here (see
          // checkSafeMode/LSWatchdog.h) — same single continuation to the
          // safe menu as the other reasons. Without an explicit case the
          // switch's default armed NO continuation and the safe menu never
          // registered (a latent trap for any reason value outside 1-3).
          _display.println(" Repeated");
          _display.println(" hang");
          _safeModeReasonText = "Hang";
          if (_screenStateTimerId >= 0) {  // Delete-before-rearm — see splashScreen2 (4.5.7 review)
            _screenStateTimer.deleteTimer(_screenStateTimerId);
          }
          _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::safeModeMenu>(CONF_SAFEMODE_MENU_TIMEOUT, this);
          break;
        }
        default:
        {
          _display.println("");
          _safeModeReasonText = "Unknown";
          if (_screenStateTimerId >= 0) {
            _screenStateTimer.deleteTimer(_screenStateTimerId);
          }
          _screenStateTimerId = _screenStateTimer.setTimeout<LSScreen, &LSScreen::safeModeMenu>(CONF_SAFEMODE_MENU_TIMEOUT, this);
          break;
        }
  }
  if (_screenStateTimerId < 0) {
    safeModeMenu();
    return;
  }
  _display.println("");

  flushDisplay();
}

//*********************************//
// Function   : safeModeMenu
//
// Description: Format and display an page showing that device is in safe boot mode
//
// Arguments :  void
//
// Return     : void
//*********************************//

void LSScreen::safeModeMenu(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::safeModeMenu()"); }
  bootTrailStamp(3);  // witness v7: the safe-mode menu registered
  _screenStateTimerId = -1;  // The boot-page chain's timer just fired — its slot is free; keep the id honest (see finishSplashClear)

  _safeModeMenuText[1] = _safeModeReasonText;

  _currentMenu = SAFEMODE_MENU;
  _currentMenuLength = _safeModeMenuLen;
  _currentMenuText = _safeModeMenuText;
  _cursorStart = 2;
  _currentSelection = 0;
  _countMenuScroll = 0;

  displayMenu();  //  Print items in current menu
}

//*********************************//
// Function   : print4LineString
//
// Description: Format and display a page with 4 lines of Strings
//
// Arguments :  String s1, String s2, String s3, String s4 (four lines of text as Strings)
//
// Return     : void
//*********************************//

void LSScreen::print4LineString(String s1, String s2, String s3, String s4) {
  if (!_begun) return;

  setupDisplay();
  _display.println(s1);
  _display.println(s2);
  _display.println(s3);
  _display.println(s4);
  flushDisplay();
}

//*********************************//
// Function   : enableTimeout
//
// Description: Enables the display timeout
//
// Arguments :  void
//
// Return     : void
//*********************************//

void LSScreen::enableTimeout(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::enableTimeout()"); }
  
  _menuTimeoutEnabled = true;
}

//*********************************//
// Function   : disableTimeout
//
// Description: Disables the display timeout
//
// Arguments :  void
//
// Return     : void
//*********************************//

void LSScreen::disableTimeout(void) {
  if (USB_DEBUG) { Serial.println("USBDEBUG: LSScreen::disableTimeout()"); }

  _menuTimeoutEnabled = false;
}

#endif
