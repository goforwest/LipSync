/* 
* File: LSAPI.h
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
  If not, see <http://www.gnu.org/licenses/>.
*/


//***API FUNCTIONS***// - DO NOT CHANGE
#include "LSApiDeclarations.h"  // Explicit declarations for the API surface (roadmap item 5) — the function table below references functions defined further down the merged tabs
typedef void (*FunctionPointer)(bool, bool, String);      // Type definition for API function pointer

typedef struct {                                  // Type definition for API function list
  String endpoint;                                // Unique two character end point
  String code;                                    // Unique one character command code
  String parameter;                               // Parameter that is passed to function
  FunctionPointer function;                       // API function pointer
} _functionList;

// Declare individual API functions with command, parameter, and corresponding function
_functionList getModelNumberFunction =            {"MN", "0", "0", &getModelNumber};
_functionList getVersionNumberFunction =          {"VN", "0", "0", &getVersionNumber};
_functionList getDeviceIDFunction =               {"ID", "0", "0", &getDeviceID};
_functionList getOperatingModeFunction =          {"OM", "0", "0", &getOperatingMode};
_functionList setOperatingModeFunction =          {"OM", "1", "",  &setOperatingMode};
_functionList getCommunicationModeFunction =      {"CM", "0", "0", &getCommunicationMode};
_functionList setCommunicationModeFunction =      {"CM", "1", "",  &setCommunicationMode};

_functionList getJoystickInitializationFunction = {"IN", "0", "0", &getJoystickInitialization};
_functionList setJoystickInitializationFunction = {"IN", "1", "1", &setJoystickInitialization};
_functionList getJoystickCalibrationFunction =    {"CA", "0", "0", &getJoystickCalibration};
_functionList setJoystickCalibrationFunction =    {"CA", "1", "1", &setJoystickCalibration};

_functionList getJoystickInnerDeadzoneFunction =  {"IZ", "0", "0", &getJoystickInnerDeadzone};
_functionList setJoystickInnerDeadzoneFunction =  {"IZ", "1", "",  &setJoystickInnerDeadzone};
_functionList getJoystickOuterDeadzoneFunction =  {"OZ", "0", "0", &getJoystickOuterDeadzone};
_functionList setJoystickUpperDeadzoneFunction =  {"OZ", "1", "",  &setJoystickOuterDeadzone};
_functionList getJoystickAccelerationFunction =   {"AV", "0", "0", &getJoystickAcceleration};
_functionList setJoystickAccelerationFunction =   {"AV", "1", "",  &setJoystickAcceleration};

_functionList getCursorSpeedFunction =            {"SS", "0", "0", &getCursorSpeed};
_functionList setCursorSpeedFunction =            {"SS", "1", "",  &setCursorSpeed};
_functionList getScrollLevelFunction =            {"SL", "0", "0", &getScrollLevel};
_functionList setScrollLevelFunction =            {"SL", "1", "",  &setScrollLevel};

_functionList getPressureValueFunction =          {"PV", "0", "0", &getPressureValue};
_functionList getBusHealthFunction =              {"BH", "0", "0", &getBusHealth};
_functionList getSipPressureThresholdFunction =   {"ST", "0", "0", &getSipPressureThreshold};
_functionList setSipPressureThresholdFunction =   {"ST", "1", "",  &setSipPressureThreshold};
_functionList getPuffPressureThresholdFunction =  {"PT", "0", "0", &getPuffPressureThreshold};
_functionList setPuffPressureThresholdFunction =  {"PT", "1", "",  &setPuffPressureThreshold};

_functionList getSoundModeFunction =              {"SM", "0", "0", &getSoundMode};
_functionList setSoundModeFunction =              {"SM", "1", "",  &setSoundMode};
_functionList getLightModeFunction =              {"LM", "0", "0", &getLightMode};
_functionList setLightModeFunction =              {"LM", "1", "",  &setLightMode};
_functionList getLightBrightnessLevelFunction =   {"LL", "0", "0", &getLightBrightnessLevel};
_functionList setLightBrightnessLevelFunction =   {"LL", "1", "",  &setLightBrightnessLevel};

_functionList getSapDisabledFunction =             {"SD", "0", "0", &getSapDisabled};
_functionList setSapDisabledFunction =             {"SD", "1", "",  &setSapDisabled};
_functionList getMenuButtonsOnlyFunction =         {"MA", "0", "0", &getMenuButtonsOnly};
_functionList setMenuButtonsOnlyFunction =         {"MA", "1", "",  &setMenuButtonsOnly};
_functionList getModeLockedFunction =              {"LK", "0", "0", &getModeLocked};
_functionList setModeLockedFunction =              {"LK", "1", "",  &setModeLocked};
_functionList getCursorHoldFunction =              {"CF", "0", "0", &getCursorHoldDuringSap};
_functionList setCursorHoldFunction =              {"CF", "1", "",  &setCursorHoldDuringSap};

_functionList getInputWindowShortFunction =        {"WS", "0", "0", &getInputWindowShort};
_functionList setInputWindowShortFunction =        {"WS", "1", "",  &setInputWindowShort};
_functionList getInputWindowLongFunction =         {"WL", "0", "0", &getInputWindowLong};
_functionList setInputWindowLongFunction =         {"WL", "1", "",  &setInputWindowLong};
_functionList getInputWindowMaxFunction =          {"WM", "0", "0", &getInputWindowMax};
_functionList setInputWindowMaxFunction =          {"WM", "1", "",  &setInputWindowMax};

_functionList getSwitchActionSAFunction =          {"SA", "0", "0", &getSwitchActionSA};
_functionList setSwitchActionSAFunction =          {"SA", "1", "",  &setSwitchActionSA};
_functionList getSwitchActionSBFunction =          {"SB", "0", "0", &getSwitchActionSB};
_functionList setSwitchActionSBFunction =          {"SB", "1", "",  &setSwitchActionSB};
_functionList getSwitchActionSCFunction =          {"SC", "0", "0", &getSwitchActionSC};
_functionList setSwitchActionSCFunction =          {"SC", "1", "",  &setSwitchActionSC};
_functionList getSwitchActionGAFunction =          {"GA", "0", "0", &getSwitchActionGA};
_functionList setSwitchActionGAFunction =          {"GA", "1", "",  &setSwitchActionGA};
_functionList getSwitchActionGBFunction =          {"GB", "0", "0", &getSwitchActionGB};
_functionList setSwitchActionGBFunction =          {"GB", "1", "",  &setSwitchActionGB};
_functionList getSwitchActionGCFunction =          {"GC", "0", "0", &getSwitchActionGC};
_functionList setSwitchActionGCFunction =          {"GC", "1", "",  &setSwitchActionGC};

_functionList controlHubMenuFunction =            {"CH", "1", "",  &controlHubMenu};
_functionList getHubMenuStateFunction =           {"CH", "0", "0", &getHubMenuState};

_functionList getDebugModeFunction =              {"DM", "0", "0", &getDebugMode};
_functionList setDebugModeFunction =              {"DM", "1", "",  &setDebugMode};
_functionList getJoystickValueFunction =          {"JV", "0", "0", &getJoystickValue};

_functionList exportSettingsFunction =             {"EX", "0", "0", &exportSettings};

_functionList runTestFunction =                   {"RT", "1", "",  &runTest};
_functionList softResetFunction =                 {"SR", "1", "1", &softReset};
_functionList resetSettingsFunction =               {"RS", "1", "1", &resetSettings};
_functionList factoryResetFunction =               {"FR", "1", "1", &doFactoryReset};
_functionList enterUpdateModeFunction =            {"UM", "1", "1", &enterUpdateMode};

// Declare array of API functions
_functionList apiFunction[] = { //Let compiler determine number of functions
  getModelNumberFunction,
  getVersionNumberFunction,
  getDeviceIDFunction,
  getOperatingModeFunction,
  setOperatingModeFunction,
  getCommunicationModeFunction,
  setCommunicationModeFunction,
  getJoystickInitializationFunction,
  setJoystickInitializationFunction,
  getJoystickCalibrationFunction,
  setJoystickCalibrationFunction,
  getJoystickInnerDeadzoneFunction,
  setJoystickInnerDeadzoneFunction,
  getJoystickOuterDeadzoneFunction,
  setJoystickUpperDeadzoneFunction,
  getCursorSpeedFunction,
  setCursorSpeedFunction,
  getScrollLevelFunction,
  setScrollLevelFunction,
  getJoystickValueFunction,
  getPressureValueFunction,
  getBusHealthFunction,
  getSipPressureThresholdFunction,
  setSipPressureThresholdFunction,
  getPuffPressureThresholdFunction,
  setPuffPressureThresholdFunction,
  getJoystickAccelerationFunction,
  setJoystickAccelerationFunction,
  getSoundModeFunction,
  setSoundModeFunction,
  getLightModeFunction,
  setLightModeFunction,
  getLightBrightnessLevelFunction,
  setLightBrightnessLevelFunction,
  getSapDisabledFunction,
  setSapDisabledFunction,
  getMenuButtonsOnlyFunction,
  setMenuButtonsOnlyFunction,
  getModeLockedFunction,
  setModeLockedFunction,
  getCursorHoldFunction,
  setCursorHoldFunction,
  getInputWindowShortFunction,
  setInputWindowShortFunction,
  getInputWindowLongFunction,
  setInputWindowLongFunction,
  getInputWindowMaxFunction,
  setInputWindowMaxFunction,
  getSwitchActionSAFunction,
  setSwitchActionSAFunction,
  getSwitchActionSBFunction,
  setSwitchActionSBFunction,
  getSwitchActionSCFunction,
  setSwitchActionSCFunction,
  getSwitchActionGAFunction,
  setSwitchActionGAFunction,
  getSwitchActionGBFunction,
  setSwitchActionGBFunction,
  getSwitchActionGCFunction,
  setSwitchActionGCFunction,
  controlHubMenuFunction,
  getHubMenuStateFunction,
  getDebugModeFunction,
  setDebugModeFunction,
  exportSettingsFunction,
  runTestFunction,
  softResetFunction,
  resetSettingsFunction,
  factoryResetFunction,
  enterUpdateModeFunction
};

//***SERIAL SETTINGS FUNCTION TO CHANGE SPEED AND COMMUNICATION MODE USING SOFTWARE***//

// Non-blocking input framing for the settings API. A host may terminate a
// message with a newline — it is framed and processed the instant it
// arrives, with no added latency — or, like the current LipSync Connect,
// send it bare and rely on an idle gap to delimit messages
// (SERIAL_FRAMING_GAP_MS, kept for wire compatibility with the tuned v4.x
// host). serialSettings() never blocks: it drains whatever bytes have
// arrived and returns, so the 20 ms joystick / 50 ms pressure ticks stay
// alive even mid-message. readString()'s timed busy-wait — whose timeout was
// the message delimiter, padding every exchange with a fixed delay (1 s per
// message at the Stream default) and freezing the loop while it spun — is
// gone entirely.
const unsigned long SERIAL_FRAMING_GAP_MS = 50;   // Idle that ends a legacy (bare) message
const size_t SERIAL_RX_MAX_LEN = 32;              // Longest legal command is 11 bytes; past this it is a flood
String serialRxBuf = "";
unsigned long serialRxBufLastByteMs = 0;

// Process one delimited message. Returns the settings-mode flag for the
// caller to carry across loop iterations.
bool processSerialMessage(String commandString, bool settingsFlag) {
  commandString.trim();  // Tolerate CR/LF line endings and stray whitespace

  if (settingsFlag == false && commandString == "SETTINGS") {
    // SETTINGS received
    printResponseInt(true, true, true, 0, commandString, false, 0);
    return true;  // Enter settings mode
  } else if (settingsFlag == true && commandString == "EXIT") {
    // EXIT received
    printResponseInt(true, true, true, 0, commandString, false, 0);
    return false;  // Leave settings mode
  } else if (settingsFlag == true && isValidCommandFormat(commandString)) {
    performCommand(commandString);  // Valid command — run it and leave settings mode
    return false;
  }

  // Unrecognized input in either mode
  printResponseInt(true, true, false, 0, commandString, false, 0);
  return false;
}

// Function   : serialSettings
//
// Description: Drains pending serial bytes, framing messages on newline or
//              idle gap, and processes each through processSerialMessage().
//              Called every loop() pass; must never block.
//
// Parameters :  enabled : bool : The current settings-mode flag
//
// Return     : bool : The settings-mode flag to carry to the next pass
//*********************************************************************//
bool serialSettings(bool enabled) {

  bool settingsFlag = enabled;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    serialRxBufLastByteMs = millis();

    if (c == '\n' || c == '\r') {
      if (serialRxBuf.length() == 0) continue;  // Blank line (e.g. the \n of a \r\n pair)
      String message = serialRxBuf;
      serialRxBuf = "";
      settingsFlag = processSerialMessage(message, settingsFlag);
      Serial.flush();
    } else {
      serialRxBuf += c;
      if (serialRxBuf.length() > SERIAL_RX_MAX_LEN) {
        // A flood with no terminator: discard and answer once (echo truncated)
        String overflow = serialRxBuf.substring(0, SERIAL_RX_MAX_LEN);
        serialRxBuf = "";
        printResponseInt(true, true, false, 0, overflow, false, 0);
        settingsFlag = false;
      }
    }
  }

  // Legacy framing: a bare message ends when the line falls idle
  if (serialRxBuf.length() > 0 && millis() - serialRxBufLastByteMs >= SERIAL_FRAMING_GAP_MS) {
    String message = serialRxBuf;
    serialRxBuf = "";
    settingsFlag = processSerialMessage(message, settingsFlag);
    Serial.flush();
  }

  return settingsFlag;
}

//***PERFORM COMMAND FUNCTION TO CHANGE SETTINGS USING SOFTWARE***//
// Function   : performCommand
//
// Description: This function takes processes an input string from the serial and calls the
//              corresponding API function, or outputs an error.
//
// Parameters :  inputString : String : The input command as a string.
//
// Return     : void
//*********************************//
void performCommand(String inputString) {
  int inputIndex = inputString.indexOf(':');

  // Extract command string from input string
  String inputCommandString = inputString.substring(0, inputIndex);

  int inputCommandIndex = inputCommandString.indexOf(',');

  String inputEndpointString = inputCommandString.substring(0, inputCommandIndex);

  String inputCodeString = inputCommandString.substring(inputCommandIndex + 1);

  // Extract parameter string from input string
  String inputParameterString = inputString.substring(inputIndex + 1);

  // Determine total number of API commands
  int apiTotalNumber = sizeof(apiFunction) / sizeof(apiFunction[0]);

  // Iterate through each API command
  for (int apiIndex = 0; apiIndex < apiTotalNumber; apiIndex++) {

    // Test if input command string matches API command and input parameter string matches API parameter string
    if (inputEndpointString == apiFunction[apiIndex].endpoint &&
        inputCodeString == apiFunction[apiIndex].code) {

      // Matching Command String found
      if (!isValidCommandParameter( inputParameterString )) {
        printResponseInt(true, true, false, 2, inputString, false, 0);
      }
      else if (inputParameterString == apiFunction[apiIndex].parameter || apiFunction[apiIndex].parameter == "") {
        apiFunction[apiIndex].function(true, true, inputParameterString);
      }
      else { // Invalid input parameter

        // Outut error message
        printResponseInt(true, true, false, 3, inputString, false, 0);
      }
      break;
    } else if (apiIndex == (apiTotalNumber - 1)) { // api doesn’t exist

      // Output error message
      printResponseInt(true, true, false, 1, inputString, false, 0);

      //delay(5);
      break;
    }
  } // end iterate through API functions

}

//***VALIDATE INPUT COMMAND FORMAT FUNCTION***//
// Function   : isValidCommandFormat
//
// Description: This function confirms command string has correct format.
//              It returns true if the string has a correct format.
//              It returns false if the string doesn't have a correct format.
//
// Parameters :  inputCommandString : String : The input string
//
// Return     : boolean
//***********************************************//
bool isValidCommandFormat(String inputCommandString) {
  bool isValidFormat = false;
  if ((inputCommandString.length() == (6) || // XX,d:d
       inputCommandString.length() == (7) || // XX,d:dd
       inputCommandString.length() == (8) || // XX,d:ddd
       inputCommandString.length() == (9) || // XX,d:dddd
       inputCommandString.length() == (10) || // XX,d:ddddd
       inputCommandString.length() == (11)) && inputCommandString.charAt(2) == ',' && inputCommandString.charAt(4) == ':') { // XX,d:dddddd
    isValidFormat = true;
  }
  return isValidFormat;
}

//***VALIDATE INPUT COMMAND PARAMETER FUNCTION***//
// Function   : isValidCommandParameter
//
// Description: This function checks if the input string is a valid command parameters.
//              It returns true if the string includes valid parameters.
//              It returns false if the string includes invalid parameters.
//
// Parameters :  inputParamterString : String : The input string
//
// Return     : boolean
//*************************************************//
bool isValidCommandParameter(String inputParameterString) {
  if (isStrNumber(inputParameterString)) {
    return true;
  }
  return false;
}

//***CHECK IF STRING IS A NUMBER FUNCTION***//
// Function   : isStrNumber
//
// Description: This function checks if the input string is a number.
//              It returns true if the string includes all numeric characters.
//              It returns false if the string includes a non numeric character.
//
// Parameters :  str : String : The input string
//
// Return     : boolean
//******************************************//
boolean isStrNumber(String str) {
  boolean isNumber = false;
  for (byte i = 0; i < str.length(); i++) {
    isNumber = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
    if (!isNumber) {
      return false; // Non numeric character detected
    }
  }
  return true; // All numeric characters detected
}

//***CHECK IF CHAR IS A VALID DELIMITER FUNCTION***//
// Function   : isValidDelimiter
//
// Description: This function checks if the input char is a valid delimiter.
//              It returns true if the character is a valid delimiter.
//              It returns false if the character is not a valid delimiter.
//
// Parameters :  inputDelimiter : char : The input char delimiter
//
// Return     : boolean
//******************************************//
bool isValidDelimiter(char inputDelimiter) {
  bool validOutput;

  (inputDelimiter == ',' || inputDelimiter == ':' || inputDelimiter == '-') ? validOutput = true : validOutput = false;

  return validOutput;
}


//***GET MODEL NUMBER FUNCTION***//
// Function   : getModelNumber
//
// Description: This function retrieves the current LipSync firmware model number.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getModelNumber(bool responseEnabled, bool apiEnabled) {
  String commandKey = "MN";
  int tempModelNumber = mem.readInt(CONF_SETTINGS_FILE, commandKey);
  if (tempModelNumber != CONF_LIPSYNC_MODEL) {                          // If the previous firmware was different model then reset the settings
    resetSettings(false, false);

    tempModelNumber = CONF_LIPSYNC_MODEL;                               // And store the model number
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempModelNumber);
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, "MN,0", true, tempModelNumber);

}
//***GET MODEL NUMBER API FUNCTION***//
// Function   : getModelNumber
//
// Description: This function is redefinition of main getModelNumber function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getModelNumber(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getModelNumber(responseEnabled, apiEnabled);
  }
}

//***GET VERSION FUNCTION***//
// Function   : getVersionNumber
//
// Description: This function retrieves the current LipSync firmware version number.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getVersionNumber(bool responseEnabled, bool apiEnabled) {
  String commandKey = "VN";
  
  int tempMajorVersionNumber = mem.readInt(CONF_SETTINGS_FILE, "VN1");
  int tempMinorVersionNumber = mem.readInt(CONF_SETTINGS_FILE, "VN2");
  int tempRevVersionNumber   = mem.readInt(CONF_SETTINGS_FILE, "VN3");
  
  if (tempMajorVersionNumber != CONF_LIPSYNC_VERSION_MAJOR) {                          // If the previous firmware was different version then reset the settings
    // could add factory reset here if version number saved in memory is different
    tempMajorVersionNumber = CONF_LIPSYNC_VERSION_MAJOR;                               // And store the version number
    mem.writeInt(CONF_SETTINGS_FILE, "VN1", tempMajorVersionNumber);
    
    tempMinorVersionNumber = CONF_LIPSYNC_VERSION_MINOR;                               // And store the version number
    mem.writeInt(CONF_SETTINGS_FILE, "VN2", tempMinorVersionNumber);

    tempRevVersionNumber = CONF_LIPSYNC_VERSION_REV;                               // And store the version number
    mem.writeInt(CONF_SETTINGS_FILE, "VN3", tempRevVersionNumber);
  }

  else if (tempMinorVersionNumber != CONF_LIPSYNC_VERSION_MINOR) {                     // If the previous firmware was different version then reset the settings
    // could reset some settings here if version number saved in memory is different
    tempMinorVersionNumber = CONF_LIPSYNC_VERSION_MINOR;                               // And store the version number
    mem.writeInt(CONF_SETTINGS_FILE, "VN2", tempMinorVersionNumber);

    tempRevVersionNumber = CONF_LIPSYNC_VERSION_REV;                                    // And store the version number
    mem.writeInt(CONF_SETTINGS_FILE, "VN3", tempRevVersionNumber);
  }

  else if (tempRevVersionNumber != CONF_LIPSYNC_VERSION_REV) {                          // If the previous firmware was different version then reset the settings
    tempRevVersionNumber = CONF_LIPSYNC_VERSION_REV;                                    // And store the version number
    mem.writeInt(CONF_SETTINGS_FILE, "VN3", tempRevVersionNumber);
  }
  

  String tempLipsyncVersionStr = String(tempMajorVersionNumber) + "." + String(tempMinorVersionNumber) + "." + String(tempRevVersionNumber);
  
  printResponseString(responseEnabled, apiEnabled, true, 0, "VN,0", true, tempLipsyncVersionStr);
}
//***GET VERSION API FUNCTION***//
// Function   : getVersionNumber
//
// Description: This function is redefinition of main getVersionNumber function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getVersionNumber(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getVersionNumber(responseEnabled, apiEnabled);
  }
}

//***GET DEVICE ID FUNCTION***//
// Function   : getDeviceID
//
// Description: This function retrieves the UID from the device
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getDeviceID(bool responseEnabled, bool apiEnabled) {
  String commandKey = "ID";
  String tempDeviceID = mem.readString(CONF_SETTINGS_FILE, commandKey);
  String deviceID = readUID();
 
  if (tempDeviceID != deviceID) {  // If string doesn't match, store to memory
    mem.writeString(CONF_SETTINGS_FILE, commandKey, deviceID);
  }
  g_deviceUID = deviceID; // Update global variable
  
  // Answer the ID now in force (the chip's real UID): on a fresh settings
  // file the stored value is the compiled placeholder, and answering it made
  // the FIRST read after a factory reset (or any board swap) report a value
  // the device had already replaced in memory — the truth only arrived on
  // the second read.
  printResponseString(responseEnabled, apiEnabled, true, 0, "ID,0", true, deviceID);

}
//***GET DEVICE ID API FUNCTION***//
// Function   : getDeviceID
//
// Description: This function is redefinition of main getDeviceID function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getDeviceID(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getDeviceID(responseEnabled, apiEnabled);
  }
}

//***GET OPERATING MODE STATE FUNCTION***//
// Function   : getOperatingMode
//
// Description: This function retrieves the state of operating mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : operatingState : intol : The current state of operating mode.
//*********************************//
int getOperatingMode(bool responseEnabled, bool apiEnabled) {
  String commandKey = "OM";
  int tempOperatingMode;
  tempOperatingMode = mem.readInt(CONF_SETTINGS_FILE, commandKey);

  if ((tempOperatingMode < CONF_OPERATING_MODE_MIN) || (tempOperatingMode > CONF_OPERATING_MODE_MAX)) {
    tempOperatingMode = CONF_OPERATING_MODE_DEFAULT;
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempOperatingMode);
  }

  printResponseInt(responseEnabled, apiEnabled, true, 0, "OM,0", true, tempOperatingMode);

  return tempOperatingMode;
}
//***GET OPERATING MODE STATE API FUNCTION***//
// Function   : getOperatingMode
//
// Description: This function is redefinition of main getOperatingMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getOperatingMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getOperatingMode(responseEnabled, apiEnabled);
  }
}

//***SET OPERATING MODE STATE FUNCTION***//
// Function   : setOperatingMode
//
// Description: This function sets the state of operating mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputOperatingMode : int : The new operating mode state 
//
// Return     : void
//*********************************//
void setOperatingMode(bool responseEnabled, bool apiEnabled, int inputOperatingMode) {
  String commandKey = "OM";

  if ((inputOperatingMode >= CONF_OPERATING_MODE_MIN) && (inputOperatingMode <= CONF_OPERATING_MODE_MAX)) {   
    if ((g_comMode == CONF_COM_MODE_BLE) && (inputOperatingMode == CONF_OPERATING_MODE_GAMEPAD)){
      printResponseInt(responseEnabled, apiEnabled, false, 3, "OM,1", true, inputOperatingMode);    // Return error if user tries to change to Bluetooth mode while in Gamepad mode
    } else if (mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputOperatingMode)) {
      // The settings-size budget guard can refuse the write (LSMemory) — a
      // refused write must never report SUCCESS or change the live mode, or
      // the app's readout and the device would disagree after a reboot.
      printResponseInt(responseEnabled, apiEnabled, true, 0, "OM,1", true, inputOperatingMode);
      g_operatingMode = inputOperatingMode;
      changeOperatingMode(inputOperatingMode);
    } else {
      printResponseInt(responseEnabled, apiEnabled, false, 3, "OM,1", true, inputOperatingMode);
    }
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "OM,1", true, inputOperatingMode);
  }

  // TODO: move softwareReset() here instead of in changeOperatingMode?

}
//***SET OPERATING MODE STATE API FUNCTION***//
// Function   : setOperatingMode
//
// Description: This function is redefinition of main setOperatingMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setOperatingMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setOperatingMode(responseEnabled, apiEnabled, optionalParameter.toInt());
}


//***GET JOYSTICK CURSOR SPEED FUNCTION***//
// Function   : getCursorSpeed
//
// Description: This function retrieves the current joystick speed level.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
int getCursorSpeed(bool responseEnabled, bool apiEnabled) {
  String commandKey = "SS";
  int tempCursorSpeedLevel = CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT;
  if (CONF_API_ENABLED) {
    tempCursorSpeedLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempCursorSpeedLevel < CONF_JOY_CURSOR_SPEED_LEVEL_MIN) || (tempCursorSpeedLevel > CONF_JOY_CURSOR_SPEED_LEVEL_MAX)) {
      tempCursorSpeedLevel = CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempCursorSpeedLevel);
    }
    
  }
  js.setOutputRange(tempCursorSpeedLevel);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "SS,0", true, tempCursorSpeedLevel);

  return tempCursorSpeedLevel;
}
//***GET JOYSTICK SPEED API FUNCTION***//
// Function   : getCursorSpeed
//
// Description: This function is redefinition of main getCursorSpeed function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getCursorSpeed(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getCursorSpeed(responseEnabled, apiEnabled);
  }
}

//***SET JOYSTICK SPEED FUNCTION***//
// Function   : setCursorSpeed
//
// Description: This function sets the current joystick speed level.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputSpeedLevel : bool : The new joystick speed level.
//
// Return     : void
//*********************************//
void setCursorSpeed(bool responseEnabled, bool apiEnabled, int inputSpeedLevel) {
  String commandKey = "SS";
  bool isValidSpeed = true;
  int tempCursorSpeedLevel = inputSpeedLevel;
  
  if ((tempCursorSpeedLevel >= CONF_JOY_CURSOR_SPEED_LEVEL_MIN) && (tempCursorSpeedLevel <= CONF_JOY_CURSOR_SPEED_LEVEL_MAX)) { // Check if inputSpeedCounter is valid
    // Valid inputSpeedLevel — the settings-size budget guard can still refuse
    // the store (LSMemory), which must read as a failed write, not a silent
    // success the next boot forgets.
    isValidSpeed = mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempCursorSpeedLevel);
    if (!isValidSpeed) {
      tempCursorSpeedLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    }
    if (!CONF_API_ENABLED && isValidSpeed) {
      tempCursorSpeedLevel = CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT;
    }
    performLedAction(ledCurrentState);
  }
  else {
    // Invalid inputSpeedLevel
    tempCursorSpeedLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    performLedAction(ledCurrentState);
    isValidSpeed = false;
  }

  js.setOutputRange(tempCursorSpeedLevel);

  int responseCode = 0;
  (isValidSpeed) ? responseCode = 0 : responseCode = 3;
  printResponseInt(responseEnabled, apiEnabled, isValidSpeed, responseCode, "SS,1", true, tempCursorSpeedLevel);
}
//***SET JOYSTICK SPEED API FUNCTION***//
// Function   : setCursorSpeed
//
// Description: This function is redefinition of main setCursorSpeed function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setCursorSpeed(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setCursorSpeed(responseEnabled, apiEnabled, optionalParameter.toInt());
}


//***GET SCROLL LEVEL FUNCTION***//
// Function   : getScrollLevel
//
// Description: This function retrieves the current scroll level.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
int getScrollLevel(bool responseEnabled, bool apiEnabled) {
  String commandKey = "SL";
  int tempScrollLevel = CONF_SCROLL_LEVEL_DEFAULT;
  if (CONF_API_ENABLED) {
    tempScrollLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempScrollLevel < CONF_SCROLL_LEVEL_MIN) || (tempScrollLevel > CONF_SCROLL_LEVEL_MAX)) {
      tempScrollLevel = CONF_SCROLL_LEVEL_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempScrollLevel);
    }
    
  }
  //js.setOutputRange(tempScrollLevel);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "SL,0", true, tempScrollLevel);

  return tempScrollLevel;
}
//***GET JOYSTICK LEVEL FUNCTION***//
// Function   : getScrollLevel
//
// Description: This function is redefinition of main getScrollLevel function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getScrollLevel(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getScrollLevel(responseEnabled, apiEnabled);
  }
}

//***SET SCROLL LEVEL FUNCTION***//
// Function   : setScrollLevel
//
// Description: This function sets the current scroll level.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputScrollLevel : bool : The new scroll speed level.
//
// Return     : void
//*********************************//
void setScrollLevel(bool responseEnabled, bool apiEnabled, int inputScrollLevel) {
  String commandKey = "SL";
  bool isValidLevel = true;
  int tempScrollLevel = inputScrollLevel;
  
  if ((tempScrollLevel >= CONF_SCROLL_LEVEL_MIN) && (tempScrollLevel <= CONF_SCROLL_LEVEL_MAX)) { // Check if inputLevelCounter is valid
    // Valid inputScrollLevel — a refused store (settings budget, LSMemory)
    // must read as a failed write, not a silent success.
    isValidLevel = mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempScrollLevel);
    if (!isValidLevel) {
      tempScrollLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    }
    if (!CONF_API_ENABLED && isValidLevel) {
      tempScrollLevel = CONF_SCROLL_LEVEL_DEFAULT;
    }
    performLedAction(ledCurrentState);
    g_scrollLevel = tempScrollLevel;
  }
  else {
    // Invalid inputScrollLevel
    tempScrollLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    performLedAction(ledCurrentState);
    isValidLevel = false;
  }

  //js.setOutputRange(tempScrollLevel);
  int responseCode = 0;
  (isValidLevel) ? responseCode = 0 : responseCode = 3;
  printResponseInt(responseEnabled, apiEnabled, isValidLevel, responseCode, "SL,1", true, tempScrollLevel);
}
//***SET SCROLL LEVEL API FUNCTION***//
// Function   : setScrollLevel
//
// Description: This function is redefinition of main setScrollLevel function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setScrollLevel(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setScrollLevel(responseEnabled, apiEnabled, optionalParameter.toInt());
}


//***INCREASE JOYSTICK CURSOR SPEED LEVEL FUNCTION***//
// Function   : increaseCursorSpeed
//
// Description: This function increases the joystick cursor speed level by one.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void increaseCursorSpeed(bool responseEnabled, bool apiEnabled) {
  int tempCursorSpeedLevel = js.getOutputRange();

  tempCursorSpeedLevel++;
  if(tempCursorSpeedLevel <= CONF_JOY_CURSOR_SPEED_LEVEL_MAX) {  // Check validity of input
    setLedState(LED_ACTION_BLINK, 
    CONF_JOY_SPEED_INC_LED_COLOR, 
    CONF_JOY_SPEED_INC_LED_NUMBER, 
    CONF_JOY_SPEED_CHANGE_LED_BLINK, 
    CONF_JOY_SPEED_CHANGE_LED_DELAY, 
    led.getLedBrightness());   // Blink once
  } 
  else{
    setLedState(LED_ACTION_BLINK, 
    CONF_JOY_SPEED_LIMIT_LED_COLOR, 
    CONF_JOY_SPEED_INC_LED_NUMBER, 
    CONF_JOY_SPEED_LIMIT_LED_BLINK, 
    CONF_JOY_SPEED_LIMIT_LED_DELAY, 
    led.getLedBrightness());   // Blink 3 times
  }

  setCursorSpeed(responseEnabled, apiEnabled, tempCursorSpeedLevel);
}

//***DECREASE JOYSTICK CURSOR SPEED LEVEL FUNCTION***//
// Function   : decreaseCursorSpeed
//
// Description: This function decreases the joystick cursor speed level by one.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void decreaseCursorSpeed(bool responseEnabled, bool apiEnabled) {
  int tempCursorSpeedLevel = js.getOutputRange();

  tempCursorSpeedLevel--;
  if(tempCursorSpeedLevel >= CONF_JOY_CURSOR_SPEED_LEVEL_MIN) {
    setLedState(LED_ACTION_BLINK, 
    CONF_JOY_SPEED_DEC_LED_COLOR, 
    CONF_JOY_SPEED_DEC_LED_NUMBER, 
    CONF_JOY_SPEED_CHANGE_LED_BLINK, 
    CONF_JOY_SPEED_CHANGE_LED_DELAY, 
    led.getLedBrightness());   // Blink once
  } 
  else{
    setLedState(LED_ACTION_BLINK, 
    CONF_JOY_SPEED_LIMIT_LED_COLOR, 
    CONF_JOY_SPEED_DEC_LED_NUMBER, 
    CONF_JOY_SPEED_LIMIT_LED_BLINK, 
    CONF_JOY_SPEED_LIMIT_LED_DELAY, 
    led.getLedBrightness());   // Blink 3 times
  }
  
    
  setCursorSpeed(responseEnabled, apiEnabled, tempCursorSpeedLevel);
}


//***GET JOYSTICK INITIALIZATION FUNCTION***//
// Function   : getJoystickInitialization
//
// Description: This function retrieves the neutral values from joystick initialization.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getJoystickInitialization(bool responseEnabled, bool apiEnabled) {
  pointFloatType tempCenterPoint = js.getInputCenter();
  js.setMinimumRadius();                                                                        // Update the minimum cursor operating radius 
  printResponseFloatPoint(responseEnabled, apiEnabled, true, 0, "IN,0", true, tempCenterPoint);
}
//***GET JOYSTICK INITIALIZATION API FUNCTION***//
// Function   : getJoystickInitialization
//
// Description: This function is redefinition of main getJoystickInitialization function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getJoystickInitialization(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getJoystickInitialization(responseEnabled, apiEnabled);
  }
}

//***SET JOYSTICK INITIALIZATION FUNCTION***//
// Function   : setJoystickInitialization
//
// Description: This function performs joystick Initialization.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void setJoystickInitialization(bool responseEnabled, bool apiEnabled) {
  // Record who initiated the center (4.5.2, wire hygiene): the completion's
  // IN,1 line is an ack (SUCCESS) only when the serial API asked for it —
  // a device-initiated center (boot auto-center, Hub-menu Center Reset,
  // the calibration's internal step 5) prints MANUAL instead, so a boot
  // straggler can never falsely resolve a host's pending IN,1:1 waiter.
  // 4.5.4: the initiator rides the flow's own timer payload
  // (CONF_CENTER_INIT_API_FLAG) instead of a shared global, so overlapping
  // centers each carry their own initiator through their own chain.
  int stepPayload = apiEnabled ? CONF_CENTER_INIT_API_FLAG : 0;
  canOutputAction = false;
  calibrationTimerId[0] = calibrationTimer.setTimeout(CONF_JOY_INIT_START_DELAY, performJoystickCenter, (int *)stepPayload);
  if (calibrationTimerId[0] < 0) {
    // Registration refused (a loud, counted LSTimer failure): the flow never
    // starts — without this guard the completion that would have restored
    // canOutputAction never fires and the cursor output stays dead. Answer
    // FAIL,3 so the host's IN,1 waiter resolves honestly instead of timing
    // out on a center that will never run.
    canOutputAction = true;
    printResponseInt(responseEnabled, apiEnabled, false, 3, "IN,1", true, 1);
  }
}
//***SET JOYSTICK INITIALIZATION API FUNCTION***//
// Function   : setJoystickInitialization
//
// Description: This function is redefinition of main setJoystickInitialization function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setJoystickInitialization(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    setJoystickInitialization(responseEnabled, apiEnabled);
  }
}

//*** GET JOYSTICK CALIBRATION FUNCTION***//
// Function   : getJoystickCalibration
//
// Description: This function retrieves calibration values from joystick Calibration.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getJoystickCalibration(bool responseEnabled, bool apiEnabled) {
  String commandKey;
  pointFloatType calibrationPointArray[5];
  calibrationPointArray[0] = js.getInputCenter();
  for (int i = 1; i < 5; i++)
  {
    commandKey = "CA" + String(i);
    calibrationPointArray[i] = mem.readPoint(CONF_SETTINGS_FILE, commandKey);
    js.setInputMax(i, calibrationPointArray[i]);
  }
  js.setMinimumRadius();                                                                              // Update the minimum cursor operating radius 
  printResponseFloatPointArray(responseEnabled, apiEnabled, true, 0, "CA,0", true, "", 5, ',', calibrationPointArray);

}
//***GET JOYSTICK CALIBRATION API FUNCTION***//
// Function   : getJoystickCalibration
//
// Description: This function is redefinition of main getJoystickCalibration function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getJoystickCalibration(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getJoystickCalibration(responseEnabled, apiEnabled);
  }
}

//*** SET JOYSTICK CALIBRATION FUNCTION***//
// Function   : setJoystickCalibration
//
// Description: This function starts the joystick Calibration.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void setJoystickCalibration(bool responseEnabled, bool apiEnabled) {
  js.clear();                                                                                           // Clear previous calibration values
  // The initiator rides the chain's payload (CONF_CALIB_API_FLAG): the
  // per-corner lines print SUCCESS only for a host-requested calibration
  // (CA,1:1) and MANUAL for a Hub-menu one — the 4.5.2 initiator contract.
  int stepPayload = apiEnabled ? CONF_CALIB_API_FLAG : 0;
  canOutputAction = false;
  // #36: apiEnabled is true only on the serial-dispatch path — a calibration
  // the app started must not end by parking the Hub's menu on screen. The
  // menu-driven path (fullCalibrationPage) passes false and keeps the
  // return-to-menu behavior the device user expects.
  screen.calibrationEndsByClosingMenu(apiEnabled);
  calibrationTimerId[0] = calibrationTimer.setTimeout(CONF_JOY_CALIB_START_DELAY, performJoystickCalibration, (int *)stepPayload);  // Start the process
  if (calibrationTimerId[0] < 0) {
    // Registration refused (a loud, counted LSTimer failure): the flow never
    // starts, so restore the state a completed calibration would have —
    // otherwise the output stays dead and the menu-close flag stays armed
    // for an unrelated later transition. Answer FAIL,3 so the host's CA,1
    // waiter resolves honestly instead of timing out on a calibration that
    // will never run.
    canOutputAction = true;
    screen.calibrationEndsByClosingMenu(false);
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CA,1", true, 1);
  }
}
//***SET JOYSTICK CALIBRATION API FUNCTION***//
// Function   : setJoystickCalibration
//
// Description: This function is redefinition of main setJoystickCalibration function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setJoystickCalibration(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    setJoystickCalibration(responseEnabled, apiEnabled);
  }
}


//*** GET JOYSTICK DEADZONE FUNCTION***//
// Function   : getJoystickInnerDeadzone
//
// Description: This function retrieves the joystick deadzone.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
float getJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled) {
  String deadzoneCommand = "IZ";
  float tempDeadzone;
  tempDeadzone = mem.readFloat(CONF_SETTINGS_FILE, deadzoneCommand);

  if ((tempDeadzone <= CONF_JOY_DEADZONE_MIN) || (tempDeadzone >= CONF_JOY_DEADZONE_MAX)) {
    tempDeadzone = CONF_JOY_DEADZONE_INNER_DEFAULT;
    mem.writeFloat(CONF_SETTINGS_FILE, deadzoneCommand, tempDeadzone);
  }
  js.setInnerDeadzone(true, tempDeadzone);
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "IZ,0", true, tempDeadzone);
  return tempDeadzone;
}
//***GET JOYSTICK DEADZONE API FUNCTION***//
// Function   : getJoystickInnerDeadzone
//
// Description: This function is redefinition of main getJoystickInnerDeadzone function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getJoystickInnerDeadzone(responseEnabled, apiEnabled);
  }
}

//*** SET JOYSTICK DEADZONE FUNCTION***//
// Function   : setJoystickInnerDeadzone
//
// Description: This function starts the joystick deadzone.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void setJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled, float inputDeadzone) {
  String deadzoneCommand = "IZ";
  if (((inputDeadzone > CONF_JOY_DEADZONE_MIN) && (inputDeadzone <= CONF_JOY_DEADZONE_MAX))
      && mem.writeFloat(CONF_SETTINGS_FILE, deadzoneCommand, inputDeadzone)) {
    // The store and the live value only land together: a refused store
    // (settings budget, LSMemory) falls to the FAIL,3 branch below.
    js.setInnerDeadzone(true, inputDeadzone);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "IZ,1", true, inputDeadzone);
  }
  else {
    printResponseFloat(responseEnabled, apiEnabled, false, 3, "IZ,1", true, inputDeadzone);
  }
}
//***SET JOYSTICK DEADZONE API FUNCTION***//
// Function   : setJoystickInnerDeadzone
//
// Description: This function is redefinition of main setJoystickInnerDeadzone function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setJoystickInnerDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setJoystickInnerDeadzone(responseEnabled, apiEnabled, optionalParameter.toFloat());
}

//*** GET JOYSTICK OUTER DEADZONE FUNCTION***//
// Function   : getJoystickOuterDeadzone
//
// Description: This function retrieves the joystick outer deadzone for max output.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
float getJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled) {
  String outerDeadzoneCommand = "OZ";
  float tempOuterDeadzone;
  tempOuterDeadzone = mem.readFloat(CONF_SETTINGS_FILE, outerDeadzoneCommand);

  if ((tempOuterDeadzone <= CONF_JOY_DEADZONE_MIN) || (tempOuterDeadzone >= CONF_JOY_DEADZONE_MAX)) {
    tempOuterDeadzone = CONF_JOY_DEADZONE_OUTER_DEFAULT;
    mem.writeFloat(CONF_SETTINGS_FILE, outerDeadzoneCommand, tempOuterDeadzone);
  }
  js.setOuterDeadzone(true, tempOuterDeadzone);
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "OZ,0", true, tempOuterDeadzone);
  return tempOuterDeadzone;
}
//***GET JOYSTICK OUTER DEADZONE API FUNCTION***//
// Function   : getJoystickInnerDeadzone
//
// Description: This function is redefinition of main getJoystickOuterDeadzone function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getJoystickOuterDeadzone(responseEnabled, apiEnabled);
  }
}

//*** SET JOYSTICK OUTER DEADZONE FUNCTION***//
// Function   : setJoystickOuterDeadzone
//
// Description: This function starts the joystick deadzone.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputUpperDeadZone : float : The new upper deadzone value.
//
// Return     : void
//*********************************//
void setJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled, float inputUpperDeadZone) {
  String outerDeadzoneCommand = "OZ";
  if (((inputUpperDeadZone > CONF_JOY_DEADZONE_MIN) && (inputUpperDeadZone <= CONF_JOY_DEADZONE_MAX))
      && mem.writeFloat(CONF_SETTINGS_FILE, outerDeadzoneCommand, inputUpperDeadZone)) {
    // See setJoystickInnerDeadzone: a refused store must not report SUCCESS.
    js.setOuterDeadzone(true, inputUpperDeadZone);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "OZ,1", true, inputUpperDeadZone);
  }
  else {
    printResponseFloat(responseEnabled, apiEnabled, false, 3, "OZ,1", true, inputUpperDeadZone);
  }
}
//***SET JOYSTICK OUTER DEADZONE API FUNCTION***//
// Function   : setJoystickOuterDeadzone
//
// Description: This function is redefinition of main setJoystickOuterDeadzone function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setJoystickOuterDeadzone(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setJoystickOuterDeadzone(responseEnabled, apiEnabled, optionalParameter.toFloat());
}

//***GET JOYSTICK VALUE FUNCTION***//
// Function   : getJoystickValue
//
// Description: This function returns raw joystick value.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getJoystickValue(bool responseEnabled, bool apiEnabled) {
  // No synchronous sensor read here: the joystick poll tick keeps the buffers
  // fresh, and an on-demand js.update() used to run a full TLV493D I2C
  // transaction sequence inside the serial command path — on a flaky shared
  // bus that could block for seconds and trip the watchdog (observed live:
  // every JV exchange against a crawling bus silenced then reset the device).
  // The buffers return the last good reading, which is exactly what this
  // endpoint should report.
   
  const int outputArraySize = 6; 
  float tempJoystickArray[outputArraySize];
  
  pointFloatType tempJoystickRaw = js.getXYRaw();                                 // Read the raw values
  pointIntType tempJoystickIn = js.getXYIn();                                     // Read the input
  pointIntType tempJoystickProcessed = js.getXYOut();                             // Read the filtered values  
  
  tempJoystickArray[0] = tempJoystickRaw.x;
  tempJoystickArray[1] = tempJoystickRaw.y;
  tempJoystickArray[2] = (float)tempJoystickIn.x;
  tempJoystickArray[3] = (float)tempJoystickIn.y;
  tempJoystickArray[4] = (float)tempJoystickProcessed.x;
  tempJoystickArray[5] = (float)tempJoystickProcessed.y;
  
  printResponseFloatArray(responseEnabled, apiEnabled, true, 0, "JV,0", true, "", outputArraySize, ',', tempJoystickArray);
  
}

//***GET JOYSTICK VALUE API FUNCTION***//
// Function   : getJoystickValue
//
// Description: This function is redefinition of main getJoystickValue function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getJoystickValue(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getJoystickValue(responseEnabled, apiEnabled);
  }
}

//***GET PRESSURE VALUE FUNCTION***//
// Function   : getPressureValue
//
// Description: This function returns pressure difference.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getPressureValue(bool responseEnabled, bool apiEnabled) {

  ps.update();      // Request new values from pressure class
  
  const int outputArraySize = 3;   
  float tempSapPressure[outputArraySize];
  tempSapPressure[0] = ps.getSapPressureAbs();    // Read the mouthpiece pressure 
  tempSapPressure[1] = ps.getAmbientPressure();   // Read the ambient pressure
  tempSapPressure[2] = ps.getSapPressure();       // Read the diff pressure

  
  printResponseFloatArray(responseEnabled, apiEnabled, true, 0, "PV,0", true, "", outputArraySize, ',', tempSapPressure);
}

//***GET PRESSURE VALUE API FUNCTION***//
// Function   : getPressureValue
//
// Description: This function is redefinition of main getPressureValue function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getPressureValue(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getPressureValue(responseEnabled, apiEnabled);
  }
}

//***GET BUS HEALTH FUNCTION***//
// Function   : getBusHealth
//
// Description: Read-only I2C bus diagnostic (roadmap item 3): everything
//              the 4.2.0 flaky-bus hunt measured with custom timing probes,
//              in one endpoint. Turns "the screen is black, is it the USB?"
//              into a one-line answer for LipSync Connect's health check and
//              for any maker with a serial monitor.
//
//              Response value (comma-separated):
//                recoveries  : total bounded-timeout bus recoveries this boot
//                              (Wire.recoveryCount(); every one is a bus that
//                              wedged hard enough to need the NXP bus-clear)
//                busHealthy  : 1 = no recent recovery, 0 = a recovery happened
//                              within the hold-debounce window (bus is being
//                              held right now)
//                display     : 0 = not detected, 1 = answering,
//                              2 = demoted (probing on exponential backoff)
//                pressure    : 0 = not detected, 1 = healthy,
//                              2 = unhealthy (gated: sip/puff cannot fire)
//                joystick    : 0 = not detected, 1 = healthy,
//                              2 = poll backed off (bus crawls)
//                ageSec      : seconds since the last recovery, -1 when none
//                              happened this boot
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getBusHealth(bool responseEnabled, bool apiEnabled) {
  uint32_t recoveries = Wire.recoveryCount();
  int ageSec = -1;
  if (recoveries > 0) {
    ageSec = (int)((millis() - Wire.lastRecoveryMs()) / 1000UL);
  }

  int displayHealth = 0;
  if (g_displayConnected) displayHealth = screen.isDisplayHealthy() ? 1 : 2;

  int pressureHealth = 0;
  if (g_mouthpiecePressureSensorConnected && g_ambientPressureSensorConnected)
    pressureHealth = ps.isSensorsHealthy() ? 1 : 2;

  int joystickHealth = 0;
  if (g_joystickSensorConnected) joystickHealth = js.isPollBackedOff() ? 2 : 1;

  // HID-layer + joystick-pipeline telemetry (bench 2026-10-16 hunt):
  // [6..14] = usbMounted, hidReady, hidRetrying, hidTimedOut,
  // centerComplete, reportAttempts, reportSuccesses, nonzeroReports,
  // maxAbsReport — read from the ACTIVE mode's HID interface (mouse mode:
  // usbmouse; gamepad mode: the gamepad's own counters, bench 2026-10-18 —
  // the old unconditional mouse read made gamepad-mode BH report hidReady=0
  // and attempts=0 forever on a healthy boot). [15..28] = the pipeline
  // internals (see fillJoystickPipelineDiagnostics). The app's health check
  // reads the first six values and ignores the rest; a serial monitor sees
  // everything.
  int hidDiagnostics[9];
  fillUsbHidDiagnostics(hidDiagnostics);

  // Round 3 (bench 2026-10-16): the input→output pipeline's live internals —
  // the corner-hold JV read raw≈-17,-23 with in/out at exact 0.00, and every
  // stage traces healthy in source, so the running values must speak.
  int pipelineDiagnostics[14];
  fillJoystickPipelineDiagnostics(pipelineDiagnostics);

  const int outputArraySize = 29;
  int tempBusHealth[outputArraySize];
  tempBusHealth[0] = (int)recoveries;
  tempBusHealth[1] = Wire.busHealthy() ? 1 : 0;
  tempBusHealth[2] = displayHealth;
  tempBusHealth[3] = pressureHealth;
  tempBusHealth[4] = joystickHealth;
  tempBusHealth[5] = ageSec;
  for (int diagIndex = 0; diagIndex < 9; diagIndex++) {
    tempBusHealth[6 + diagIndex] = hidDiagnostics[diagIndex];
  }
  for (int diagIndex = 0; diagIndex < 14; diagIndex++) {
    tempBusHealth[15 + diagIndex] = pipelineDiagnostics[diagIndex];
  }

  printResponseIntArray(responseEnabled, apiEnabled, true, 0, "BH,0", true, "", outputArraySize, ',', tempBusHealth);
}

//***GET BUS HEALTH API FUNCTION***//
// Function   : getBusHealth
//
// Description: This function is redefinition of main getBusHealth function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getBusHealth(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getBusHealth(responseEnabled, apiEnabled);
  }
}

//***GET SIP PRESSURE THRESHOLD FUNCTION***//
// Function   : getSipPressureThreshold
//
// Description: This function returns the current sip pressure threshold.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
float getSipPressureThreshold(bool responseEnabled, bool apiEnabled) {
  String commandKey = "ST";
  float tempSipThreshold;
  tempSipThreshold = mem.readFloat(CONF_SETTINGS_FILE, commandKey);

  if ((tempSipThreshold <= 0.0) || (tempSipThreshold >= CONF_PRESS_MAX_THRESHOLD)) {
    tempSipThreshold = CONF_SIP_THRESHOLD;
    mem.writeFloat(CONF_SETTINGS_FILE, commandKey, tempSipThreshold);
  }
  ps.setSipThreshold(tempSipThreshold);
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "ST,0", true, tempSipThreshold);
  return tempSipThreshold;
}

//***GET PRESSURE THRESHOLD API FUNCTION***//
// Function   : getSipPressureThreshold
//
// Description: This function is redefinition of main getSipPressureThreshold function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getSipPressureThreshold(responseEnabled, apiEnabled);
  }
}

//***SET SIP PRESSURE THRESHOLD FUNCTION***//
// Function   : setSipPressureThreshold
//
// Description: This function sets the current Sip pressure threshold.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputSipThreshold : float : The new sip pressure threshold.
//
// Return     : void
//*********************************//
void setSipPressureThreshold(bool responseEnabled, bool apiEnabled, float inputSipThreshold) {
  String commandKey = "ST";

  if ((inputSipThreshold >= CONF_PRESS_MIN_THRESHOLD && inputSipThreshold < CONF_PRESS_MAX_THRESHOLD)
      && mem.writeFloat(CONF_SETTINGS_FILE, commandKey, inputSipThreshold)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 — a
    // SUCCESS here would tell the app a threshold the device did not keep.
    ps.setSipThreshold(inputSipThreshold);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "ST,1", true, inputSipThreshold);
  }
  else {
    printResponseFloat(responseEnabled, apiEnabled, false, 3, "ST,1", true, inputSipThreshold);

  }
}

//***SET SIP PRESSURE THRESHOLD API FUNCTION***//
// Function   : setSipPressureThreshold
//
// Description: This function is redefinition of main setSipPressureThreshold function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSipPressureThreshold(responseEnabled, apiEnabled, optionalParameter.toFloat());
}

//***GET PUFF THRESHOLD FUNCTION***//
// Function   : getPuffPressureThreshold
//
// Description: This function returns the puff pressure threshold.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
float getPuffPressureThreshold(bool responseEnabled, bool apiEnabled) {
  String commandKey = "PT";
  float tempPuffThreshold;
  tempPuffThreshold = mem.readFloat(CONF_SETTINGS_FILE, commandKey);

  if ((tempPuffThreshold <= 0.0) || (tempPuffThreshold >= CONF_PRESS_MAX_THRESHOLD)) {
    tempPuffThreshold = CONF_PUFF_THRESHOLD;
    mem.writeFloat(CONF_SETTINGS_FILE, commandKey, tempPuffThreshold);
  }
  ps.setPuffThreshold(tempPuffThreshold);
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "PT,0", true, tempPuffThreshold);

  return tempPuffThreshold;
}

//***GET PUFF THRESHOLD API FUNCTION***//
// Function   : getPuffPressureThreshold
//
// Description: This function is redefinition of main getPuffPressureThreshold function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getPuffPressureThreshold(responseEnabled, apiEnabled);
  }
}


//***SET PUFF PRESSURE THRESHOLD FUNCTION***//
// Function   : setPuffPressureThreshold
//
// Description: This function sets the current Puff pressure threshold in percentage.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputPuffThreshold : float : The new sip pressure threshold in percentage.
//
// Return     : void
//*********************************//
void setPuffPressureThreshold(bool responseEnabled, bool apiEnabled, float inputPuffThreshold) {
  String commandKey = "PT";

  if ((inputPuffThreshold >= CONF_PRESS_MIN_THRESHOLD && inputPuffThreshold < CONF_PRESS_MAX_THRESHOLD)
      && mem.writeFloat(CONF_SETTINGS_FILE, commandKey, inputPuffThreshold)) {
    // See setSipPressureThreshold: a refused store reports FAIL,3.
    ps.setPuffThreshold(inputPuffThreshold);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "PT,1", true, inputPuffThreshold);
  }
  else {
    printResponseFloat(responseEnabled, apiEnabled, false, 3, "PT,1", true, inputPuffThreshold);

  }
}
//***SET PUFF PRESSURE THRESHOLD API FUNCTION***//
// Function   : setPuffPressureThreshold
//
// Description: This function is redefinition of main setPuffPressureThreshold function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setPuffPressureThreshold(responseEnabled, apiEnabled, optionalParameter.toFloat());
}

//***GET JOYSTICK ACCELERATION FUNCTION***//
// Function   : getJoystickAcceleration
//
// Description: This function retrieves the current joystick speed level.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
int getJoystickAcceleration(bool responseEnabled, bool apiEnabled) {
  String commandKey = "AV";
  int tempJoystickAccelerationLevel = CONF_JOY_ACCELERATION_LEVEL_DEFAULT;
  if (CONF_API_ENABLED) {
    tempJoystickAccelerationLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempJoystickAccelerationLevel < CONF_JOY_ACCELERATION_LEVEL_MIN) || (tempJoystickAccelerationLevel > CONF_JOY_ACCELERATION_LEVEL_MAX)) {
      tempJoystickAccelerationLevel = CONF_JOY_ACCELERATION_LEVEL_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempJoystickAccelerationLevel);
    }
    
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, "AV,0", true, tempJoystickAccelerationLevel);

  return tempJoystickAccelerationLevel;
}
//***GET JOYSTICK ACCELERATION IO API FUNCTION***//
// Function   : getJoystickAcceleration
//
// Description: This function is redefinition of main getJoystickAcceleration function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getJoystickAcceleration(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getJoystickAcceleration(responseEnabled, apiEnabled);
  }
}

//***SET JOYSTICK ACCELERATION FUNCTION***//
// Function   : setJoystickAcceleration
//
// Description: This function sets the current joystick acceleration level.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputAccelerationLevel : bool : The new joystick acceleration level.
//
// Return     : void
//*********************************//
void setJoystickAcceleration(bool responseEnabled, bool apiEnabled, int inputAccelerationLevel) {
  String commandKey = "AV";
  bool isValidAcceleration = true;
  int tempJoystickAccelerationLevel = inputAccelerationLevel;
  
  if ((tempJoystickAccelerationLevel >= CONF_JOY_ACCELERATION_LEVEL_MIN) && (tempJoystickAccelerationLevel <= CONF_JOY_ACCELERATION_LEVEL_MAX)) { // Check if inputAccelerationCounter is valid
    // Valid inputAccelerationLevel — a refused store (settings budget,
    // LSMemory) reads back the stored level and reports FAIL,3.
    isValidAcceleration = mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempJoystickAccelerationLevel);
    if (!isValidAcceleration) {
      tempJoystickAccelerationLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    } else if (!CONF_API_ENABLED) {
      tempJoystickAccelerationLevel = CONF_JOY_ACCELERATION_LEVEL_DEFAULT;
    }
  }
  else {
    // Invalid inputAccelerationLevel
    tempJoystickAccelerationLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    isValidAcceleration = false;
  }


  int responseCode = 0;
  (isValidAcceleration) ? responseCode = 0 : responseCode = 3;
  printResponseInt(responseEnabled, apiEnabled, isValidAcceleration, responseCode, "AV,1", true, tempJoystickAccelerationLevel);

  // Keep the live global in sync with the stored level (roadmap item 9):
  // the joystick output path reads `acceleration` every tick, and the
  // hub-menu Increase/Decrease entries step from it.
  acceleration = tempJoystickAccelerationLevel;
}
//***SET JOYSTICK ACCELERATION API FUNCTION***//
// Function   : setJoystickAcceleration
//
// Description: This function is redefinition of main setJoystickAcceleration function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setJoystickAcceleration(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setJoystickAcceleration(responseEnabled, apiEnabled, optionalParameter.toInt());
}



//***INCREASE/ACCELERATE JOYSTICK MOVEMENT FUNCTION***//
// Function   : increaseJoystickAcceleration
//
// Description: This function increases the joystick acceleration level by one.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void increaseJoystickAcceleration(bool responseEnabled, bool apiEnabled) {
  int tempJoystickAccelerationLevel=acceleration;
  tempJoystickAccelerationLevel++;
  if(tempJoystickAccelerationLevel <= CONF_JOY_ACCELERATION_LEVEL_MAX) {
    setJoystickAcceleration(responseEnabled, apiEnabled, tempJoystickAccelerationLevel);
  } 
  else {
  }

}

//***DECREASE/DECELERATE JOYSTICK MOVEMENT FUNCTION***//
// Function   : decreaseJoystickAcceleration
//
// Description: This function decreases the joystick acceleration level by one.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void decreaseJoystickAcceleration(bool responseEnabled, bool apiEnabled) {
  int tempJoystickAccelerationLevel=acceleration;
  tempJoystickAccelerationLevel--;
  // The old comparison was inverted (<= MIN) — the level could only ever be
  // decreased once it was already at the bottom; anything above it no-op'd.
  if(tempJoystickAccelerationLevel >= CONF_JOY_ACCELERATION_LEVEL_MIN){
    setJoystickAcceleration(responseEnabled, apiEnabled, tempJoystickAccelerationLevel);
  }
  else{
  }

}

//***GET COMMUNICATION MODE FUNCTION***//
// Function   : getCommunicationMode
//
// Description: This function retrieves the state of communication mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : communicationMode : int : The current state of communication method.
//*********************************//
int getCommunicationMode(bool responseEnabled, bool apiEnabled) {
  String commandKey = "CM";
  int tempComMode;
  tempComMode = mem.readInt(CONF_SETTINGS_FILE, commandKey);

  if ((tempComMode < CONF_COM_MODE_MIN) || (tempComMode > CONF_COM_MODE_MAX)) {
    tempComMode = CONF_COM_MODE_DEFAULT;
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempComMode);
  }

  printResponseInt(responseEnabled, apiEnabled, true, 0, "CM,0", true, tempComMode);

  return tempComMode;
}
//***GET COMMUNICATION MODE API FUNCTION***//
// Function   : getCommunicationMode
//
// Description: This function is redefinition of main getCommunicationMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getCommunicationMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getCommunicationMode(responseEnabled, apiEnabled);
  }
}

//***SET COMMUNICATION MODE FUNCTION***//
// Function   : setCommunicationMode
//
// Description: This function sets the state of communication mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputCommunicationMode : int : The new communication mode state ( 0 = None, 1 = USB , 2 = BLE )
//
// Return     : void
//*********************************//
void setCommunicationMode(bool responseEnabled, bool apiEnabled, int inputCommunicationMode) {
  String commandKey = "CM";
  
  if ((inputCommunicationMode >= CONF_COM_MODE_MIN) && (inputCommunicationMode <= CONF_COM_MODE_MAX)) {
    if ((inputCommunicationMode == CONF_COM_MODE_BLE) && (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD)){
      printResponseInt(responseEnabled, apiEnabled, false, 3, "CM,1", true, inputCommunicationMode);    // Return error if user tries to change to Bluetooth mode while in Gamepad mode
    } else if (mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputCommunicationMode)) {
      // Store first, switch second: a refused store (settings budget,
      // LSMemory) must not flip the live mode the next boot would forget.
      g_comMode = inputCommunicationMode;
      setCommunicationModeLed(g_comMode);
      setLedDefault();
      printResponseInt(responseEnabled, apiEnabled, true, 0, "CM,1", true, inputCommunicationMode);

      // The interface hand-off runs ONLY on the success path: a refused or
      // rejected write leaves g_comMode unchanged, and re-beginning the
      // current interface for a command the device just refused can block
      // the serial handler for the full mount wait when the host is away.
      releaseOutputAction();
      switch(g_comMode) {
        case CONF_COM_MODE_USB:       // USB Mouse
          btmouse.end();
          usbmouse.begin();
          break;
        case CONF_COM_MODE_BLE:       // Bluetooth Mouse
          usbmouse.end();
          btmouse.begin();
          break;
      }
    } else {
      printResponseInt(responseEnabled, apiEnabled, false, 3, "CM,1", true, inputCommunicationMode);
    }

  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CM,1", true, inputCommunicationMode);

  }

}

//***SET COMMUNICATION MODE LED FUNCTION***//
// Function   : setCommunicationModeLed
//
// Description: This function sets the color communication mode blink LED action.
//
// Parameters :  inputCommunicationMode : int : The new communication mode state ( 0 = None, 1 = USB , 2 = BLE )
//
// Return     : void
//*********************************//
void setCommunicationModeLed(int inputCommunicationMode) {
    int modeLedColor = LED_CLR_NONE;
    if(inputCommunicationMode==CONF_COM_MODE_USB) {
      modeLedColor = LED_CLR_WHITE;
    } else if (inputCommunicationMode==CONF_COM_MODE_BLE) {
      modeLedColor = LED_CLR_BLUE;
    }
    setLedState(LED_ACTION_BLINK, modeLedColor, CONF_COM_MODE_LED_NUMBER, CONF_COM_MODE_LED_BLINK, CONF_COM_MODE_LED_BLINK_DELAY,led.getLedBrightness());    
    performLedAction(ledCurrentState);   
}
//***SET COMMUNICATION MODE API FUNCTION***//
// Function   : setCommunicationMode
//
// Description: This function is redefinition of main setCommunicationMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setCommunicationMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setCommunicationMode(responseEnabled, apiEnabled, optionalParameter.toInt());
}


//***TOGGLE COMMUNICATION MODE FUNCTION***//
// Function   : toggleCommunicationMode
//
// Description: This function is used to toggle or increment communication mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
void toggleCommunicationMode(bool responseEnabled, bool apiEnabled) {
  if (g_comMode < CONF_COM_MODE_MAX) {
    g_comMode++;
  }
  else {
    g_comMode = CONF_COM_MODE_MIN;
  }
  setCommunicationMode(responseEnabled, apiEnabled, g_comMode);
}

//***GET SOUND MODE STATE FUNCTION***//
// Function   : getSoundMode
//
// Description: This function retrieves the state of sound mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : debugState : intol : The current state of sound mode.
//*********************************//
int getSoundMode(bool responseEnabled, bool apiEnabled) {
  String commandKey = "SM";
  int tempSoundMode;
  tempSoundMode = mem.readInt(CONF_SETTINGS_FILE, commandKey);

  if ((tempSoundMode < CONF_SOUND_MODE_MIN) || (tempSoundMode > CONF_SOUND_MODE_MAX)) {
    tempSoundMode = CONF_SOUND_MODE_DEFAULT;
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempSoundMode);
  }

  printResponseInt(responseEnabled, apiEnabled, true, 0, "SM,0", true, tempSoundMode);

  return tempSoundMode;
}

//***GET SOUND MODE STATE API FUNCTION***//
// Function   : getSoundMode
//
// Description: This function is redefinition of main getSoundMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getSoundMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getSoundMode(responseEnabled, apiEnabled);
  }
}

//***SET SOUND MODE STATE FUNCTION***//
// Function   : setSoundMode
//
// Description: This function sets the state of sound mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputSoundMode : int : The new sound mode state.
//
// Return     : void
//*********************************//
void setSoundMode(bool responseEnabled, bool apiEnabled, int inputSoundMode) {
  String commandKey = "SM";

  if (((inputSoundMode >= CONF_SOUND_MODE_MIN) && (inputSoundMode <= CONF_SOUND_MODE_MAX))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputSoundMode)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 and leaves
    // the live sound mode untouched.
    g_soundMode = inputSoundMode;  // Update global variable TODO - currently unused
    buzzer.setSoundModeLevel(inputSoundMode);  // Update sound mode level within buzzer instance
    printResponseInt(responseEnabled, apiEnabled, true, 0, "SM,1", true, inputSoundMode);

  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "SM,1", true, inputSoundMode);

  }

}
//***SET SOUND MODE STATE API FUNCTION***//
// Function   : setSoundMode
//
// Description: This function is redefinition of main setSoundMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setSoundMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSoundMode(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***GET LIGHT MODE STATE FUNCTION***//
// Function   : getLightMode
//
// Description: This function retrieves the state of light mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : tempLightMode : int : The current state of light mode.
//*********************************//
int getLightMode(bool responseEnabled, bool apiEnabled) {
  String commandKey = "LM";
  int tempLightMode;
  tempLightMode = mem.readInt(CONF_SETTINGS_FILE, commandKey);

  if ((tempLightMode < CONF_LIGHT_MODE_MIN) || (tempLightMode > CONF_LIGHT_MODE_MAX)) {
    tempLightMode = CONF_LIGHT_MODE_DEFAULT;
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempLightMode);
  }

  g_lightMode = tempLightMode;  // Keep the live global honest (the input-state
                                // LED mode reads it; before this it was only
                                // ever set by setLightMode, so a boot with a
                                // stored mode read as 0/None until first write)

  printResponseInt(responseEnabled, apiEnabled, true, 0, "LM,0", true, tempLightMode);

  return tempLightMode;
}

//***GET LIGHT MODE STATE API FUNCTION***//
// Function   : getLightMode
//
// Description: This function is redefinition of main getLightMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getLightMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getLightMode(responseEnabled, apiEnabled);
  }
}

//***SET LIGHT MODE STATE FUNCTION***//
// Function   : setLightMode
//
// Description: This function sets the state of sound mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputLightMode : int : The new light mode state 
//
// Return     : void
//*********************************//
void setLightMode(bool responseEnabled, bool apiEnabled, int inputLightMode) {
  String commandKey = "LM";

  if (((inputLightMode >= CONF_LIGHT_MODE_MIN) && (inputLightMode <= CONF_LIGHT_MODE_MAX))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputLightMode)) {
    // See setSoundMode: a refused store reports FAIL,3.
    g_lightMode = inputLightMode;
    led.setLightModeLevel(inputLightMode);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "LM,1", true, inputLightMode);
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "LM,1", true, inputLightMode);

  }

}

//***SET LIGHT MODE STATE API FUNCTION***//
// Function   : setLightMode
//
// Description: This function is redefinition of main setLightMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setLightMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setLightMode(responseEnabled, apiEnabled, optionalParameter.toInt());
}

// *********************************************************************************

//***GET LIGHT BRIGHTNESS LEVEL FUNCTION***//
// Function   : getLightBrightnessLevel
//
// Description: This function retrieves the level of the light brightness.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : tempLightLevel : int : The current light brightness level (0-10)
//*********************************//
int getLightBrightnessLevel(bool responseEnabled, bool apiEnabled) {
  String commandKey = "LL";
  int tempLightLevel;
  tempLightLevel = mem.readInt(CONF_SETTINGS_FILE, commandKey);

  if ((tempLightLevel < CONF_LED_BRIGHTNESS_LEVEL_MIN) || (tempLightLevel > CONF_LED_BRIGHTNESS_LEVEL_MAX)) {
    tempLightLevel = CONF_LED_BRIGHTNESS_LEVEL_DEFAULT;
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempLightLevel);
  }

  printResponseInt(responseEnabled, apiEnabled, true, 0, "LL,0", true, tempLightLevel);

  return tempLightLevel;
}

//***GET LIGHT BRIGHTNESS LEVEL API FUNCTION***//
// Function   : getLightBrightnessLevel
//
// Description: This function is redefinition of main getLightBrightnessLevel function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getLightBrightnessLevel(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getLightBrightnessLevel(responseEnabled, apiEnabled);
  }
}

//***SET LIGHT BRIGHTNESS LEVEL FUNCTION***//
// Function   : setLightBrightnessLevel
//
// Description: This function sets the level of the light brightness.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputLightLevel : int : The new light brightness level (0-10)
//
// Return     : void
//*********************************//
void setLightBrightnessLevel(bool responseEnabled, bool apiEnabled, int inputLightLevel) {
  String commandKey = "LL";

  if (((inputLightLevel >= CONF_LED_BRIGHTNESS_LEVEL_MIN) && (inputLightLevel <= CONF_LED_BRIGHTNESS_LEVEL_MAX))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputLightLevel)) {
    // See setSoundMode: a refused store reports FAIL,3.
    led.setLedBrightnessLevel(inputLightLevel);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "LL,1", true, inputLightLevel);
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "LL,1", true, inputLightLevel);

  }

  //TODO: Add LED blink
  setLedState(LED_ACTION_BLINK,
            LED_CLR_RED, 
            CONF_LED_ALL, 
            1,                     // number of blinks
            1000,                     // blink time
            led.getLedBrightness());  // brightness
  // Perform led action
  performLedAction(ledCurrentState);

}

//***SET LIGHT BRIGHTNESS LEVEL API FUNCTION***//
// Function   : setLightBrightnessLevel
//
// Description: This function is redefinition of main setLightBrightnessLevel function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of the input light level.
//
// Return     : void
void setLightBrightnessLevel(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setLightBrightnessLevel(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***INPUT RESTRICTION SETTINGS (roadmap item 11)***//
// Four boolean settings, one coherent family (SD/MA/LK/CF), each exposed as
// a get/set pair like every other setting. All four default to 0 — the
// unrestricted legacy behavior — which is also what a settings file written
// by older firmware reads for the missing keys, so an update can never
// silently lock a device down. The getters also refresh the live gate
// globals, so reading is always safe and keeps the gates honest.

//***GET SAP DISABLED FUNCTION***//
// Function   : getSapDisabled
//
// Description: Returns the sip & puff disable state (SD; upstream #21).
//              1 = mouthpiece inputs cannot fire actions; the pressure
//              state machine and its diagnostics keep running.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : sapDisabled : int : 1 when sip & puff are disabled
//*********************************//
int getSapDisabled(bool responseEnabled, bool apiEnabled) {
  String commandKey = "SD";
  int tempSapDisabled = CONF_SAP_DISABLED_DEFAULT;
  if (CONF_API_ENABLED) {
    tempSapDisabled = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempSapDisabled != 0) && (tempSapDisabled != 1)) {
      tempSapDisabled = CONF_SAP_DISABLED_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempSapDisabled);
    }
  }
  g_sapDisabled = (tempSapDisabled == 1);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "SD,0", true, tempSapDisabled);
  return tempSapDisabled;
}
//***GET SAP DISABLED API FUNCTION***//
void getSapDisabled(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getSapDisabled(responseEnabled, apiEnabled);
  }
}

//***SET SAP DISABLED FUNCTION***//
// Function   : setSapDisabled
//
// Description: Sets the sip & puff disable state (SD; upstream #21).
//
// Parameters :  inputSapDisabled : int : 0 = sip & puff enabled (default), 1 = disabled
//
// Return     : void
//*********************************//
void setSapDisabled(bool responseEnabled, bool apiEnabled, int inputSapDisabled) {
  String commandKey = "SD";
  if ((inputSapDisabled == 0 || inputSapDisabled == 1)
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputSapDisabled)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 and leaves
    // the live gate untouched — all four restriction setters share this.
    g_sapDisabled = (inputSapDisabled == 1);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "SD,1", true, inputSapDisabled);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "SD,1", true, inputSapDisabled);
  }
}
//***SET SAP DISABLED API FUNCTION***//
void setSapDisabled(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSapDisabled(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***GET MENU BUTTONS ONLY FUNCTION***//
// Function   : getMenuButtonsOnly
//
// Description: Returns the menu-access restriction state (MA; upstream #77).
//              1 = only the hub buttons may open/close/navigate the menu;
//              sip & puff and external switches lose menu control.
//
// Return     : menuButtonsOnly : int : 1 when the menu is restricted to hub buttons
//*********************************//
int getMenuButtonsOnly(bool responseEnabled, bool apiEnabled) {
  String commandKey = "MA";
  int tempMenuButtonsOnly = CONF_MENU_BUTTONS_ONLY_DEFAULT;
  if (CONF_API_ENABLED) {
    tempMenuButtonsOnly = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempMenuButtonsOnly != 0) && (tempMenuButtonsOnly != 1)) {
      tempMenuButtonsOnly = CONF_MENU_BUTTONS_ONLY_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempMenuButtonsOnly);
    }
  }
  g_menuButtonsOnly = (tempMenuButtonsOnly == 1);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "MA,0", true, tempMenuButtonsOnly);
  return tempMenuButtonsOnly;
}
//***GET MENU BUTTONS ONLY API FUNCTION***//
void getMenuButtonsOnly(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getMenuButtonsOnly(responseEnabled, apiEnabled);
  }
}

//***SET MENU BUTTONS ONLY FUNCTION***//
// Function   : setMenuButtonsOnly
//
// Description: Sets the menu-access restriction state (MA; upstream #77).
//
// Parameters :  inputMenuButtonsOnly : int : 0 = all inputs may control the menu (default), 1 = hub buttons only
//
// Return     : void
//*********************************//
void setMenuButtonsOnly(bool responseEnabled, bool apiEnabled, int inputMenuButtonsOnly) {
  String commandKey = "MA";
  if ((inputMenuButtonsOnly == 0 || inputMenuButtonsOnly == 1)
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputMenuButtonsOnly)) {
    // See setSapDisabled: a refused store reports FAIL,3.
    g_menuButtonsOnly = (inputMenuButtonsOnly == 1);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "MA,1", true, inputMenuButtonsOnly);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "MA,1", true, inputMenuButtonsOnly);
  }
}
//***SET MENU BUTTONS ONLY API FUNCTION***//
void setMenuButtonsOnly(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setMenuButtonsOnly(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***GET MODE LOCKED FUNCTION***//
// Function   : getModeLocked
//
// Description: Returns the operating-mode lock state (LK; upstream #68).
//              1 = input paths cannot change the operating mode; the serial
//              API stays open so a caregiver can still adjust the device.
//
// Return     : modeLocked : int : 1 when the operating mode is locked
//*********************************//
int getModeLocked(bool responseEnabled, bool apiEnabled) {
  String commandKey = "LK";
  int tempModeLocked = CONF_MODE_LOCK_DEFAULT;
  if (CONF_API_ENABLED) {
    tempModeLocked = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempModeLocked != 0) && (tempModeLocked != 1)) {
      tempModeLocked = CONF_MODE_LOCK_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempModeLocked);
    }
  }
  g_modeLocked = (tempModeLocked == 1);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "LK,0", true, tempModeLocked);
  return tempModeLocked;
}
//***GET MODE LOCKED API FUNCTION***//
void getModeLocked(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getModeLocked(responseEnabled, apiEnabled);
  }
}

//***SET MODE LOCKED FUNCTION***//
// Function   : setModeLocked
//
// Description: Sets the operating-mode lock state (LK; upstream #68).
//
// Parameters :  inputModeLocked : int : 0 = inputs may change the mode (default), 1 = locked
//
// Return     : void
//*********************************//
void setModeLocked(bool responseEnabled, bool apiEnabled, int inputModeLocked) {
  String commandKey = "LK";
  if ((inputModeLocked == 0 || inputModeLocked == 1)
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputModeLocked)) {
    // See setSapDisabled: a refused store reports FAIL,3.
    g_modeLocked = (inputModeLocked == 1);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "LK,1", true, inputModeLocked);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "LK,1", true, inputModeLocked);
  }
}
//***SET MODE LOCKED API FUNCTION***//
void setModeLocked(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setModeLocked(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***GET CURSOR HOLD FUNCTION***//
// Function   : getCursorHoldDuringSap
//
// Description: Returns the cursor-hold state (CF; upstream #44).
//              1 = cursor movement freezes while a sip or puff is in
//              progress, so deflecting the stick mid-input cannot drag the
//              pointer around.
//
// Return     : cursorHold : int : 1 when the cursor freezes during sip & puff
//*********************************//
int getCursorHoldDuringSap(bool responseEnabled, bool apiEnabled) {
  String commandKey = "CF";
  int tempCursorHold = CONF_CURSOR_HOLD_DEFAULT;
  if (CONF_API_ENABLED) {
    tempCursorHold = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if ((tempCursorHold != 0) && (tempCursorHold != 1)) {
      tempCursorHold = CONF_CURSOR_HOLD_DEFAULT;
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempCursorHold);
    }
  }
  g_freezeCursorDuringSap = (tempCursorHold == 1);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "CF,0", true, tempCursorHold);
  return tempCursorHold;
}
//***GET CURSOR HOLD API FUNCTION***//
void getCursorHoldDuringSap(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getCursorHoldDuringSap(responseEnabled, apiEnabled);
  }
}

//***SET CURSOR HOLD FUNCTION***//
// Function   : setCursorHoldDuringSap
//
// Description: Sets the cursor-hold state (CF; upstream #44).
//
// Parameters :  inputCursorHold : int : 0 = cursor moves during sip & puff (default), 1 = frozen during input
//
// Return     : void
//*********************************//
void setCursorHoldDuringSap(bool responseEnabled, bool apiEnabled, int inputCursorHold) {
  String commandKey = "CF";
  if ((inputCursorHold == 0 || inputCursorHold == 1)
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputCursorHold)) {
    // See setSapDisabled: a refused store reports FAIL,3.
    g_freezeCursorDuringSap = (inputCursorHold == 1);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "CF,1", true, inputCursorHold);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CF,1", true, inputCursorHold);
  }
}
//***SET CURSOR HOLD API FUNCTION***//
void setCursorHoldDuringSap(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setCursorHoldDuringSap(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***INPUT TIMING WINDOW SETTINGS (roadmap item 15, upstream #62/#9)***//
// Three window boundaries in ms — WS short|long, WL long|very-long, WM max —
// each stored as 0 = the compiled default (which is also what a settings
// file written by older firmware reads for the missing keys). The getters
// return the window IN FORCE (the default when stored 0); the setters accept
// 0 (reset to default) or a value in range that keeps short < long < max.

//***GET INPUT WINDOW SHORT FUNCTION***//
// Function   : getInputWindowShort
//
// Description: Returns the short/long input window boundary in force (WS).
//
// Return     : windowMs : int : the boundary in ms
//*********************************//
int getInputWindowShort(bool responseEnabled, bool apiEnabled) {
  String commandKey = "WS";
  int tempWindow = CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (stored > 0) tempWindow = stored;
    g_inputWindowShortMs = (stored > 0) ? (unsigned long)stored : 0;
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, "WS,0", true, tempWindow);
  return tempWindow;
}
//***GET INPUT WINDOW SHORT API FUNCTION***//
void getInputWindowShort(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getInputWindowShort(responseEnabled, apiEnabled);
  }
}

//***SET INPUT WINDOW SHORT FUNCTION***//
// Function   : setInputWindowShort
//
// Description: Sets the short/long input window boundary (WS). 0 resets to
//              the compiled default. The value must stay below the long
//              window in force.
//
// Parameters :  inputWindowMs : unsigned long : the boundary in ms (0 = default)
//
// Return     : void
//*********************************//
void setInputWindowShort(bool responseEnabled, bool apiEnabled, unsigned long inputWindowMs) {
  String commandKey = "WS";
  unsigned long longEff = (g_inputWindowLongMs > 0) ? g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  unsigned long newEff = (inputWindowMs > 0) ? inputWindowMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
  if (((inputWindowMs == 0) ||
       (inputWindowMs >= CONF_INPUT_WINDOW_SHORT_MIN_MS && inputWindowMs <= CONF_INPUT_WINDOW_SHORT_MAX_MS && newEff < longEff))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, (int)inputWindowMs)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 and leaves
    // the live window untouched — all three window setters share this.
    g_inputWindowShortMs = (inputWindowMs > 0) ? inputWindowMs : 0;
    printResponseInt(responseEnabled, apiEnabled, true, 0, "WS,1", true, (int)newEff);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "WS,1", true, (int)inputWindowMs);
  }
}
//***SET INPUT WINDOW SHORT API FUNCTION***//
void setInputWindowShort(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setInputWindowShort(responseEnabled, apiEnabled, (unsigned long)optionalParameter.toInt());
}

//***GET INPUT WINDOW LONG FUNCTION***//
// Function   : getInputWindowLong
//
// Description: Returns the long/very-long input window boundary in force (W2).
//
// Return     : windowMs : int : the boundary in ms
//*********************************//
int getInputWindowLong(bool responseEnabled, bool apiEnabled) {
  String commandKey = "WL";
  int tempWindow = CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (stored > 0) tempWindow = stored;
    g_inputWindowLongMs = (stored > 0) ? (unsigned long)stored : 0;
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, "WL,0", true, tempWindow);
  return tempWindow;
}
//***GET INPUT WINDOW LONG API FUNCTION***//
void getInputWindowLong(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getInputWindowLong(responseEnabled, apiEnabled);
  }
}

//***SET INPUT WINDOW LONG FUNCTION***//
// Function   : setInputWindowLong
//
// Description: Sets the long/very-long input window boundary (W2). 0 resets
//              to the compiled default. The value must stay between the
//              short window and the max window in force.
//
// Parameters :  inputWindowMs : unsigned long : the boundary in ms (0 = default)
//
// Return     : void
//*********************************//
void setInputWindowLong(bool responseEnabled, bool apiEnabled, unsigned long inputWindowMs) {
  String commandKey = "WL";
  unsigned long shortEff = (g_inputWindowShortMs > 0) ? g_inputWindowShortMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
  unsigned long maxEff = (g_inputWindowMaxMs > 0) ? g_inputWindowMaxMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
  unsigned long newEff = (inputWindowMs > 0) ? inputWindowMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  if (((inputWindowMs == 0) ||
       (inputWindowMs >= CONF_INPUT_WINDOW_LONG_MIN_MS && inputWindowMs <= CONF_INPUT_WINDOW_LONG_MAX_MS && newEff > shortEff && newEff < maxEff))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, (int)inputWindowMs)) {
    // See setInputWindowShort: a refused store reports FAIL,3.
    g_inputWindowLongMs = (inputWindowMs > 0) ? inputWindowMs : 0;
    printResponseInt(responseEnabled, apiEnabled, true, 0, "WL,1", true, (int)newEff);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "WL,1", true, (int)inputWindowMs);
  }
}
//***SET INPUT WINDOW LONG API FUNCTION***//
void setInputWindowLong(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setInputWindowLong(responseEnabled, apiEnabled, (unsigned long)optionalParameter.toInt());
}

//***GET INPUT WINDOW MAX FUNCTION***//
// Function   : getInputWindowMax
//
// Description: Returns the very-long input window boundary in force (W3).
//
// Return     : windowMs : int : the boundary in ms
//*********************************//
int getInputWindowMax(bool responseEnabled, bool apiEnabled) {
  String commandKey = "WM";
  int tempWindow = CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (stored > 0) tempWindow = stored;
    g_inputWindowMaxMs = (stored > 0) ? (unsigned long)stored : 0;
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, "WM,0", true, tempWindow);
  return tempWindow;
}
//***GET INPUT WINDOW MAX API FUNCTION***//
void getInputWindowMax(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getInputWindowMax(responseEnabled, apiEnabled);
  }
}

//***SET INPUT WINDOW MAX FUNCTION***//
// Function   : setInputWindowMax
//
// Description: Sets the very-long input window boundary (W3). 0 resets to
//              the compiled default. The value must stay above the long
//              window in force.
//
// Parameters :  inputWindowMs : unsigned long : the boundary in ms (0 = default)
//
// Return     : void
//*********************************//
void setInputWindowMax(bool responseEnabled, bool apiEnabled, unsigned long inputWindowMs) {
  String commandKey = "WM";
  unsigned long longEff = (g_inputWindowLongMs > 0) ? g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  unsigned long newEff = (inputWindowMs > 0) ? inputWindowMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
  if (((inputWindowMs == 0) ||
       (inputWindowMs >= CONF_INPUT_WINDOW_MAX_MIN_MS && inputWindowMs <= CONF_INPUT_WINDOW_MAX_MAX_MS && newEff > longEff))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, (int)inputWindowMs)) {
    // See setInputWindowShort: a refused store reports FAIL,3.
    g_inputWindowMaxMs = (inputWindowMs > 0) ? inputWindowMs : 0;
    printResponseInt(responseEnabled, apiEnabled, true, 0, "WM,1", true, (int)newEff);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "WM,1", true, (int)inputWindowMs);
  }
}
//***SET INPUT WINDOW MAX API FUNCTION***//
void setInputWindowMax(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setInputWindowMax(responseEnabled, apiEnabled, (unsigned long)optionalParameter.toInt());
}

// *********************************************************************************

// External switch action remapping (roadmap item 11's #20 follow-up, firmware
// 4.5.0): the MOUSE- and GAMEPAD-mode action cells of the three switch jacks
// are settings — SA/SB/SC are jacks 1/2/3 in mouse mode, GA/GB/GC the same
// jacks in gamepad mode. The GET answers "short,long,max" (the action each
// hold window triggers, in force); the SET takes
// CONF_SWITCH_ACTION_PACK_BASE + short*10000 + long*100 + max (the base keeps
// every non-reset value a 6-digit number, the one shape the serial format
// validator accepts at that magnitude), where every cell must be on the
// caregiver-safe allowlist; 0 resets that jack/mode to the compiled factory
// mapping. Menu- and safe-mode actions, the S13 combination row and the time
// windows themselves stay compiled — see LSConfig.h's remap block.

//***GET SWITCH ACTION FUNCTION***//
// Function   : getSwitchAction
//
// Description: Reads one jack/mode's three window actions (short,long,max),
//              applies them onto the live switch action table, and answers
//              them as a comma triple. Stored values below the pack base
//              (0 = a settings file written by older firmware) mean the
//              compiled defaults — and the live cells are re-applied either
//              way, so a read always leaves the table consistent with what
//              it answers.
//
// Parameters : commandKey : String : SA / SB / SC / GA / GB / GC
//
// Return     : String : "short,long,max" as answered
//*********************************//
String getSwitchAction(bool responseEnabled, bool apiEnabled, String commandKey) {
  bool gamepad = (commandKey.charAt(0) == 'G');
  int jack = (int)(commandKey.charAt(1) - 'A');
  int cells[3] = { 0, 0, 0 };
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (stored >= CONF_SWITCH_ACTION_PACK_BASE) {
      int packed = stored - CONF_SWITCH_ACTION_PACK_BASE;
      cells[0] = packed / 10000;
      cells[1] = (packed / 100) % 100;
      cells[2] = packed % 100;
      // Defense-in-depth for the caregiver allowlist (the write path already
      // enforces it): a stored value can only come from a hand-edited or
      // foreign settings file, but the whole point of the allowlist is that
      // one switch press can never erase settings or restart the device —
      // so a stored triple that fails it falls back to the compiled
      // factory mapping instead of loading the forbidden cells.
      if (!switchRemapAllowed(cells[0]) || !switchRemapAllowed(cells[1]) || !switchRemapAllowed(cells[2])) {
        for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
      }
    } else {
      for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
    }
  } else {
    for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
  }
  for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) switchRemapApplyCell(jack, w, gamepad, cells[w]);
  String triple = String(cells[0]) + "," + String(cells[1]) + "," + String(cells[2]);
  printResponseString(responseEnabled, apiEnabled, true, 0, commandKey + ",0", true, triple);
  return triple;
}

//***SET SWITCH ACTION FUNCTION***//
// Function   : setSwitchAction
//
// Description: Writes one jack/mode's three window actions. 0 resets to the
//              compiled factory mapping; otherwise the value must be
//              CONF_SWITCH_ACTION_PACK_BASE + short*10000 + long*100 + max
//              with every cell on the remap allowlist. The stored value is
//              the wire value verbatim (0 = default), the live table takes
//              the cells immediately, and the response echoes what the
//              device actually took.
//
// Parameters : commandKey : String : SA / SB / SC / GA / GB / GC
//              packedWireValue : int : 0 or base + packed triple
//
// Return     : void
//*********************************//
void setSwitchAction(bool responseEnabled, bool apiEnabled, String commandKey, int packedWireValue) {
  bool gamepad = (commandKey.charAt(0) == 'G');
  int jack = (int)(commandKey.charAt(1) - 'A');
  bool accepted = false;
  int cells[3] = { 0, 0, 0 };
  if (packedWireValue == 0) {
    for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
    accepted = true;
  } else if (packedWireValue >= CONF_SWITCH_ACTION_PACK_BASE &&
             packedWireValue <= CONF_SWITCH_ACTION_PACK_BASE + 19 * 10000 + 19 * 100 + 19) {
    int packed = packedWireValue - CONF_SWITCH_ACTION_PACK_BASE;
    cells[0] = packed / 10000;
    cells[1] = (packed / 100) % 100;
    cells[2] = packed % 100;
    accepted = switchRemapAllowed(cells[0]) && switchRemapAllowed(cells[1]) && switchRemapAllowed(cells[2]);
  }
  if (accepted && mem.writeInt(CONF_SETTINGS_FILE, commandKey, packedWireValue)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 and leaves
    // the live table untouched.
    for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) switchRemapApplyCell(jack, w, gamepad, cells[w]);
    printResponseInt(responseEnabled, apiEnabled, true, 0, commandKey + ",1", true, packedWireValue);
  } else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey + ",1", true, packedWireValue);
  }
}

// Per-endpoint API wrappers: the dispatch table calls the (bool, bool, String)
// overload of each of the six endpoints.
void getSwitchActionSA(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) getSwitchAction(responseEnabled, apiEnabled, "SA");
}
void setSwitchActionSA(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSwitchAction(responseEnabled, apiEnabled, "SA", optionalParameter.toInt());
}
void getSwitchActionSB(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) getSwitchAction(responseEnabled, apiEnabled, "SB");
}
void setSwitchActionSB(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSwitchAction(responseEnabled, apiEnabled, "SB", optionalParameter.toInt());
}
void getSwitchActionSC(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) getSwitchAction(responseEnabled, apiEnabled, "SC");
}
void setSwitchActionSC(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSwitchAction(responseEnabled, apiEnabled, "SC", optionalParameter.toInt());
}
void getSwitchActionGA(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) getSwitchAction(responseEnabled, apiEnabled, "GA");
}
void setSwitchActionGA(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSwitchAction(responseEnabled, apiEnabled, "GA", optionalParameter.toInt());
}
void getSwitchActionGB(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) getSwitchAction(responseEnabled, apiEnabled, "GB");
}
void setSwitchActionGB(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSwitchAction(responseEnabled, apiEnabled, "GB", optionalParameter.toInt());
}
void getSwitchActionGC(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) getSwitchAction(responseEnabled, apiEnabled, "GC");
}
void setSwitchActionGC(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setSwitchAction(responseEnabled, apiEnabled, "GC", optionalParameter.toInt());
}

// *********************************************************************************

//***OPEN HUB MENU FUNCTION***//
// Function   : openHubMenu
//
// Description: This function opens the Hub Menu.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputLightMode : int : The new debug mode state ( true = ON , false = OFF )
//
// Return     : void
//*********************************//
void controlHubMenu(bool responseEnabled, bool apiEnabled, int inputMenuControl) {
  String commandKey = "CH";

  if ((inputMenuControl >= CONF_MENU_CONTROL_MIN) && (inputMenuControl <= CONF_MENU_CONTROL_MAX)) {
    switch(inputMenuControl) {
      case CONF_MENU_CONTROL_OPEN: 
        screen.activateMenu();
        break;     
      case CONF_MENU_CONTROL_SELECT:
        screen.selectMenuItem();
        break;
      case CONF_MENU_CONTROL_NEXT:
        screen.nextMenuItem();
        break;     
      case CONF_MENU_CONTROL_CLOSE:
        screen.deactivateMenu();
        break;      
    } // end switch
   
    printResponseInt(responseEnabled, apiEnabled, true, 0, "CH,1", true, inputMenuControl);

  } // end if valid inputMenuControl
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CH,1", true, inputMenuControl);

  }

}
//***CONTROL HUB MENU API FUNCTION***//
// Function   : controlHubMenu
//
// Description: This function is redefinition of main controlHubMenu function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void controlHubMenu(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  controlHubMenu(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***GET HUB MENU STATE FUNCTION***//
// Function   : getHubMenuState
//
// Description: The read half of Hub remote control (firmware 4.5.1+, bench
//              follow-up 2026-10-18): the app's remote sends relative
//              presses (CH,1:1/2/3/4) and models the Hub's menu position
//              client-side — but a refused select (LK locked → the ~2 s
//              "Mode is locked" page, whose frozen input window also drops
//              subsequent presses while still acking SUCCESS) desyncs the
//              model from the real Hub, and every later press lands
//              somewhere the model does not expect. This endpoint returns
//              the live truth — menu active, current page id, current row
//              — so the app re-syncs after each press and surfaces device
//              position instead of assumption.
//
//              The response is three comma-separated ints: active (0/1),
//              pageId (the LSScreen.h page defines: 0 main, 3 mode, 5 more,
//              32 mode-locked, 551 calib confirm, …), row (raw index into
//              the page's text array — the app maps it past the page's
//              header rows).
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void getHubMenuState(bool responseEnabled, bool apiEnabled) {
  int menuState[3];
  screen.fillMenuState(menuState);
  const int outputArraySize = 3;
  printResponseIntArray(responseEnabled, apiEnabled, true, 0, "CH,0", true, "", outputArraySize, ',', menuState);
}

//***GET HUB MENU STATE API FUNCTION***//
// Function   : getHubMenuState
//
// Description: This function is redefinition of main getHubMenuState function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getHubMenuState(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getHubMenuState(responseEnabled, apiEnabled);
  }
}

// *********************************************************************************

//***GET DEBUG MODE STATE FUNCTION***//
// Function   : getDebugMode
//
// Description: This function retrieves the state of debug mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : debugState : intol : The current state of debug mode.
//*********************************//
int getDebugMode(bool responseEnabled, bool apiEnabled) {
  String commandKey = "DM";
  int tempDebugMode;
  tempDebugMode = mem.readInt(CONF_SETTINGS_FILE, commandKey);

  if ((tempDebugMode < CONF_DEBUG_MODE_MIN) || (tempDebugMode > CONF_DEBUG_MODE_MAX)) {
    tempDebugMode = CONF_DEBUG_MODE_DEFAULT;
    mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempDebugMode);
  }

  setDebugState(tempDebugMode);

  printResponseInt(responseEnabled, apiEnabled, true, 0, "DM,0", true, tempDebugMode);

  return tempDebugMode;
}

//***GET DEBUG MODE STATE API FUNCTION***//
// Function   : getDebugMode
//
// Description: This function is redefinition of main getDebugMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void getDebugMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getDebugMode(responseEnabled, apiEnabled);
  }
}

//***SET DEBUG MODE STATE FUNCTION***//
// Function   : setDebugMode
//
// Description: This function sets the state of debug mode.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               inputDebugStateMode : int : The new debug mode state ( true = ON , false = OFF )
//
// Return     : void
//*********************************//
void setDebugMode(bool responseEnabled, bool apiEnabled, int inputDebugMode) {
  String commandKey = "DM";

  if (((inputDebugMode >= CONF_DEBUG_MODE_MIN) && (inputDebugMode <= CONF_DEBUG_MODE_MAX))
      && mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputDebugMode)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 and leaves
    // the live debug state untouched.
    g_debugMode = inputDebugMode;
    setDebugState(inputDebugMode);
    printResponseInt(responseEnabled, apiEnabled, true, 0, "DM,1", true, inputDebugMode);
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "DM,1", true, inputDebugMode);
  }

}

//***SET DEBUG MODE STATE API FUNCTION***//
// Function   : setDebugMode
//
// Description: This function is redefinition of main setDebugMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void setDebugMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setDebugMode(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***EXPORT SETTINGS FUNCTION***//
// Function   : exportSettings
//
// Description: One-command settings dump for non-app contexts (roadmap item
//              13's serial half, upstream #67): a maker or support tech with
//              a serial monitor sends SETTINGS + EX,0:0 and receives every
//              stored setting as the exact response lines the individual
//              endpoint reads produce — the same grammar the team already
//              reads, so the output pastes straight into a support ticket
//              with no new format to learn.
//
//              The dump is framed by SUCCESS,0:EX,0:BEGIN and
//              SUCCESS,0:EX,0:END so a paste has clean edges. Immediately
//              after BEGIN, one SUCCESS,0:EX,0:SIZE:<n> line reports the
//              serialized settings file's EXACT on-flash size in bytes
//              (the settings-size budget's ground truth, bench follow-up
//              2026-10-18: the file sat at ~383 B through 4.4.x with
//              firmware 4.1.0's 512-byte reader as the only consumer that
//              cannot be updated — 4.5.0's six switch mappings ate most of
//              that headroom, so the bench needs to watch the number, not
//              estimate it). The device identifier (ID) is deliberately
//              excluded — the same privacy line LipSync Connect's settings
//              and log exports draw; a support request that genuinely needs
//              the identifier can ask for ID,0:0 explicitly.
//
//              Every replayed getter is a pure read of stored settings (the
//              identical calls the app's connect-time load makes), so this
//              is safe to run at any time — no input, screen or output state
//              changes.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void exportSettings(bool responseEnabled, bool apiEnabled) {
  printResponseString(responseEnabled, apiEnabled, true, 0, "EX,0", true, "BEGIN");

  // The live serialized size (the number the CONF_SETTINGS_FILE_BUDGET
  // guard enforces against; -1 would mean no file, which a booted device
  // never has). Inside the frame, first line after BEGIN.
  long settingsSize = mem.size(CONF_SETTINGS_FILE);
  printResponseString(responseEnabled, apiEnabled, true, 0, "EX,0", true,
                      String("SIZE:") + String(settingsSize) + String("/") + String(CONF_SETTINGS_FILE_BUDGET));

  getModelNumber(responseEnabled, apiEnabled);
  getVersionNumber(responseEnabled, apiEnabled);
  getOperatingMode(responseEnabled, apiEnabled);
  getCommunicationMode(responseEnabled, apiEnabled);
  getCursorSpeed(responseEnabled, apiEnabled);
  getScrollLevel(responseEnabled, apiEnabled);
  getSipPressureThreshold(responseEnabled, apiEnabled);
  getPuffPressureThreshold(responseEnabled, apiEnabled);
  getJoystickAcceleration(responseEnabled, apiEnabled);
  getJoystickInnerDeadzone(responseEnabled, apiEnabled);
  getJoystickOuterDeadzone(responseEnabled, apiEnabled);
  getJoystickInitialization(responseEnabled, apiEnabled);
  getJoystickCalibration(responseEnabled, apiEnabled);
  getSoundMode(responseEnabled, apiEnabled);
  getLightMode(responseEnabled, apiEnabled);
  getLightBrightnessLevel(responseEnabled, apiEnabled);
  getDebugMode(responseEnabled, apiEnabled);
  getSapDisabled(responseEnabled, apiEnabled);
  getMenuButtonsOnly(responseEnabled, apiEnabled);
  getModeLocked(responseEnabled, apiEnabled);
  getCursorHoldDuringSap(responseEnabled, apiEnabled);
  getInputWindowShort(responseEnabled, apiEnabled);
  getInputWindowLong(responseEnabled, apiEnabled);
  getInputWindowMax(responseEnabled, apiEnabled);
  getSwitchAction(responseEnabled, apiEnabled, "SA");
  getSwitchAction(responseEnabled, apiEnabled, "SB");
  getSwitchAction(responseEnabled, apiEnabled, "SC");
  getSwitchAction(responseEnabled, apiEnabled, "GA");
  getSwitchAction(responseEnabled, apiEnabled, "GB");
  getSwitchAction(responseEnabled, apiEnabled, "GC");

  printResponseString(responseEnabled, apiEnabled, true, 0, "EX,0", true, "END");
}
//***EXPORT SETTINGS API FUNCTION***//
// Function   : exportSettings
//
// Description: This function is redefinition of main exportSettings function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void exportSettings(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    exportSettings(responseEnabled, apiEnabled);
  }
}

//***RUN TEST FUNCTION***//
// Function   : runTest
//
// Description: This function activates an internal test.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               intputTest : int : The id of test to run.
//
// Return     : void
//*********************************//
void runTest(bool responseEnabled, bool apiEnabled, int inputTest) {
  String commandKey = "RT";

  if ((inputTest >= CONF_TEST_MODE_MIN) && (inputTest <= CONF_TEST_MODE_MAX)) { 
    printResponseInt(responseEnabled, apiEnabled, true, 0, "RT,1", true, inputTest);
    activateTest(inputTest); // run the test
  }
  else { // error message
    printResponseInt(responseEnabled, apiEnabled, false, 3, "RT,1", true, inputTest);
  }
}

//***RUN TEST API FUNCTION***//
// Function   : runTest
//
// Description: This function is redefinition of main runTest function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void runTest(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  runTest(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***SOFT RESET FUNCTION***//
// Function   : softReset
//
// Description: This function performs a software reset.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//***************************//
void softReset(bool responseEnabled, bool apiEnabled) {
  printResponseInt(responseEnabled, apiEnabled, true, 0, "SR,1", true, 1);
  softwareReset();
}

//***FACTORY RESET API FUNCTION***//
// Function   : softReset
//
// Description: This function is redefinition of main softRest function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void softReset(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    softReset(responseEnabled, apiEnabled);
  }
}

//***RESET SETTINGS FUNCTION***//
// Function   : resetSettings
//
// Description: This function performs reset.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//***************************//
void resetSettings(bool responseEnabled, bool apiEnabled) {
  printResponseInt(responseEnabled, apiEnabled, true, 0, "RS,1", true, 0);
}

//***RESET SETTINGS API FUNCTION***//
// Function   : resetSettings
//
// Description: This function is redefinition of main resetSettings function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void resetSettings(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    resetSettings(responseEnabled, apiEnabled);
  }
}

//***DO FACTORY RESET FUNCTION***//
// Function   : doFactoryReset
//
// Description: This function does a factory reset.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//***************************//
void doFactoryReset(bool responseEnabled, bool apiEnabled) {

  // Set all LEDs to red to indicate factory reset process 
  setLedState(LED_ACTION_ON, LED_CLR_RED, CONF_LED_ALL, 0, 0, CONF_LED_BRIGHTNESS_MAX);                           
  performLedAction(ledCurrentState);  

  // Factory reset process 
  Serial.println("FACTORY RESET TRIGGERED");
  
  resetMemory();  // Format and reinitialize internal file system 

  setCommunicationMode(false, false, CONF_COM_MODE_DEFAULT);
  setOperatingMode(false, false, CONF_OPERATING_MODE_DEFAULT);
  setDebugMode(false, false, CONF_DEBUG_MODE_DEFAULT);
  setJoystickInnerDeadzone(false, false, CONF_JOY_DEADZONE_INNER_DEFAULT);
  setJoystickOuterDeadzone(false, false, CONF_JOY_DEADZONE_OUTER_DEFAULT);
  setSipPressureThreshold(false, false, CONF_SIP_THRESHOLD);
  setPuffPressureThreshold(false, false, CONF_PUFF_THRESHOLD);
  setCursorSpeed(false, false, CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT);  
  setScrollLevel(false, false, CONF_SCROLL_LEVEL_DEFAULT);
  setLightBrightnessLevel(false, false, CONF_LED_BRIGHTNESS_LEVEL_DEFAULT);
  setJoystickAcceleration(false, false, CONF_JOY_ACCELERATION_LEVEL_DEFAULT);
  setSapDisabled(false, false, CONF_SAP_DISABLED_DEFAULT);          // Restrictions reset to the
  setMenuButtonsOnly(false, false, CONF_MENU_BUTTONS_ONLY_DEFAULT);  // unrestricted defaults —
  setModeLocked(false, false, CONF_MODE_LOCK_DEFAULT);              // a factory reset can never
  setCursorHoldDuringSap(false, false, CONF_CURSOR_HOLD_DEFAULT);   // lock a device out
  setInputWindowShort(false, false, 0);   // Timing windows reset to the
  setInputWindowLong(false, false, 0);    // compiled factory feel
  setInputWindowMax(false, false, 0);
  setSwitchAction(false, false, "SA", 0);  // Switch action remaps reset
  setSwitchAction(false, false, "SB", 0);  // to the compiled factory
  setSwitchAction(false, false, "SC", 0);  // mapping — a factory reset can
  setSwitchAction(false, false, "GA", 0);  // never leave a caregiver
  setSwitchAction(false, false, "GB", 0);  // mapping behind
  setSwitchAction(false, false, "GC", 0);
  printResponseInt(responseEnabled, apiEnabled, true, 0, "FR,1", true, 1);

  // Clear all LEDs to indicate factory reset process is finished 
  setLedState(LED_ACTION_OFF, LED_CLR_NONE, CONF_JOY_CALIB_LED_NUMBER, 0, 0, CONF_LED_BRIGHTNESS_MAX);                           
  performLedAction(ledCurrentState);  

  Serial.println("Activating software reset");

  softwareReset();

}

//***FACTORY RESET API FUNCTION***//
// Function   : doFactoryReset
//
// Description: This function is redefinition of main doFactoryReset function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of zero.
//
// Return     : void
void doFactoryReset(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    doFactoryReset(responseEnabled, apiEnabled);
  }
}

//***ENTER UPDATE MODE FUNCTION***//
// Function   : enterUpdateMode
//
// Description: Deliberate update-mode entry over the settings API
//              (firmware-update improvements item 1): ack, draw the
//              "Update mode" splash as the final frame, then reset into
//              the stock serial-DFU bootloader. The stock bootloader never
//              drives the Hub's OLED, so without this the screen freezes
//              on whatever the app firmware drew last (via the SR-first
//              restart: the "RESTARTING" message) and the device looks
//              stuck mid-restart while it is actually fine and waiting
//              for the flash. LipSync Connect (4.3.0+) sends UM,1:1 over
//              the live session in place of the 1200-baud touch; on older
//              firmware it falls back to the touch, and a maker with a
//              serial monitor can send SETTINGS + UM,1:1 instead of the
//              double-reset dance.
//
//              enterSerialDfu() is the core's own 1200-baud-touch
//              destination (GPREGRET serial-DFU magic + reset), so this
//              lands in exactly the bootloader the touch lands in.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//
// Return     : void
//*********************************//
void enterUpdateMode(bool responseEnabled, bool apiEnabled) {
  // Ack BEFORE anything else: the reset below kills USB, and the host must
  // already hold the confirmation when the port departs. The bounded dwell
  // then gives the ack time to reach the host and the splash's framebuffer
  // push time to complete before the OLED loses its driver.
  printResponseInt(responseEnabled, apiEnabled, true, 0, "UM,1", true, 1);

  if (g_displayConnected) {
    screen.updateModePage();
  }

  delay(CONF_UPDATE_MODE_DWELL_MS);

  enterSerialDfu();
}

//***ENTER UPDATE MODE API FUNCTION***//
// Function   : enterUpdateMode
//
// Description: This function is redefinition of main enterUpdateMode function to match the types of API function arguments.
//
// Parameters :  responseEnabled : bool : The response for serial printing is enabled if it's set to true.
//                                        The serial printing is ignored if it's set to false.
//               apiEnabled : bool : The api response is sent if it's set to true.
//                                   Manual response is sent if it's set to false.
//               optionalParameter : String : The input parameter string should contain one element with value of one.
//
// Return     : void
void enterUpdateMode(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    enterUpdateMode(responseEnabled, apiEnabled);
  }
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH STRING PARAMETER FUNCTION***//
// Function   : printResponseString
//
// Description: Serial Print output of the responses from APIs with string parameter as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : String : The response parameters printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseString(bool responseEnabled, 
                         bool apiEnabled,
                         bool responseStatus,
                         int responseNumber,
                         String responseCommand, 
                         bool responseParameterEnabled, 
                         String responseParameter) {
  if (responseEnabled) {

    if (responseStatus) {
      (apiEnabled) ? Serial.print("SUCCESS") : Serial.print("MANUAL");
    } else {
      Serial.print("FAIL");
    }
    Serial.print(",");
    Serial.print(responseNumber);
    Serial.print(":");
    Serial.print(responseCommand);
    if (responseParameterEnabled) {
      Serial.print(":");
      Serial.println(responseParameter);
    } else {
      Serial.println("");
    }
  }
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH INT PARAMETER FUNCTION***//
// Function   : printResponseInt
//
// Description: Serial Print output of the responses from APIs with int parameter as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : int : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseInt(bool responseEnabled, 
                      bool apiEnabled,
                      bool responseStatus, 
                      int responseNumber, 
                      String responseCommand, 
                      bool responseParameterEnabled, 
                      int responseParameter) {
  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, String(responseParameter));
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH INT PARAMETER FUNCTION***//
// Function   : printResponseIntArray
//
// Description: Serial Print output of the responses from APIs with an int array as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : int[] : The array response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseIntArray(bool responseEnabled, 
                           bool apiEnabled, 
                           bool responseStatus, 
                           int responseNumber, 
                           String responseCommand, 
                           bool responseParameterEnabled, 
                           String responsePrefix, 
                           int responseParameterSize, 
                           char responseParameterDelimiter, 
                           int responseParameter[]) {
  char tempParameterDelimiter[1];

  (isValidDelimiter(responseParameterDelimiter)) ? tempParameterDelimiter[0] = {responseParameterDelimiter} : tempParameterDelimiter[0] = {'\0'};

  String responseParameterString = String(responsePrefix);
  for (int parameterIndex = 0; parameterIndex < responseParameterSize; parameterIndex++) {
    responseParameterString.concat(responseParameter[parameterIndex]);
    if (parameterIndex < (responseParameterSize - 1)) {
      responseParameterString.concat(tempParameterDelimiter[0]);
    };
  }

  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, responseParameterString);

}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH POINT PARAMETER FUNCTION***//
// Function   : printResponseIntPoint
//
// Description: Serial Print output of the responses from APIs with an point as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : pointIntType : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseIntPoint(bool responseEnabled,
                           bool apiEnabled, 
                           bool responseStatus, 
                           int responseNumber, 
                           String responseCommand, 
                           bool responseParameterEnabled, 
                           pointIntType responseParameter) {
  String responseParameterString = "";
  responseParameterString.concat(responseParameter.x);
  responseParameterString.concat("|");
  responseParameterString.concat(responseParameter.y);
  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, responseParameterString);

}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH INT POINT ARRAY PARAMETER FUNCTION***//
// Function   : printResponseIntPointArray
//
// Description: Serial Print output of the responses from APIs with an point array as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : pointIntType : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseIntPointArray(bool responseEnabled,
                                bool apiEnabled, 
                                bool responseStatus, 
                                int responseNumber, 
                                String responseCommand, 
                                bool responseParameterEnabled, 
                                String responsePrefix, 
                                int responseParameterSize, 
                                char responseParameterDelimiter, 
                                pointIntType responseParameter[]) {
  char tempParameterDelimiter[1];

  (isValidDelimiter(responseParameterDelimiter)) ? tempParameterDelimiter[0] = {responseParameterDelimiter} : tempParameterDelimiter[0] = {'\0'};

  String responseParameterString = String(responsePrefix);
  for (int parameterIndex = 0; parameterIndex < responseParameterSize; parameterIndex++) {
    responseParameterString.concat(responseParameter[parameterIndex].x);
    responseParameterString.concat("|");
    responseParameterString.concat(responseParameter[parameterIndex].y);
    if (parameterIndex < (responseParameterSize - 1)) {
      responseParameterString.concat(tempParameterDelimiter[0]);
    };
  }

  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, responseParameterString);
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH FLOAT PARAMETER FUNCTION***//
// Function   : printResponseFloat
//
// Description: Serial Print output of the responses from APIs with float parameter as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : float : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseFloat(bool responseEnabled,
                        bool apiEnabled,
                        bool responseStatus,
                        int responseNumber,
                        String responseCommand,
                        bool responseParameterEnabled,
                        float responseParameter) {
  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, String(responseParameter));
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH FLOAT ARRAY PARAMETER FUNCTION***//
// Function   : printResponseFloat
//
// Description: Serial Print output of the responses from APIs with float parameter as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responsePrefix : String : 
//               responseParameterSize : int : Number of responses in array
//               responseParameterDelimiter : char : Delimter character to separate responses
//               responseParameter : float[] : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseFloatArray(bool responseEnabled, 
                             bool apiEnabled,
                             bool responseStatus,
                             int responseNumber,
                             String responseCommand,
                             bool responseParameterEnabled,
                             String responsePrefix,
                             int responseParameterSize,
                             char responseParameterDelimiter,
                             float responseParameter[]) {
  char tempParameterDelimiter[1];

  (isValidDelimiter(responseParameterDelimiter)) ? tempParameterDelimiter[0] = {responseParameterDelimiter} : tempParameterDelimiter[0] = {'\0'};

  String responseParameterString = String(responsePrefix);
  for (int parameterIndex = 0; parameterIndex < responseParameterSize; parameterIndex++) {
    responseParameterString.concat(responseParameter[parameterIndex]);
    if (parameterIndex < (responseParameterSize - 1)) {
      responseParameterString.concat(tempParameterDelimiter[0]);
    };
  }

  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, responseParameterString);
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH FLOAT POINT PARAMETER FUNCTION***//
// Function   : printResponseFloatPoint
//
// Description: Serial Print output of the responses from APIs with float point as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : pointFloatType : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseFloatPoint(bool responseEnabled, 
                             bool apiEnabled, 
                             bool responseStatus, 
                             int responseNumber, 
                             String responseCommand, 
                             bool responseParameterEnabled, 
                             pointFloatType responseParameter) {
  String responseParameterString = "";
  responseParameterString.concat(responseParameter.x);
  responseParameterString.concat("|");
  responseParameterString.concat(responseParameter.y);

  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, responseParameterString);
}

//***SERIAL PRINT OUT COMMAND RESPONSE WITH FLOAT POINT ARRAY PARAMETER FUNCTION***//
// Function   : printResponseFloatPointArray
//
// Description: Serial Print output of the responses from APIs with float point as the output
//
// Parameters :  responseEnabled : bool : Print the response if it's set to true, and skip the response if it's set to false.
//               apiEnabled : bool : Print the response and indicate if the the function was called via the API if it's set to true.
//                                   Print Manual response if the function wasn't called via API.
//               responseStatus : bool : The response status (SUCCESS,FAIL)
//               responseNumber : int : 0,1,2 (Different meanings depending on the responseStatus)
//               responseCommand : String : The End-Point command which is returned as output.
//               responseParameterEnabled : bool : Print the parameter if it's set to true, and skip the parameter if it's set to false.
//               responseParameter : pointFloatType[] : The response parameter printed as output.
//
// Return     : void
//***********************************************************************//
void printResponseFloatPointArray(bool responseEnabled, 
                                  bool apiEnabled,
                                  bool responseStatus,
                                  int responseNumber,
                                  String responseCommand,
                                  bool responseParameterEnabled,
                                  String responsePrefix,
                                  int responseParameterSize,
                                  char responseParameterDelimiter,
                                  pointFloatType responseParameter[]) {
  char tempParameterDelimiter[1];

  (isValidDelimiter(responseParameterDelimiter)) ? tempParameterDelimiter[0] = {responseParameterDelimiter} : tempParameterDelimiter[0] = {'\0'};

  String responseParameterString = String(responsePrefix);
  for (int parameterIndex = 0; parameterIndex < responseParameterSize; parameterIndex++) {
    responseParameterString.concat(responseParameter[parameterIndex].x);
    responseParameterString.concat("|");
    responseParameterString.concat(responseParameter[parameterIndex].y);
    if (parameterIndex < (responseParameterSize - 1)) {
      responseParameterString.concat(tempParameterDelimiter[0]);
    };
  }

  printResponseString(responseEnabled, apiEnabled, responseStatus, responseNumber, responseCommand, responseParameterEnabled, responseParameterString);
}