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
bool g_safeModeEnabled = false;
int  g_safeModeReason = 0;
bool readyToUseFirstTime = true;

bool g_btIsConnected = false;   // Bluetooth connection state
bool g_usbIsConnected = false;  // USB Connection state

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

int pollTimerId[10];  // 9 poll timers
LSTimer<void> pollTimer;

int ledTimerId[5];  // 3 LED timers 0 - startup feedback, 1 - IBM, 2- normal blinks, 3 - Bluetooth Status, 4 - error
LSTimer<ledStateStruct> ledStateTimer;

int usbConnectTimerId[1];
LSTimer<int> usbConnectTimer;

unsigned int g_usbAttempt = 0;
unsigned int g_usbConnectDelay = CONF_USB_HID_INIT_DELAY;
unsigned long g_usbNotReadySinceMs = 0;  // When the current HID-not-ready stretch began; 0 = none in progress
unsigned int g_usbReenumCount = 0;        // Forced re-enumerations spent on the current episode

// Joystick module variables and structures
int acceleration = 0;
int g_scrollLevel = 0;
int g_scrollNumRuns = 0;

int outputAction;
bool canOutputAction = true;
bool g_startupCenterReset = true;
bool g_resetCenterComplete = false;  // global variable for center reset status
bool g_calibrationError = false;     // Global variable for error in full calibration

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

  initMemory();  // Initialize Memory

  //initUID(); // Intilize unique identifier
  getDeviceID(false, false);       // Retrieve Device ID from memory
  getVersionNumber(false, false);  // Retrieve version number from memory

  initLed();  // Initialize LED Feedback

  initBuzzer();  // Initialize Buzzer

  initInput();  // Initialize input buttons and input switches

  initRestrictions();  // Load the input restriction settings (roadmap item 11) before any input can fire


  ledWaitFeedback();  // Turn on all LEDS

  beforeComOpMillis = millis() - beginMillis;  // Note time before USB/BT connection
  beginComOpMode();                            // Initialize Operating Mode, Communication Mode, and start instance of mouse or gamepad
  afterComOpMillis = millis() - beginMillis;   // Note time after USB/BT connection


  checkSafeMode();  // Check to see if we need to boot in safe mode.

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


  pollTimer.disable(CONF_TIMER_USB); // TODO 2025-Feb-21 Disable usbConnectionLoop until implemented
  // If USB is not connected, try to reconnect
  if (g_comMode == CONF_COM_MODE_USB) {
    usbConnectTimerId[0] = usbConnectTimer.setTimeout(g_usbConnectDelay, usbCheckConnection);  // Call usbCheckConnection function when g_usbConnectDelay reached
  }


  //enablePoll(true);

  ledActionEnabled = true;

  //ledReadyFeedback();

  if (g_displayConnected) {
    pollTimer.enable(CONF_TIMER_SCREEN);
  } else {
    pollTimer.disable(CONF_TIMER_SCREEN);
  }

  if (g_mouthpiecePressureSensorConnected && g_ambientPressureSensorConnected) {
    pollTimer.enable(CONF_TIMER_PRESSURE);
    if (USB_DEBUG) { Serial.println("USBDEBUG: Pressure timer started."); }
  } else {
    pollTimer.disable(CONF_TIMER_PRESSURE);
    if (USB_DEBUG) { Serial.println("USBDEBUG: Pressure timer NOT started."); }
  }

  if (g_joystickSensorConnected) {
    pollTimer.enable(CONF_TIMER_JOYSTICK);
    pollTimer.enable(CONF_TIMER_SCROLL);
  } else {
    pollTimer.disable(CONF_TIMER_JOYSTICK);
    pollTimer.disable(CONF_TIMER_SCROLL);
  }

  // If any devices are not connected, handle error
  if (!g_displayConnected || !g_mouthpiecePressureSensorConnected || !g_ambientPressureSensorConnected || !g_joystickSensorConnected) {
    hardwareErrorCheck();
  }

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
  }

  if (USB_DEBUG) { Serial.print("USBDEBUG: lastRebootReason: "); Serial.println(g_lastRebootReason); }
  if (USB_DEBUG) { Serial.println("USBDEBUG: Setup complete."); }

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
  g_usbIsConnected = false;
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
    //pollTimer.disable(CONF_TIMER_SCREEN);
    pollTimer.disable(CONF_TIMER_PRESSURE);
    pollTimer.disable(CONF_TIMER_JOYSTICK);
    pollTimer.disable(CONF_TIMER_SCROLL);
    pollTimer.disable(CONF_TIMER_DEBUG);
    pollTimer.disable(CONF_TIMER_BLUETOOTH);

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
    pollTimer.enable(CONF_TIMER_PRESSURE);
    pollTimer.enable(CONF_TIMER_INPUT);
    pollTimer.enable(CONF_TIMER_BLUETOOTH);
  } else {
    pollTimer.disable(CONF_TIMER_JOYSTICK);
    pollTimer.disable(CONF_TIMER_PRESSURE);
    pollTimer.disable(CONF_TIMER_INPUT);
    pollTimer.disable(CONF_TIMER_BLUETOOTH);
    pollTimer.disable(CONF_TIMER_DEBUG);
    pollTimer.disable(CONF_TIMER_SCROLL);
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
  mem.begin();  // Begin memory
  
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

//***CLEAR MENU SCREEN FUNCTION***//
// Function   : closeMenu
//
// Description: This function closes the menu and clears the screen
//
// Parameters : void
//
// Return     : void
//****************************************//
void closeMenu() {
  screen.deactivateMenu();
}

//***CLEAR SPLASH SCREEN FUNCTION***//
// Function   : clearSplashScreen
//
// Description: This function clears the splash screen
//
// Parameters : void
//
// Return     : void
//****************************************//
void clearSplashScreen() {

  // Check if menu is active, only turn off screen if menu is not open
  if (!screen.isMenuActive()) {
    screen.deactivateMenu();
  }
}

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
// Function   : buzzerErrorCue
//
// Description: One short queued error tone for refused interactions (e.g.
//              selecting a mode while the mode lock is on, roadmap item 11)
//              — "that input is locked", not "the device is broken".
//
// Parameters : void
//
// Return     : void
//****************************************//
void buzzerErrorCue() {
  buzzer.playErrorSound();
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
  //if (USB_DEBUG) { Serial.println("USBDEBUG: usbConnectionLoop()"); }
  
  // Check if USB is connected
  if (g_comMode == CONF_COM_MODE_USB) {
    if (g_operatingMode == CONF_OPERATING_MODE_MOUSE) {
      g_usbIsConnected = usbmouse.isConnected();
    }
    else if (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) {
      g_usbIsConnected = gamepad.isConnected();
    }
    else { // Bluetooth mouse mode
      g_usbIsConnected = false;
    }
  }

  /*  // TODO 2025-Feb-21 Integrate USB connection check
  if (g_comMode == CONF_COM_MODE_USB) {
    usbConnectTimerId[0] = usbConnectTimer.setTimeout(g_usbConnectDelay, usbCheckConnection);  // Call usbCheckConnection function when g_usbConnectDelay reached
  }
  */
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
        delay(100);  // Long enough for the host to register the disconnect
        USBDevice.attach();
        g_usbConnectDelay = CONF_USB_HID_INIT_DELAY;  // Resume fast checks while the fresh enumeration settles
        g_usbNotReadySinceMs = millis();              // Fresh budget for the fresh enumeration
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
// Description: This function initializes the operating mode based on the
//              based on stored settings in the flash memory (0 = None , 1 = USB Mouse , 2 = Wireless Mouse, 3 = USB Gamepad)
//
// Parameters : void
//
// Return     : void
//****************************************//
void initOperatingMode() {

  //g_operatingMode = getOperatingMode(false,false);
  g_operatingMode = mem.readInt(CONF_SETTINGS_FILE, "OM"); // retrieve operating mode from memory

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

  initCommunicationMode();  // Retrieve communication mode from memory (None, USB, Bluetooth)
  initOperatingMode();      // Retrieve operating mode from memory (USB Mouse, Bluetooth Mouse, Gamepad)

  // Bus up only now: setup() detached at the top, so no host could have
  // locked in a CDC-only enumeration during boot. CDC has been registered
  // since Serial.begin(); the mode interface registers microseconds below.
  // A host needs at least a connect-detect + bus-reset cycle (ms) after
  // attach before it can fetch a descriptor, so every enumeration now sees
  // the complete composite. Safe mode intentionally comes up CDC-only:
  // serial recovery is the whole point.
  USBDevice.attach();

  switch (g_operatingMode) {
    case CONF_OPERATING_MODE_MOUSE:
      switch (g_comMode) {
        case CONF_COM_MODE_USB:  // USB Mouse
          usbmouse.begin();
          break;
        case CONF_COM_MODE_BLE:  // Bluetooth Mouse
          String btName = String("LS_") + String(g_deviceUID);  // Form Bluetooth name using device UID // TODO This may be limited to 15 characters
          btmouse.begin(btName.c_str());
          break;
      }
      break;
    case CONF_OPERATING_MODE_GAMEPAD:  // USB Gamepad
      gamepad.begin();
      break;
    case CONF_OPERATING_MODE_SAFE: // Safe mode
      Serial.print("USBDEBUG: beginComOpMode: Safe Mode");
      break;
      // default:
  }


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
  // External switch action remaps (#20) join the same stroke — the getters
  // apply any stored mapping onto the live table before the first tick.
  initSwitchActions();
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
//              device). switchRemapApplyCell writes one remapped cell into
//              the live table; the jack index runs 0-2, the window index
//              0-2 (short/long/max), gamepad selects the action column.
//
// Parameters : actionNumber : int : an output action number
//              jack / window : int : cell coordinates
//              gamepad : bool : true = gamepad-mode column
//
// Return     : bool : allowlist membership
//              void : (apply)
//****************************************//
bool switchRemapAllowed(int actionNumber) {
  for (unsigned int i = 0; i < CONF_SWITCH_REMAP_ALLOWED_NUM; i++) {
    if (CONF_SWITCH_REMAP_ALLOWED[i] == actionNumber) return true;
  }
  return false;
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
// Parameters : out : int[9] : filled with [usbMounted, hidReady, hidRetrying,
//              hidTimedOut, centerComplete, reportAttempts, reportSuccesses,
//              nonzeroReports, maxAbsReport]
//
// Return     : void
//****************************************//
void fillUsbHidDiagnostics(int out[9]) {
  out[0] = USBDevice.mounted() ? 1 : 0;
  out[1] = usbmouse.isReady() ? 1 : 0;
  out[2] = usbmouse.usbRetrying ? 1 : 0;
  out[3] = usbmouse.timedOut ? 1 : 0;
  out[4] = g_resetCenterComplete ? 1 : 0;
  out[5] = (int)(usbmouse.reportAttempts % 1000000007);  // Wrap-safe read of a boot-long counter
  out[6] = (int)(usbmouse.reportSuccesses % 1000000007);
  out[7] = (int)(usbmouse.nonzeroReports % 1000000007);
  out[8] = usbmouse.maxAbsReport;
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
  if(g_joystickSensorConnected){
    if (outputAction == CONF_ACTION_SCROLL) {
      pollTimer.enable(CONF_TIMER_SCROLL);
      pollTimer.disable(CONF_TIMER_JOYSTICK);
    } else if (outputAction != CONF_ACTION_SCROLL) {
      pollTimer.enable(CONF_TIMER_JOYSTICK);
      pollTimer.disable(CONF_TIMER_SCROLL);
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

      // Perform output action
      performOutputAction(tempActionIndex);

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
      // LEDs show the window the hold is in — green in the short window,
      // yellow after crossing into the long window, red in the very-long
      // window — instead of the pending action's color. Actions are not
      // one-to-one with inputs (#43), so the window is the honest signal,
      // and the progression previews what a release will do without
      // memorizing any mapping. Re-evaluated every tick, so the LEDs
      // step through the traffic light as the hold crosses each boundary;
      // a gated (restricted) input still shows nothing at all.
      if (tempActionIndex != CONF_ACTION_NOTHING && g_lightMode == CONF_LIGHT_MODE_STATE) {
        unsigned long holdMs = actionState.elapsedTime;
        int stateColor = CONF_INPUT_STATE_LED_MAX_COLOR;
        if (holdMs < effectiveWindowMs(CONF_INPUT_WINDOW_LONG_DEFAULT_MS)) {
          stateColor = (holdMs < effectiveWindowMs(CONF_INPUT_WINDOW_SHORT_DEFAULT_MS))
                           ? CONF_INPUT_STATE_LED_SHORT_COLOR
                           : CONF_INPUT_STATE_LED_LONG_COLOR;
        }
        setLedState(LED_ACTION_ON, stateColor, CONF_INPUT_STATE_LED_NUMBER, 0, 0, led.getLedBrightness());
        performLedAction(ledCurrentState);
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
        if (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD) {
          gamepadButtonReleaseAll();
          Serial.println("Released all");
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
        screen.deactivateMenu();  // Deactivate Menu
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

  if (action == CONF_ACTION_DRAG || action == CONF_ACTION_SCROLL) {
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
  int stepNumber = (int)args;
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
    // Start timer to get 5 reading every 100 ms
    calibrationTimerId[1] = calibrationTimer.setTimer(CONF_JOY_INIT_READING_DELAY, currentReadingStart, CONF_JOY_INIT_READING_NUMBER, performJoystickCenterStep, (int*)stepNumber);
    ++stepNumber;
    // Start exit step
    calibrationTimerId[0] = calibrationTimer.setTimeout(nextStepStart, performJoystickCenter, (int*)stepNumber);
  } else {
    js.evaluateInputCenter();           // Evaluate the center point using values in the buffer
    js.setMinimumRadius();              // Update minimum radius of operation
    centerPoint = js.getInputCenter();  // Get the new center for API output
    printResponseFloatPoint(true, true, true, 0, "IN,1", true, centerPoint);
    calibrationTimer.deleteTimer(0);  // Delete timer
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
  int stepNumber = (int)args;

  // Duration of the max corner reading ( 2 seconds )
  unsigned long readingDuration = CONF_JOY_CALIB_READING_DELAY * CONF_JOY_CALIB_READING_NUMBER;

  // Time until start of current reading
  unsigned long currentReadingStart = CONF_JOY_CALIB_STEP_DELAY + (CONF_JOY_CALIB_STEP_BLINK_DELAY * ((CONF_JOY_CALIB_STEP_BLINK * 2) + 1));

  // Time until start of next step. (1.5 + (3*300) seconds )
  unsigned long nextStepStart = currentReadingStart + readingDuration + CONF_JOY_CALIB_START_DELAY;  // Time until start of next step. ( 2.4 + 2 + 1 seconds )

  if (stepNumber == 0) {                     // STEP 0: Calibration started
    pollTimer.disable(CONF_TIMER_JOYSTICK);  // Temporarily disable joystick data polling timer
    pollTimer.disable(CONF_TIMER_INPUT);
    pollTimer.disable(CONF_TIMER_PRESSURE);
    setLedState(LED_ACTION_BLINK, CONF_JOY_CALIB_START_LED_COLOR, CONF_JOY_CALIB_LED_NUMBER, CONF_JOY_CALIB_STEP_BLINK, CONF_JOY_CALIB_STEP_BLINK_DELAY, led.getLedBrightness());
    performLedAction(ledCurrentState);
    ++stepNumber;
    calibrationTimerId[0] = calibrationTimer.setTimeout(currentReadingStart, performJoystickCalibration, (int*)stepNumber);  // Start next step
  } else if (stepNumber < 5) {                                                                                               // STEP 1-4: Joystick Calibration Corner Points
    screen.fullCalibrationPrompt(stepNumber);
    buzzer.calibCornerTone();
    setLedState(LED_ACTION_BLINK, CONF_JOY_CALIB_STEP_BLINK_COLOR, CONF_JOY_CALIB_LED_NUMBER, CONF_JOY_CALIB_STEP_BLINK, CONF_JOY_CALIB_STEP_BLINK_DELAY, led.getLedBrightness());
    performLedAction(ledCurrentState);  // LED Feedback to show start of performJoystickCalibrationStep
    js.zeroInputMax(stepNumber);        // Clear the existing calibration value

    calibrationTimerId[1] = calibrationTimer.setTimer(CONF_JOY_CALIB_READING_DELAY, currentReadingStart, CONF_JOY_CALIB_READING_NUMBER, performJoystickCalibrationStep, (int*)stepNumber);
    ++stepNumber;
    // Set LED's feedback to show step is already started and get the max reading for the quadrant/step

    calibrationTimerId[0] = calibrationTimer.setTimeout(nextStepStart, performJoystickCalibration, (int*)stepNumber);  // Start next step
  } else if (stepNumber == 5) {                                                                                        // STEP 5 : Joystick center point initialization
    screen.fullCalibrationPrompt(stepNumber);
    buzzer.calibCenterTone();
    setJoystickInitialization(false, false);
    ++stepNumber;
    calibrationTimerId[0] = calibrationTimer.setTimeout(nextStepStart, performJoystickCalibration, (int*)stepNumber);  // Start next step
  }

  else  // STEP 6: Calibration ended
  {
    setLedState(LED_ACTION_BLINK, CONF_JOY_CALIB_START_LED_COLOR, CONF_JOY_CALIB_LED_NUMBER, CONF_JOY_CALIB_STEP_BLINK, CONF_JOY_CALIB_STEP_BLINK_DELAY, led.getLedBrightness());  // Turn off Led's
    performLedAction(ledCurrentState);
    js.setMinimumRadius();  // Update the minimum cursor operating radius
    setLedDefault();
    canOutputAction = true;
    g_resetCenterComplete = true;
    pollTimer.enable(CONF_TIMER_JOYSTICK);     // Re-Enable joystick data polling
    pollTimer.enable(CONF_TIMER_SCROLL);       // Re-enable scroll data polling
    pollTimer.enable(CONF_TIMER_INPUT);
    pollTimer.enable(CONF_TIMER_PRESSURE);
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
  int stepNumber = (int)args;
  String stepKey = "CA" + String(stepNumber);       // Key to write new calibration point to Flash memory
  String stepCommand = "CA," + String(stepNumber);  // Command to output calibration point via serial
  pointFloatType maxPoint;
  int magnetZDirection = js.getMagnetZDirection();

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
    mem.writePoint(CONF_SETTINGS_FILE, stepKey, maxPoint);                                    // Store the point in Flash Memory
    setLedState(LED_ACTION_OFF, LED_CLR_NONE, CONF_JOY_CALIB_LED_NUMBER, 0, 0, led.getLedBrightness());
    performLedAction(ledCurrentState);
    printResponseFloatPoint(true, true, true, 0, stepCommand, true, maxPoint);
    if (g_calibrationError) {
      screen.fullCalibrationPrompt(CONF_JOY_CALIB_ERROR);
      delay(3000);  // TODO 2025-Feb-02 Why the delay?
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
  // the stick mid-sip cannot drag the pointer around. Only an ENABLED sip &
  // puff freezes — with SD set there is no mouthpiece input at all, and a
  // sip must not have any observable effect.
  bool sapInProgress = !g_sapDisabled && g_freezeCursorDuringSap && (ps.getState().mainState != PRESS_SAP_MAIN_STATE_NONE);

  if (g_resetCenterComplete && !sapInProgress) {  // Don't output joystick movement until the center position has been reset
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

  if (g_scrollNumRuns % (CONF_SCROLL_MOVE_MAX - abs(scrollOutput)) == 0){
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
    pollTimer.enable(CONF_TIMER_JOYSTICK);  // Enable joystick data polling
    pollTimer.disable(CONF_TIMER_DEBUG);    // Disable debug data polling
    pollTimer.enable(CONF_TIMER_SCROLL);    // Enable scroll data polling

  } else if (inputDebugMode == CONF_DEBUG_MODE_JOYSTICK) {
    pollTimer.disable(CONF_TIMER_JOYSTICK);  // Disable joystick data polling
    pollTimer.disable(CONF_TIMER_SCROLL);    // Disable scroll data polling
    pollTimer.enable(CONF_TIMER_DEBUG);      // Enable debug data polling
  } else {
    pollTimer.enable(CONF_TIMER_DEBUG);  // Enable debug data polling
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
  Wire.end();
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
void blinkLed(ledStateStruct* args) {
  int blinkOffset = 0;  // number of milliseconds to delay start of blink
  ledTimerId[CONF_TIMER_LED_BLINK] = ledStateTimer.setTimer(args->ledBlinkTime, blinkOffset, ((args->ledBlinkNumber) * 2) + 1, ledBlinkEffect, ledCurrentState);
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
          led.setLedColor(CONF_LED_MICRO, LED_CLR_PURPLE, led.getLedBrightness());  // Set micro LED to purple if it's in BLE MODE
        } else if (g_comMode == CONF_COM_MODE_BLE && btmouse.isConnected()) {    // Set micro LED to blue if it's in BLE MODE
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

