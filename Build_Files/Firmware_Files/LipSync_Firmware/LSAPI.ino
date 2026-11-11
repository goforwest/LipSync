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
#include "LSSerialTxQueue.h"
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
_functionList setModePresetFunction =             {"MP", "1", "",  &setModePreset};

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
// Upstream #58: hard sip/puff band (four keys living in /settings2.dat).
_functionList getHardSipPressureThresholdFunction =   {"HS", "0", "0", &getHardSipPressureThreshold};
_functionList setHardSipPressureThresholdFunction =   {"HS", "1", "",  &setHardSipPressureThreshold};
_functionList getHardPuffPressureThresholdFunction =  {"HP", "0", "0", &getHardPuffPressureThreshold};
_functionList setHardPuffPressureThresholdFunction =  {"HP", "1", "",  &setHardPuffPressureThreshold};
_functionList getHardSipActionFunction =              {"HA", "0", "0", &getHardSipAction};
_functionList setHardSipActionFunction =              {"HA", "1", "",  &setHardSipAction};
_functionList getHardPuffActionFunction =             {"HB", "0", "0", &getHardPuffAction};
_functionList setHardPuffActionFunction =             {"HB", "1", "",  &setHardPuffAction};

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

// Upstream #16: multi-host Bluetooth switching (firmware 4.8.0+). BT,0 reads
// the host table (0 = count, 1..n = row), BT,1 switches, BT,2 forgets one
// bond, BT,3:1 ensures advertising for pair-new. The bond store is the
// vendored lib's /adafruit/bond_prph files — no settings keys involved.
_functionList getBluetoothHostFunction =          {"BT", "0", "",  &getBluetoothHost};
_functionList switchBluetoothHostFunction =       {"BT", "1", "",  &switchBluetoothHost};
_functionList forgetBluetoothHostFunction =       {"BT", "2", "",  &forgetBluetoothHost};
_functionList pairNewBluetoothHostFunction =      {"BT", "3", "1", &pairNewBluetoothHost};

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
  setModePresetFunction,
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
  getHardSipPressureThresholdFunction,
  setHardSipPressureThresholdFunction,
  getHardPuffPressureThresholdFunction,
  setHardPuffPressureThresholdFunction,
  getHardSipActionFunction,
  setHardSipActionFunction,
  getHardPuffActionFunction,
  setHardPuffActionFunction,
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
  getBluetoothHostFunction,
  switchBluetoothHostFunction,
  forgetBluetoothHostFunction,
  pairNewBluetoothHostFunction,
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

// Non-blocking response queue. Adafruit_USBD_CDC::write() spins/yields while
// DTR is asserted and its TX FIFO is full; a host that opens the port but
// stops reading could therefore freeze the assistive-device loop indefinitely.
// Canonical API/MANUAL/DEBUG lines enqueue here and loop() drains at most the
// FIFO space currently available. On overflow, stale whole lines are
// discarded so the newest response wins; a line whose prefix already reached
// USB is protected through its newline (LSSerialTxQueue).
const size_t SERIAL_TX_BUFFER_SIZE = 4096;
LSSerialTxQueue<SERIAL_TX_BUFFER_SIZE> serialTxQueue;

void serialTxService() {
  if (serialTxQueue.size() == 0 || !Serial) return;
  int available = Serial.availableForWrite();
  if (available <= 0) return;
  size_t toWrite = serialTxQueue.nextChunkSize((size_t)available);
  size_t written = Serial.write((const uint8_t*)serialTxQueue.readPointer(), toWrite);
  serialTxQueue.consume(written);
}

bool serialTxEnqueueLine(const char* line, size_t length) {
  // Preserve the stock CDC behavior: output generated before DTR opens is
  // discarded, not replayed as stale unsolicited traffic on the next session.
  if (!Serial) return false;
  if (!serialTxQueue.enqueueLine(line, length)) return false;
  serialTxService();
  return true;
}

bool serialTxEnqueueLine(const String &line) {
  return serialTxEnqueueLine(line.c_str(), line.length());
}

// Process one delimited message. Returns the settings-mode flag for the
// caller to carry across loop iterations.
bool processSerialMessage(String commandString, bool settingsFlag) {
  commandString.trim();  // Tolerate CR/LF line endings and stray whitespace
  // A newly received request takes priority over stale queued DEBUG/MANUAL
  // output. The host protocol is request/response sequential, so no valid
  // command response is pending when the next request arrives.
  serialTxQueue.prioritizeRequest();

  if (settingsFlag == false && commandString == "SETTINGS") {
    // SETTINGS received
    bootTrailStamp(7);  // witness v7: an actual serial message is being dispatched (stamped HERE, not on every idle loop pass, so pt=7 names a real dispatch site — 4.5.7 review)
    printResponseInt(true, true, true, 0, commandString, false, 0);
    return true;  // Enter settings mode
  } else if (settingsFlag == true && commandString == "EXIT") {
    bootTrailStamp(7);  // witness v7: an actual serial message is being dispatched
    // EXIT received
    printResponseInt(true, true, true, 0, commandString, false, 0);
    return false;  // Leave settings mode
  } else if (settingsFlag == true && isValidCommandFormat(commandString)) {
    bootTrailStamp(7);  // witness v7: an actual serial message is being dispatched
    performCommand(commandString);  // Valid command — run it and leave settings mode
    return false;
  }

  // Unrecognized input in either mode
  bootTrailStamp(7);  // witness v7: an actual serial message is being dispatched (unrecognized shape, but a dispatch)
  if (settingsFlag == true) {
    // 4.8.7 review: in-settings-mode garbage used to answer the SUCCESS
    // errno (FAIL,0) — the same numeral the not-in-settings-mode signal uses
    // — so the refusal class depended on message length (an 11-char bad
    // value got FAIL,3 from the setter's range check; a 12-char one got
    // FAIL,0 here). Answer the unknown-command errno instead. The mode still
    // exits (the deliberate recovery — a garbage flood ends the mode, and
    // the app's per-command SETTINGS handshake re-enters cleanly).
    printResponseInt(true, true, false, 1, commandString, false, 0);
  } else {
    printResponseInt(true, true, false, 0, commandString, false, 0);
  }
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
        // A flood with no terminator: discard and answer once (echo truncated).
        // 4.8.7 review: the unknown-command errno, not the SUCCESS errno —
        // an over-length line is a malformed command, not the "settings mode
        // not enabled" signal.
        String overflow = serialRxBuf.substring(0, SERIAL_RX_MAX_LEN);
        serialRxBuf = "";
        printResponseInt(true, true, false, 1, overflow, false, 0);
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
  if (inputIndex < 0) {
    printResponseInt(true, true, false, 1, inputString, false, 0);
    return;
  }

  // Extract command string from input string
  String inputCommandString = inputString.substring(0, inputIndex);

  int inputCommandIndex = inputCommandString.indexOf(',');
  if (inputCommandIndex < 0) {
    printResponseInt(true, true, false, 1, inputString, false, 0);
    return;
  }

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
      // Only the six analog settings accept a decimal point. Every other
      // endpoint is integer-valued; accepting "1.9" and then calling toInt()
      // silently executed it as 1 instead of rejecting malformed input.
      bool floatParameter = (inputCodeString == "1") &&
                            (inputEndpointString == "IZ" || inputEndpointString == "OZ" ||
                             inputEndpointString == "ST" || inputEndpointString == "PT" ||
                             inputEndpointString == "HS" || inputEndpointString == "HP");
      if (!isValidCommandParameter(inputParameterString) ||
          (!floatParameter && inputParameterString.indexOf('.') >= 0)) {
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
//              (4.5.7 review) A real numeric grammar, not a per-character
//              test: the old form accepted '+'/'-'/'.' ANYWHERE, so
//              "1-2-3" and "1.2.3" passed and relied on downstream range
//              checks to catch the toInt() truncations. Optional single
//              leading sign, digits, at most one decimal point.
//
// Parameters :  str : String : The input string
//
// Return     : boolean
//******************************************//
boolean isStrNumber(String str) {
  if (str.length() == 0) return false;
  unsigned int startIndex = 0;
  boolean hasDecimal = false;
  boolean hasDigit = false;
  if (str.charAt(0) == '-' || str.charAt(0) == '+') {
    startIndex = 1;
    if (str.length() == 1) return false;  // A bare sign is not a number
  }
  for (unsigned int i = startIndex; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isDigit(c)) {
      hasDigit = true;
      continue;
    }
    if (c == '.' && !hasDecimal) {
      hasDecimal = true;
      continue;
    }
    return false;  // Non numeric character detected
  }
  return hasDigit;  // A decimal point (with or without a sign) is not a number
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
    // MN=0 is the compiled fresh-file placeholder, not another model. Only
    // reset a successfully read, genuinely different model; a transient
    // failed read also converts to 0 and must never erase valid settings.
    if (mem.isLastReadOk() && tempModelNumber != 0) {
      resetSettings(false, false);
    }

    tempModelNumber = CONF_LIPSYNC_MODEL;                               // And store the model number
    if (mem.isLastReadOk()) {
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempModelNumber);
    }
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

  // (4.9.0 review) These heal-writes are deliberately NOT wrapped in an
  // extra isLastReadOk gate of their own — the one exemption to pin, with
  // its argument: the only legal stored values for VN1/2/3 ARE the compiled
  // constants, so even on a transient failed read (which yields 0) the
  // local assignments above restore the true constants and VN,0 answers
  // correctly; the writer family's own read-truth refusal skips the file
  // write on the failed read (harmless — the next good read heals it), and
  // a truly corrupt file is initialize()'s re-seed at the next boot.
  // Gating the LOCAL assignments instead would answer VN,0:0 on a transient
  // read failure and desync the app's version detection for nothing. Any
  // future key added to this path must re-derive this argument.
  
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
  // (4.9.0 review) Deliberately NOT wrapped in its own isLastReadOk gate —
  // the same pinned exemption as getVersionNumber: the answer is the chip's
  // live UID (always the truth), and the write family's read-truth refusal
  // skips the file write on a failed read (harmless — the next good read
  // heals the file).
  if (deviceID.length() > 0) {
    g_deviceUID = deviceID; // Update global variable
  }
  
  // Answer the ID now in force (the chip's real UID): on a fresh settings
  // file the stored value is the compiled placeholder, and answering it made
  // the FIRST read after a factory reset (or any board swap) report a value
  // the device had already replaced in memory — the truth only arrived on
  // the second read.
  if (deviceID.length() == 0) {
    deviceID = tempDeviceID;
  }
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

  if ((tempOperatingMode < CONF_OPERATING_MODE_MIN) || (tempOperatingMode > CONF_OPERATING_MODE_SETTABLE_MAX)) {  // SAFE is not a stored value (4.5.7 review)
    tempOperatingMode = CONF_OPERATING_MODE_DEFAULT;
    // Heal only a genuinely out-of-range STORED value (the getModelNumber
    // pattern): a transient failed read also converts to 0, and rewriting
    // the key from it would overwrite the user's real mode on the wire AND
    // at the next boot.
    if (mem.isLastReadOk()) {
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempOperatingMode);
    }
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

  if ((inputOperatingMode >= CONF_OPERATING_MODE_MIN) && (inputOperatingMode <= CONF_OPERATING_MODE_SETTABLE_MAX)) {  // SETTABLE_MAX excludes SAFE (4.5.7 review)
    if ((g_comMode == CONF_COM_MODE_BLE) && (inputOperatingMode == CONF_OPERATING_MODE_GAMEPAD)){
      printResponseInt(responseEnabled, apiEnabled, false, 3, "OM,1", true, inputOperatingMode);    // Return error if user tries to change to Bluetooth mode while in Gamepad mode
    } else if (mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputOperatingMode)) {
      // The settings-size budget guard can refuse the write (LSMemory) — a
      // refused write must never report SUCCESS or change the live mode, or
      // the app's readout and the device would disagree after a reboot.
      printResponseInt(responseEnabled, apiEnabled, true, 0, "OM,1", true, inputOperatingMode);
      // THE 4.5.7 FIX: g_operatingMode is NOT assigned here anymore.
      // changeOperatingMode() compares its argument against g_operatingMode
      // and resets the device when they differ — assigning the global
      // BEFORE the call (4.1.0 left the call commented out; uncommenting
      // it without reordering) made the comparison always true, so the
      // reset-on-change NEVER fired: a serial OM,1:2 left the gamepad HID
      // interface never-begun while the live mode claimed gamepad — every
      // report went out the mouse interface (the release toolchain's
      // TinyUSB hardwires instance 0), with telemetry counting the
      // misrouted reports as successes. The global now flips only inside
      // changeOperatingMode, exactly as its contract reads; the ack above
      // reaches the host during the reset's deliberate shutdown dwell.
      changeOperatingMode(inputOperatingMode);
    } else {
      printResponseInt(responseEnabled, apiEnabled, false, 3, "OM,1", true, inputOperatingMode);
    }
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "OM,1", true, inputOperatingMode);
  }
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
  // 4.9.1 read-truth doctrine (see getSapDisabled): a failed read must not
  // re-apply the compiled default to the live joystick (the old shape
  // silently rescaled every user's cursor on a transient read failure);
  // the answer is the in-force level instead.
  int tempCursorSpeedLevel;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if ((stored < CONF_JOY_CURSOR_SPEED_LEVEL_MIN) || (stored > CONF_JOY_CURSOR_SPEED_LEVEL_MAX)) {
        stored = CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, stored);
      }
      js.setOutputRange(stored);
      tempCursorSpeedLevel = stored;
    } else {
      tempCursorSpeedLevel = js.getOutputRange();  // the in-force level, not a readback
    }
  } else {
    tempCursorSpeedLevel = CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT;
    js.setOutputRange(tempCursorSpeedLevel);
  }
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
      // 4.6.6: fall back to the IN-FORCE level (the joystick's own live
      // value), not a file readback. readInt yields 0 for an unreadable file
      // (open/parse failure mid-session), which used to silently degrade the
      // live cursor to the slowest setting for the rest of the boot while
      // the endpoint answered FAIL,3 — breaking the setter's own "a refused
      // store leaves the live value untouched" contract. With a healthy
      // file the readback and the live value agree (every load/GET applies
      // the stored level onto js), so the live value is at least as correct.
      tempCursorSpeedLevel = js.getOutputRange();
    }
    if (!CONF_API_ENABLED && isValidSpeed) {
      tempCursorSpeedLevel = CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT;
    }
    performLedAction(ledCurrentState);
  }
  else {
    // Invalid inputSpeedLevel — answer the in-force value, not a readback
    // that degenerates to 0 on an unreadable file (4.6.6, as above).
    tempCursorSpeedLevel = js.getOutputRange();
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
      if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempScrollLevel);
      }
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
      // 4.6.6: fall back to the in-force level (g_scrollLevel), not a file
      // readback that degenerates to 0 on an unreadable file — a refused
      // store must leave the live scroll level untouched (the readback used
      // to collapse the live scroll speed to the minimum for the boot).
      tempScrollLevel = g_scrollLevel;
    }
    if (!CONF_API_ENABLED && isValidLevel) {
      tempScrollLevel = CONF_SCROLL_LEVEL_DEFAULT;
    }
    performLedAction(ledCurrentState);
    g_scrollLevel = tempScrollLevel;
  }
  else {
    // Invalid inputScrollLevel — answer the in-force value (4.6.6, as SS).
    tempScrollLevel = g_scrollLevel;
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
  // 4.9.1: the sensorless gate its sibling CA,0 pins as mandatory — this
  // getter is reachable on safe boots (the API table is not filtered by
  // safe mode and the connect load replays IN,0 inside EX,0), and every js
  // touch below would act on the never-begun object's state (getInputCenter
  // reads it, setMinimumRadius mutates it). Answer the zeroed default
  // center without touching state — the same shape CA,0 answers.
  if (!g_joystickSensorConnected || !js.isBegun()) {
    pointFloatType sensorlessCenter = {0.0f, 0.0f};
    printResponseFloatPoint(responseEnabled, apiEnabled, true, 0, "IN,0", true, sensorlessCenter);
    return;
  }
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
  // 4.5.7 review: refuse up front when the joystick sensor is absent — the
  // chain runs on calibrationTimer, which loop() only pumps when the sensor
  // is connected, so on a sensorless safe boot the registration succeeded,
  // the chain never fired, canOutputAction stayed false forever, and EVERY
  // input (buttons, switches, sip & puff, the safe-mode menu itself) was
  // dead until reboot — silently, with no ack and no FAIL. There is
  // nothing to center without the sensor; answer FAIL,3 so the host's
  // waiter resolves honestly.
  if (!g_joystickSensorConnected) {
    canOutputAction = true;
    printResponseInt(responseEnabled, apiEnabled, false, 3, "IN,1", true, 1);
    return;
  }
  // 4.5.7 review: one timer chain per shared id set — a center started
  // during an in-flight CALIBRATION overwrote the shared
  // calibrationTimerId[1] reading-timer variable, so the corner's
  // final-run check (which keys the flash persistence AND the CA,n ack
  // line) read the center's 5-run timer, never matched the calibration's
  // 10th run, and that corner silently never persisted while step 6 still
  // reported "calibrated". Refuse honestly; the calibration's own internal
  // step-5 center bypasses via g_calibrationCenterStep. Center flows are now
  // serialized too because they share the sample buffer and timer-id slots.
  if (g_centerRunning || (g_calibrationRunning && !g_calibrationCenterStep)) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "IN,1", true, 1);
    return;
  }
  // 4.9.4 (round 4): the same window gate CA,1 carries — a center flow
  // freezes the cursor and disables the joystick poll, and the switch
  // window's tick is serviced from the main loop, so the two would run
  // concurrently and the window's close would act on a device whose input
  // pipeline the center flow deliberately owns. Refused with the same named
  // note; the app retries once the window closes.
  //
  // 4.9.4 (round 8, fix-set audit) THE CHAIN'S OWN CALL IS EXEMPT, explicitly:
  // the full calibration's STEP 5 runs its center through THIS function
  // (performJoystickCalibration sets g_calibrationCenterStep around the call
  // precisely to bypass the in-flight gate above). That call is currently
  // unreachable while a window is open — the round-4 CA,1 gate stops a
  // calibration from starting during a window, and the executors'
  // calibrationRunning gate stops a window from opening during one, so the
  // two are mutually exclusive by construction. But this gate sits BELOW the
  // g_calibrationCenterStep bypass check, so if that mutual exclusion is ever
  // relaxed (a future caller, a re-ordered gate), step 5 would be refused
  // mid-run, g_centerRunning would stay false, and the chain would abort with
  // "center step refused" — a calibration that cannot complete, with the
  // cause three gates away. Honor the same exemption here so the chain's
  // internal call is never the one refused.
  if (lsHostSwitchWindowOpen() && !g_calibrationCenterStep) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "IN,1", true, 1);
    serialTxEnqueueLine("ERROR: IN,1 refused: a Bluetooth switch window owns the radio — retry once it closes");
    return;
  }
  // Record who initiated the center (4.5.2, wire hygiene): the completion's
  // IN,1 line is an ack (SUCCESS) only when the serial API asked for it —
  // a device-initiated center (boot auto-center, Hub-menu Center Reset,
  // the calibration's internal step 5) prints MANUAL instead, so a boot
  // straggler can never falsely resolve a host's pending IN,1:1 waiter.
  // 4.5.4: the initiator rides the flow's own timer payload
  // (CONF_CENTER_INIT_API_FLAG) instead of a shared global, so every accepted
  // flow carries its own initiator structurally.
  int stepPayload = apiEnabled ? CONF_CENTER_INIT_API_FLAG : 0;
  if (outputAction == CONF_ACTION_DRAG || outputAction == CONF_ACTION_SCROLL) {
    releaseOutputAction();
  }
  canOutputAction = false;
  g_resetCenterComplete = false;  // Freeze cursor immediately, including the 1 s start-delay window
  g_centerRunning = true;
  calibrationTimerId[0] = calibrationTimer.setTimeout(CONF_JOY_INIT_START_DELAY, performJoystickCenter, (int *)stepPayload);
  if (calibrationTimerId[0] < 0) {
    // Registration refused (a loud, counted LSTimer failure): the flow never
    // starts — without this guard the completion that would have restored
    // canOutputAction never fires and the cursor output stays dead. Answer
    // FAIL,3 so the host's IN,1 waiter resolves honestly instead of timing
    // out on a center that will never run.
    canOutputAction = true;
    g_centerRunning = false;
    g_resetCenterComplete = true;
    if (screen.showCenterResetComplete) screen.centerResetFailedPage();
    if (g_startupCenterReset) {
      g_startupCenterReset = false;
      readyToUse();
    }
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
// 4.6.6: corner plausibility gate for reads (mirror of the full-calibration
// convention, CONF_JOY_CALIB_CORNER_MIN = 3: a corner coordinate below 3 mT
// is degenerate). The capture-time convention requires BOTH axes at or
// beyond the min (a corner with one axis below is replaced by the ±13
// default at capture, LipSync_Firmware.ino) — this gate now requires BOTH
// too: the one-axis-or shape accepted corners capture itself would have
// rejected, so a stored {20, 1.5} read back as "usable" while the same
// reading at capture time could never have persisted. {0,0} (what a failed
// read used to yield implicitly) and any NaN/garbage fail this gate and the
// caller keeps the live stored corner instead.
static bool isPlausibleStoredCorner(pointFloatType p) {
  if (!isfinite(p.x) || !isfinite(p.y)) return false;
  return (fabsf(p.x) >= CONF_JOY_CALIB_CORNER_MIN) && (fabsf(p.y) >= CONF_JOY_CALIB_CORNER_MIN);
}

void getJoystickCalibration(bool responseEnabled, bool apiEnabled) {
  String commandKey;
  pointFloatType calibrationPointArray[5];
  // 4.6.6: sensorless gate. This getter is reachable on safe boots (the API
  // table is not filtered by safe mode and the app's connect load sends
  // CA,0) — every js mutation below, and getInputCenter itself, would touch
  // the never-begun object's state. Answer stored/default values only.
  const bool sensorless = !g_joystickSensorConnected || !js.isBegun();
  calibrationPointArray[0] = sensorless ? pointFloatType{0.0f, 0.0f} : js.getInputCenter();
  // 4.5.7 review: while a full calibration is IN FLIGHT, this read must not
  // touch the live corner slots — getInputMax is a max-accumulator against
  // the stored slot, so a CA,0/EX read landing inside a corner's 2 s
  // measurement window replaced the accumulating slot with the OLD stored
  // corner, and the polluted corner was then persisted at window end (the
  // user told "calibrated" while the corner silently kept its
  // pre-calibration value). The read still answers the STORED values.
  bool calibrationInFlight = g_calibrationRunning;
  // 4.6.6: a failed/implausible stored-corner read must never flow into
  // setInputMax — readPoint now reports validity, and a slot that fails the
  // gate falls back to the LIVE stored corner (no I/O) with the flag
  // clearing the radius/reprocess pass: a settings read failure used to
  // silently reset every corner to {0,0} live, including inside
  // abortJoystickCalibration's reload-to-persisted path.
  bool storedCornersUsable = true;
  for (int i = 1; i < 5; i++)
  {
    commandKey = "CA" + String(i);
    pointFloatType storedCorner = {0.0f, 0.0f};
    if (mem.readPoint(CONF_SETTINGS_FILE, commandKey, storedCorner) && isPlausibleStoredCorner(storedCorner)) {
      calibrationPointArray[i] = storedCorner;
    } else {
      calibrationPointArray[i] = js.getStoredInputMax(i);
      storedCornersUsable = false;
    }
    if (!calibrationInFlight && !sensorless) {
      js.setInputMax(i, calibrationPointArray[i]);
    }
  }
  if (!calibrationInFlight && !sensorless && storedCornersUsable) {
    js.setMinimumRadius();  // Update the minimum cursor operating radius
    js.reprocessLastReading();  // A stationary stick must reflect reloaded corners immediately
  }
  printResponseFloatPointArray(responseEnabled, apiEnabled, storedCornersUsable, storedCornersUsable ? 0 : 3, "CA,0", true, "", 5, ',', calibrationPointArray);

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
  // 4.5.7 review: same run-gate refusal as setJoystickInitialization — the
  // chain never fires on a sensorless boot (calibrationTimer is only pumped
  // when the joystick sensor is connected), and this function must bail
  // BEFORE js.clear() wipes the stored/live calibration and
  // calibrationEndsByClosingMenu arms the menu close for a flow that will
  // never run.
  if (!g_joystickSensorConnected) {
    canOutputAction = true;
    screen.calibrationEndsByClosingMenu(false);
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CA,1", true, 1);
    return;
  }
  // 4.5.7 review: one calibration at a time — a second CA,1:1 while a run
  // is in flight (a host timeout-retry lands ~4 s in; the first corner line
  // arrives ~4.4 s in) used to js.clear() the accumulating corners
  // mid-run, register a second concurrent chain, interleave two corner
  // sequences on the wire, and let last-writer-wins persistence fight over
  // the shared corner slots. Refuse honestly; the running chain finishes.
  if (g_calibrationRunning || g_centerRunning) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CA,1", true, 1);
    return;
  }
  // 4.9.4 (round 4, the symmetric half of the calibration gate): lsHostExecuteSwitch
  // and lsHostExecuteForget both refuse while a calibration/center flow is in
  // flight (calibrationRunning), but the REVERSE was never true — a CA,1:1
  // landing while a switch window is open was accepted. The two then ran
  // CONCURRENTLY: the window tick is serviced from the MAIN LOOP
  // (lsHostServiceSwitchWindow) and from btFeedbackLoop's poll, neither of
  // which calibration disables, while calibration step 0 shuts down the
  // joystick/scroll/input/pressure polls and holds canOutputAction false with
  // the cursor frozen. The window could therefore close mid-calibration —
  // admitting the target, painting LED state and (on the success path)
  // running the close's radio/LED work against a device whose input pipeline
  // is deliberately down, and the user's held corner pose would be sampled by
  // a sensor poll that is supposed to be off. Calibration is also the flow
  // that OWNS the joystick, so racing it against a radio switch has no honest
  // resolution: refuse, exactly as the switch refuses calibration, and let the
  // app retry once the window's bound elapses (60 s worst case) or the target
  // takes the link. The named note mirrors lsHostRefusalLine's calibration
  // wording so both directions read the same on the wire.
  if (lsHostSwitchWindowOpen()) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "CA,1", true, 1);
    serialTxEnqueueLine("ERROR: CA,1 refused: a Bluetooth switch window owns the radio — retry once it closes");
    return;
  }
  // Set at ACCEPTANCE, not at step 0 (4.5.7 review): the chain starts
  // CONF_JOY_CALIB_START_DELAY (1 s) after this command, and CA,1 has no
  // immediate ack — a host retry inside that window used to pass the
  // step-0-set flag, js.clear() the accumulating state, and double-chain.
  g_calibrationRunning = true;
  // The initiator rides the chain's payload (CONF_CALIB_API_FLAG): the
  // per-corner lines print SUCCESS only for a host-requested calibration
  // (CA,1:1) and MANUAL for a Hub-menu one — the 4.5.2 initiator contract.
  int stepPayload = apiEnabled ? CONF_CALIB_API_FLAG : 0;
  if (outputAction == CONF_ACTION_DRAG || outputAction == CONF_ACTION_SCROLL) {
    releaseOutputAction();
  }
  canOutputAction = false;
  g_resetCenterComplete = false;  // Freeze cursor immediately, before the delayed step-0 callback
  // 4.9.1: arm the entry freeze for the SERIAL path too. The menu path arms
  // it in fullCalibrationPage; the serial path (the app's CA,1:1) left the
  // whole START_DELAY + step-0 window (~1 s) with the current menu live to
  // the input poll — on a safe-mode boot the safe menu (Restart / Factory
  // Reset) sat registered beneath a "calibration is starting" flow, and a
  // held Select could walk its destructive confirm before step 0's poll
  // disable landed. The entry freeze covers the gap; the flow's own chain
  // re-freezes per step.
  screen.freezeMenuInputFor(CONF_JOY_CALIB_ENTRY_FREEZE_MS);
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
    g_resetCenterComplete = true;
    g_calibrationRunning = false;
    screen.calibrationAbortedPage();
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


// Load and sanitize the deadzone pair as one invariant. Both values must be
// finite, strictly inside (0,1), and inner must stay below outer; otherwise
// apply and persist the known-safe pair together. The old independent checks
// accepted inner >= outer, which reaches mapRoundInt with a zero/reversed
// input span, and the setters accepted 1.0 even though the next GET silently
// reset it.
// 4.9.1: returns whether the read that produced the values actually
// happened (read truth) — the getters use it to keep a failed read from
// rewriting the live deadzones (the read-truth doctrine, getSapDisabled);
// the setters ignore it as before.
bool loadJoystickDeadzonePair(float &innerDeadzone, float &outerDeadzone) {
  JsonObject settings = mem.readObject(CONF_SETTINGS_FILE);
  innerDeadzone = settings["IZ"].as<float>();
  outerDeadzone = settings["OZ"].as<float>();

  bool valid = mem.isLastReadOk() &&
               isfinite(innerDeadzone) && isfinite(outerDeadzone) &&
               innerDeadzone > CONF_JOY_DEADZONE_MIN &&
               innerDeadzone < CONF_JOY_DEADZONE_MAX &&
               outerDeadzone > CONF_JOY_DEADZONE_MIN &&
               outerDeadzone < CONF_JOY_DEADZONE_MAX &&
               innerDeadzone < outerDeadzone;
  if (!valid) {
    innerDeadzone = CONF_JOY_DEADZONE_INNER_DEFAULT;
    outerDeadzone = CONF_JOY_DEADZONE_OUTER_DEFAULT;
    // (4.9.0 review) the heal write is isLastReadOk-gated like every writer
    // now is — on a failed read the null-document check alone could pass (a
    // truncated-but-parseable file is non-null) and persist defaults over a
    // file the next boot would have re-seeded wholesale.
    if (!settings.isNull() && mem.isLastReadOk()) {
      settings["IZ"] = innerDeadzone;
      settings["OZ"] = outerDeadzone;
      mem.writeObject(CONF_SETTINGS_FILE, "IZ/OZ", settings);
    }
  }
  return mem.isLastReadOk();
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
  float tempDeadzone;
  float tempOuterDeadzone;
  const bool dzReadOk = loadJoystickDeadzonePair(tempDeadzone, tempOuterDeadzone);
  (void)tempOuterDeadzone;
  // 4.9.1 read-truth doctrine (see getSapDisabled): a failed read must not
  // rewrite the live deadzone and reprocess the cursor — the answer is the
  // in-force factor instead.
  if (dzReadOk) {
    js.setInnerDeadzone(true, tempDeadzone);
    js.reprocessLastReading();
  } else {
    tempDeadzone = js.getInnerDeadzoneFactor();
  }
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
  float currentInnerDeadzone;
  float currentOuterDeadzone;
  loadJoystickDeadzonePair(currentInnerDeadzone, currentOuterDeadzone);
  (void)currentInnerDeadzone;
  if ((isfinite(inputDeadzone) &&
       (inputDeadzone > CONF_JOY_DEADZONE_MIN) &&
       (inputDeadzone < CONF_JOY_DEADZONE_MAX) &&
       (inputDeadzone < currentOuterDeadzone))
      && mem.writeFloat(CONF_SETTINGS_FILE, deadzoneCommand, inputDeadzone)) {
    // The store and the live value only land together: a refused store
    // (settings budget, LSMemory) falls to the FAIL,3 branch below.
    js.setInnerDeadzone(true, inputDeadzone);
    js.reprocessLastReading();
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
  float tempInnerDeadzone;
  float tempOuterDeadzone;
  const bool dzReadOk = loadJoystickDeadzonePair(tempInnerDeadzone, tempOuterDeadzone);
  (void)tempInnerDeadzone;
  // 4.9.1 read-truth doctrine (see getSapDisabled).
  if (dzReadOk) {
    js.setOuterDeadzone(true, tempOuterDeadzone);
    js.reprocessLastReading();
  } else {
    tempOuterDeadzone = js.getOuterDeadzoneFactor();
  }
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "OZ,0", true, tempOuterDeadzone);
  return tempOuterDeadzone;
}
//***GET JOYSTICK OUTER DEADZONE API FUNCTION***//
// Function   : getJoystickOuterDeadzone
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
  float currentInnerDeadzone;
  float currentOuterDeadzone;
  loadJoystickDeadzonePair(currentInnerDeadzone, currentOuterDeadzone);
  (void)currentOuterDeadzone;
  if ((isfinite(inputUpperDeadZone) &&
       (inputUpperDeadZone > CONF_JOY_DEADZONE_MIN) &&
       (inputUpperDeadZone < CONF_JOY_DEADZONE_MAX) &&
       (inputUpperDeadZone > currentInnerDeadzone))
      && mem.writeFloat(CONF_SETTINGS_FILE, outerDeadzoneCommand, inputUpperDeadZone)) {
    // See setJoystickInnerDeadzone: a refused store must not report SUCCESS.
    js.setOuterDeadzone(true, inputUpperDeadZone);
    js.reprocessLastReading();
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
  // Read the 50 ms pressure poll's last complete sample instead of taking an
  // extra synchronous sample in the serial-dispatch path. Besides making a
  // flaky I2C device able to stall command handling, the extra update could
  // run immediately beside the scheduled poll and read the same 25 Hz sensor
  // conversion twice. That defeated the 4.6.5 consecutive-sample engagement
  // qualifier while Connect was watching PV: one transient conversion could
  // be counted twice and become a click. The scheduled pressureLoop is the
  // sole owner of sensor sampling and sip/puff state advancement.
  
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
  if (g_joystickSensorConnected) joystickHealth = (!js.isBegun() || js.isPollBackedOff()) ? 2 : 1;

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
  // 4.9.1 read-truth doctrine (see getSapDisabled): a failed read answers
  // the in-force threshold and leaves the live sensor untouched — the old
  // shape re-applied the compiled default to ps on every transient read
  // failure.
  float tempSipThreshold;
  if (CONF_API_ENABLED) {
    tempSipThreshold = mem.readFloat(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (!isfinite(tempSipThreshold) ||
          (tempSipThreshold < CONF_PRESS_MIN_THRESHOLD) ||
          (tempSipThreshold >= CONF_PRESS_MAX_THRESHOLD)) {
        tempSipThreshold = CONF_SIP_THRESHOLD;
        // 4.9.1: heal the pair only after the heal write actually landed —
        // healHardThresholds re-enters this getter for its floor, and a
        // persistently failing write would otherwise recurse
        // heal->getter->heal forever. (healHardThresholds also carries a
        // re-entrancy guard; this gate keeps the depth at one level.)
        if (mem.writeFloat(CONF_SETTINGS_FILE, commandKey, tempSipThreshold)) {
          screen.healHardThresholds();  // the heal can strand the hard band — same rule as the setter
        }
      }
      ps.setSipThreshold(tempSipThreshold);
    } else {
      tempSipThreshold = ps.getSipPressureThreshold();  // in force, not a readback
    }
  } else {
    tempSipThreshold = CONF_SIP_THRESHOLD;
    ps.setSipThreshold(tempSipThreshold);
  }
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

  if ((isfinite(inputSipThreshold) && inputSipThreshold >= CONF_PRESS_MIN_THRESHOLD && inputSipThreshold < CONF_PRESS_MAX_THRESHOLD)
      && mem.writeFloat(CONF_SETTINGS_FILE, commandKey, inputSipThreshold)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 — a
    // SUCCESS here would tell the app a threshold the device did not keep.
    ps.setSipThreshold(inputSipThreshold);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "ST,1", true, inputSipThreshold);
    // A raised regular floor can strand the hard band (hard < regular +
    // margin classifies as an empty hard pair — the feature silently goes
    // inert). The menu path heals after every step; the serial path — which
    // the app uses on every connect — heals here with the same rule
    // (screen.healHardThresholds emits the usual MANUAL lines so Connect
    // re-syncs the healed values).
    screen.healHardThresholds();
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
  // 4.9.1 read-truth doctrine (see getSipPressureThreshold).
  float tempPuffThreshold;
  if (CONF_API_ENABLED) {
    tempPuffThreshold = mem.readFloat(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (!isfinite(tempPuffThreshold) ||
          (tempPuffThreshold < CONF_PRESS_MIN_THRESHOLD) ||
          (tempPuffThreshold >= CONF_PRESS_MAX_THRESHOLD)) {
        tempPuffThreshold = CONF_PUFF_THRESHOLD;
        // 4.9.1: heal only after the write lands (see getSipPressureThreshold).
        if (mem.writeFloat(CONF_SETTINGS_FILE, commandKey, tempPuffThreshold)) {
          screen.healHardThresholds();  // the heal can strand the hard band — same rule as the setter
        }
      }
      ps.setPuffThreshold(tempPuffThreshold);
    } else {
      tempPuffThreshold = ps.getPuffPressureThreshold();  // in force, not a readback
    }
  } else {
    tempPuffThreshold = CONF_PUFF_THRESHOLD;
    ps.setPuffThreshold(tempPuffThreshold);
  }
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

  if ((isfinite(inputPuffThreshold) && inputPuffThreshold >= CONF_PRESS_MIN_THRESHOLD && inputPuffThreshold < CONF_PRESS_MAX_THRESHOLD)
      && mem.writeFloat(CONF_SETTINGS_FILE, commandKey, inputPuffThreshold)) {
    // See setSipPressureThreshold: a refused store reports FAIL,3.
    ps.setPuffThreshold(inputPuffThreshold);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "PT,1", true, inputPuffThreshold);
    // See setSipPressureThreshold: a raised regular floor heals the hard pair here too.
    screen.healHardThresholds();
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

//***GET HARD SIP PRESSURE THRESHOLD FUNCTION***//
// Function   : getHardSipPressureThreshold
//
// Description: Returns the hard sip threshold (upstream #58). The hard band
//              is a second, higher-threshold band: a hold that STARTS beyond
//              it fires its action with no hold-duration requirement. Values
//              live in /settings2.dat. A fresh value is written at first read
//              if missing or invalid. The hard threshold is constrained to
//              be above the regular sip threshold plus CONF_PRESS_HARD_MARGIN_HPA.
//
// Parameters :  responseEnabled : bool : The serial print output is enabled if true.
//               apiEnabled : bool : The API response is sent if true.
//
// Return     : float : the current hard sip threshold [hPa]
//*********************************//
float getHardSipPressureThreshold(bool responseEnabled, bool apiEnabled) {
  String commandKey = "HS";
  // 4.9.1 read-truth doctrine (see getSipPressureThreshold): the heal here
  // raises a hard floor — that CAN strand the band (hard < regular + margin
  // on a regular threshold raised separately), so the getter's heal heals
  // the pair exactly like the setter's.
  float tempHardSipThreshold;
  if (CONF_API_ENABLED) {
    tempHardSipThreshold = mem.readFloat(CONF_SETTINGS2_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (!isfinite(tempHardSipThreshold) ||
          (tempHardSipThreshold < CONF_PRESS_MIN_THRESHOLD) ||
          (tempHardSipThreshold >= CONF_PRESS_MAX_THRESHOLD)) {
        tempHardSipThreshold = CONF_HARD_SIP_THRESHOLD;
        // 4.9.1: heal only after the write lands (see getSipPressureThreshold).
        if (mem.writeFloat(CONF_SETTINGS2_FILE, commandKey, tempHardSipThreshold)) {
          screen.healHardThresholds();
        }
      }
      ps.setHardSipThreshold(tempHardSipThreshold);
    } else {
      tempHardSipThreshold = ps.getHardSipThreshold();  // in force, not a readback
    }
  } else {
    tempHardSipThreshold = CONF_HARD_SIP_THRESHOLD;
    ps.setHardSipThreshold(tempHardSipThreshold);
  }
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "HS,0", true, tempHardSipThreshold);

  return tempHardSipThreshold;
}

//***GET HARD SIP PRESSURE THRESHOLD API FUNCTION***//
// Function   : getHardSipPressureThreshold (overload)
//
// Description: API entry point for HS,0 — the getter's contract (the heal
//              write of a fresh/invalid stored value included), not a pure read.
//
// Parameters :  responseEnabled, apiEnabled, optionalParameter
//
// Return     : void
//*********************************//
void getHardSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getHardSipPressureThreshold(responseEnabled, apiEnabled);
  }
}

//***SET HARD SIP PRESSURE THRESHOLD FUNCTION***//
// Function   : setHardSipPressureThreshold
//
// Description: Sets the hard sip threshold (upstream #58). In addition to the
//              regular absolute range, the value must sit at least
//              CONF_PRESS_HARD_MARGIN_HPA above the LIVE regular sip
//              threshold — the hard band must stay a clearly distinct
//              gesture, and the classifier's hard-sticky semantics rely on
//              hard > regular. A value that violates the margin is refused
//              (FAIL,3, nothing written); the boot-time heal for out-of-order
//              STORED values lives in initSipAndPuff, not here — a serial SET
//              must never silently rewrite what the host sent.
//
// Parameters :  responseEnabled : bool
//               apiEnabled : bool
//               inputHardSipThreshold : float : new hard sip threshold [hPa]
//
// Return     : void
//*********************************//
void setHardSipPressureThreshold(bool responseEnabled, bool apiEnabled, float inputHardSipThreshold) {
  String commandKey = "HS";

  float minHard = ps.getSipPressureThreshold() + CONF_PRESS_HARD_MARGIN_HPA;

  if ((isfinite(inputHardSipThreshold) &&
       inputHardSipThreshold >= CONF_PRESS_MIN_THRESHOLD &&
       inputHardSipThreshold < CONF_PRESS_MAX_THRESHOLD &&
       inputHardSipThreshold >= minHard)
      && mem.writeFloat(CONF_SETTINGS2_FILE, commandKey, inputHardSipThreshold)) {
    // A refused store (settings budget, LSMemory) reports FAIL,3 — same
    // contract as the ST/PT setters.
    ps.setHardSipThreshold(inputHardSipThreshold);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "HS,1", true, inputHardSipThreshold);
  }
  else {
    printResponseFloat(responseEnabled, apiEnabled, false, 3, "HS,1", true, inputHardSipThreshold);
  }
}

//***SET HARD SIP PRESSURE THRESHOLD API FUNCTION***//
// Function   : setHardSipPressureThreshold (overload)
//
// Description: API entry point for HS,1 (float parameter).
//
// Parameters :  responseEnabled, apiEnabled, optionalParameter
//
// Return     : void
//*********************************//
void setHardSipPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setHardSipPressureThreshold(responseEnabled, apiEnabled, optionalParameter.toFloat());
}

//***GET HARD PUFF PRESSURE THRESHOLD FUNCTION***//
// Function   : getHardPuffPressureThreshold
//
// Description: Returns the hard puff threshold (upstream #58). The hard band
//              is a second, higher-threshold band: a hold that STARTS beyond
//              it fires its action with no hold-duration requirement. Values
//              live in /settings2.dat. A fresh value is written at first read
//              if missing or invalid. The hard threshold is constrained to
//              be above the regular puff threshold plus
//              CONF_PRESS_HARD_MARGIN_HPA (the setter enforces the margin;
//              this getter enforces the absolute range).
//
// Parameters :  responseEnabled : bool : The serial print output is enabled if true.
//               apiEnabled : bool : The API response is sent if true.
//
// Return     : float : the current hard puff threshold [hPa]
//*********************************//
float getHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled) {
  String commandKey = "HP";
  // 4.9.1 read-truth doctrine (see getSipPressureThreshold). The heal here
  // cannot strand the band from THIS side (a lowered hard floor only
  // widens it), but healHardThresholds is the one code path — keep it
  // symmetric so a future threshold edit cannot silently diverge.
  float tempHardPuffThreshold;
  if (CONF_API_ENABLED) {
    tempHardPuffThreshold = mem.readFloat(CONF_SETTINGS2_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (!isfinite(tempHardPuffThreshold) ||
          (tempHardPuffThreshold < CONF_PRESS_MIN_THRESHOLD) ||
          (tempHardPuffThreshold >= CONF_PRESS_MAX_THRESHOLD)) {
        tempHardPuffThreshold = CONF_HARD_PUFF_THRESHOLD;
        // 4.9.1: heal only after the write lands (see getSipPressureThreshold).
        if (mem.writeFloat(CONF_SETTINGS2_FILE, commandKey, tempHardPuffThreshold)) {
          screen.healHardThresholds();
        }
      }
      ps.setHardPuffThreshold(tempHardPuffThreshold);
    } else {
      tempHardPuffThreshold = ps.getHardPuffThreshold();  // in force, not a readback
    }
  } else {
    tempHardPuffThreshold = CONF_HARD_PUFF_THRESHOLD;
    ps.setHardPuffThreshold(tempHardPuffThreshold);
  }
  printResponseFloat(responseEnabled, apiEnabled, true, 0, "HP,0", true, tempHardPuffThreshold);

  return tempHardPuffThreshold;
}

//***GET HARD PUFF PRESSURE THRESHOLD API FUNCTION***//
// Function   : getHardPuffPressureThreshold (overload)
//
// Description: API entry point for HP,0 — the getter's contract (the heal
//              write of a fresh/invalid stored value included), not a pure read.
//
// Parameters :  responseEnabled, apiEnabled, optionalParameter
//
// Return     : void
//*********************************//
void getHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getHardPuffPressureThreshold(responseEnabled, apiEnabled);
  }
}

//***SET HARD PUFF PRESSURE THRESHOLD FUNCTION***//
// Function   : setHardPuffPressureThreshold
//
// Description: Sets the hard puff threshold (upstream #58). Mirror of the HS
//              setter — see its comment for the margin rationale.
//
// Parameters :  responseEnabled : bool
//               apiEnabled : bool
//               inputHardPuffThreshold : float : new hard puff threshold [hPa]
//
// Return     : void
//*********************************//
void setHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled, float inputHardPuffThreshold) {
  String commandKey = "HP";

  float minHard = ps.getPuffPressureThreshold() + CONF_PRESS_HARD_MARGIN_HPA;

  if ((isfinite(inputHardPuffThreshold) &&
       inputHardPuffThreshold >= CONF_PRESS_MIN_THRESHOLD &&
       inputHardPuffThreshold < CONF_PRESS_MAX_THRESHOLD &&
       inputHardPuffThreshold >= minHard)
      && mem.writeFloat(CONF_SETTINGS2_FILE, commandKey, inputHardPuffThreshold)) {
    ps.setHardPuffThreshold(inputHardPuffThreshold);
    printResponseFloat(responseEnabled, apiEnabled, true, 0, "HP,1", true, inputHardPuffThreshold);
  }
  else {
    printResponseFloat(responseEnabled, apiEnabled, false, 3, "HP,1", true, inputHardPuffThreshold);
  }
}

//***SET HARD PUFF PRESSURE THRESHOLD API FUNCTION***//
// Function   : setHardPuffPressureThreshold (overload)
//
// Description: API entry point for HP,1 (float parameter).
//
// Parameters :  responseEnabled, apiEnabled, optionalParameter
//
// Return     : void
//*********************************//
void setHardPuffPressureThreshold(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setHardPuffPressureThreshold(responseEnabled, apiEnabled, optionalParameter.toFloat());
}

//***GET HARD SIP / PUFF ACTION FUNCTION***//
// Function   : getHardSipAction / getHardPuffAction
//
// Description: Returns the hard-band mouse output action (upstream #58). HA =
//              hard sip's mouse action, HB = hard puff's mouse action. Stored
//              in /settings2.dat; the seed document carries the real compiled
//              defaults (5 = SCROLL for hard sip, 4 = DRAG for hard puff), and
//              a stored value outside the action-number range is healed back
//              to the compiled default at read. CONF_ACTION_NOTHING (0) is a
//              legal mapping, not a sentinel — with one accepted edge: a
//              settings2 file that PARSES but lacks the HA/HB keys (a
//              hand-edited one; the boot seed always writes them) reads back
//              0, which is NOTHING — the hard band goes inert, same
//              fail-safe direction as every other corrupt-settings case, and
//              the first SET of HA/HB re-arms it.
//
// Parameters :  responseEnabled : bool
//               apiEnabled : bool
//               commandKey : const char* : "HA" or "HB"
//
// Return     : int : the current hard-band action index
//*********************************//
static int getHardAction(bool responseEnabled, bool apiEnabled, const char* commandKey, int compiledDefault) {
  int tempHardAction;
  const bool hardSip = (commandKey[1] == 'A');
  if (CONF_API_ENABLED) {
    tempHardAction = mem.readInt(CONF_SETTINGS2_FILE, commandKey);

    // Heal bound is the ACTION VOCABULARY, not an arbitrary 127: actions run
    // 0..CONF_ACTION_FACTORY_RESET, so a stored 50 (hand-edited/foreign file)
    // used to be answered with SUCCESS while the live table kept the compiled
    // row — the read reported a value not in force (the read-vs-truth class).
    // Out-of-vocabulary values heal to the compiled default here; the live
    // table additionally applies the caregiver allowlist at load (a stored
    // in-range but disallowed action keeps the compiled row there).
    // 4.9.0: the getter's ANSWER is now allowlist-gated too — an in-vocabulary
    // but disallowed action (e.g. a hand-edited HB=24 FACTORY_RESET, which
    // switchRemapAllowed refuses) used to be answered with SUCCESS while the
    // live table kept the compiled row, hiding the divergence from the app.
    // The answer falls back to the compiled default exactly like the live
    // load path does, so what the app sees is always what is in force.
    // 4.9.1: a FAILED read is answered with the LIVE row, not the read's
    // fabricated 0 (NOTHING is in vocabulary and allowlisted, so the old
    // shape answered "nothing" with SUCCESS while the device kept firing the
    // real action) — and the live table is never rewritten from a failed
    // read (the read-truth doctrine, getSapDisabled).
    if (mem.isLastReadOk()) {
      if ((tempHardAction < CONF_ACTION_NOTHING) || (tempHardAction > CONF_ACTION_FACTORY_RESET)
          || !switchRemapAllowed(tempHardAction, false)) {
        tempHardAction = compiledDefault;
        mem.writeInt(CONF_SETTINGS2_FILE, commandKey, tempHardAction);
      }
    } else {
      tempHardAction = hardBandLiveAction(hardSip);
    }
  } else {
    tempHardAction = compiledDefault;
  }
  String responseKey = String(commandKey) + ",0";
  printResponseInt(responseEnabled, apiEnabled, true, 0, responseKey, true, tempHardAction);
  return tempHardAction;
}

//***GET HARD SIP ACTION FUNCTION***//
float getHardSipAction(bool responseEnabled, bool apiEnabled) {
  return (float)getHardAction(responseEnabled, apiEnabled, "HA", CONF_ACTION_SCROLL);
}

//***GET HARD SIP ACTION API FUNCTION***//
void getHardSipAction(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getHardSipAction(responseEnabled, apiEnabled);
  }
}

//***GET HARD PUFF ACTION FUNCTION***//
float getHardPuffAction(bool responseEnabled, bool apiEnabled) {
  return (float)getHardAction(responseEnabled, apiEnabled, "HB", CONF_ACTION_DRAG);
}

//***GET HARD PUFF ACTION API FUNCTION***//
void getHardPuffAction(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 0) {
    getHardPuffAction(responseEnabled, apiEnabled);
  }
}

//***SET HARD SIP / PUFF ACTION FUNCTION***//
// Function   : setHardSipAction / setHardPuffAction
//
// Description: Sets the hard-band mouse output action (upstream #58). The
//              value is validated against the caregiver-safe switch-remap
//              allowlist (roadmap item 11's guard — one hard gesture can
//              never be remapped to erase settings or restart the device;
//              mouse column, so drag/scroll — the compiled defaults — stay
//              legal). On success the action is written to /settings2.dat
//              and applied to the LIVE SAP table immediately (4.8.1 — the
//              old store-only shape needed a reboot; applyHardBandAction-
//              Overrides also runs at boot via initSapActions).
//
// Parameters :  responseEnabled : bool
//               apiEnabled : bool
//               commandKey : const char* : "HA" or "HB"
//               inputHardAction : int : new hard-band action index
//               compiledDefault : int : default when stored value is absent/invalid
//
// Return     : void
//*********************************//
static void setHardAction(bool responseEnabled, bool apiEnabled, const char* commandKey, int inputHardAction, int compiledDefault) {
  (void)compiledDefault;  // documented for the getter's heal path; the setter stores what it is given
  String responseKey = String(commandKey) + ",1";
  if (switchRemapAllowed(inputHardAction, false)) {
    if (mem.writeInt(CONF_SETTINGS2_FILE, commandKey, inputHardAction)) {
      // Apply LIVE, not just at the next boot: every sibling setter (SA-GC
      // apply to the live remap table; HS/HP call ps.setHard*Threshold)
      // takes effect immediately, and the getter contract says "the first
      // SET of HA/HB re-arms" a table that a keyless file left inert. The
      // old store-only shape acked SUCCESS while the gesture kept firing
      // the old action until a reboot — no wire hint that a reboot was
      // "required".
      applyHardBandActionOverrides();
      printResponseInt(responseEnabled, apiEnabled, true, 0, responseKey, true, inputHardAction);
    }
    else {
      printResponseInt(responseEnabled, apiEnabled, false, 3, responseKey, true, inputHardAction);
    }
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, responseKey, true, inputHardAction);
  }
}

//***SET HARD SIP ACTION FUNCTION***//
void setHardSipAction(bool responseEnabled, bool apiEnabled, int inputHardSipAction) {
  setHardAction(responseEnabled, apiEnabled, "HA", inputHardSipAction, CONF_ACTION_SCROLL);
}

//***SET HARD SIP ACTION API FUNCTION***//
void setHardSipAction(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setHardSipAction(responseEnabled, apiEnabled, optionalParameter.toInt());
}

//***SET HARD PUFF ACTION FUNCTION***//
void setHardPuffAction(bool responseEnabled, bool apiEnabled, int inputHardPuffAction) {
  setHardAction(responseEnabled, apiEnabled, "HB", inputHardPuffAction, CONF_ACTION_DRAG);
}

//***SET HARD PUFF ACTION API FUNCTION***//
void setHardPuffAction(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setHardPuffAction(responseEnabled, apiEnabled, optionalParameter.toInt());
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
      if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempJoystickAccelerationLevel);
      }
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
      // 4.6.6: fall back to the in-force level (the live `acceleration`
      // global), not a file readback that degenerates to 0 on an unreadable
      // file — a refused store must leave the live acceleration untouched.
      tempJoystickAccelerationLevel = acceleration;
    } else if (!CONF_API_ENABLED) {
      tempJoystickAccelerationLevel = CONF_JOY_ACCELERATION_LEVEL_DEFAULT;
    }
  }
  else {
    // Invalid inputAccelerationLevel — answer the in-force value (4.6.6).
    tempJoystickAccelerationLevel = acceleration;
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
// (4.9.0 review) increaseJoystickAcceleration / decreaseJoystickAcceleration
// removed here: zero call sites (the dispatch table exposes only AV,0/AV,1
// and the Hub menu steps acceleration via get/setJoystickAcceleration
// directly) — the same dead-code class the 4.8.1 round removed.

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
    if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempComMode);
    }
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
  int previousComMode = g_comMode;  // captured BEFORE anything mutates it (4.5.8: the reset-on-change compare must see the old value — the exact ordering bug that killed the OM reset for a year)

  if ((inputCommunicationMode >= CONF_COM_MODE_MIN) && (inputCommunicationMode <= CONF_COM_MODE_MAX)) {
    if ((inputCommunicationMode == CONF_COM_MODE_BLE) && (g_operatingMode == CONF_OPERATING_MODE_GAMEPAD)){
      printResponseInt(responseEnabled, apiEnabled, false, 3, "CM,1", true, inputCommunicationMode);    // Return error if user tries to change to Bluetooth mode while in Gamepad mode
    } else if (mem.writeInt(CONF_SETTINGS_FILE, commandKey, inputCommunicationMode)) {
      // Store first, switch second: a refused store (settings budget,
      // LSMemory) must not flip the live mode the next boot would forget.
      g_comMode = inputCommunicationMode;
      printResponseInt(responseEnabled, apiEnabled, true, 0, "CM,1", true, inputCommunicationMode);

      // 4.5.8 BENCH FIX — the live interface hand-off is GONE; a mode change
      // now works like the operating-mode change: save, ack, software reset,
      // and the next boot begins the right interface fresh. The bench caught
      // the live hand-off dead-hanging the firmware twice (session logs
      // 2026-10-25): the pre-hand-off setLedDefault() called
      // Bluefruit.connected() on a never-begun SoftDevice (never returned —
      // no ack, 30 s watchdog death), and even with that guarded, a runtime
      // SoftDevice enable mid-session remained the fragile path. The reset
      // flow is the one the bench validated (OM,1:x worked perfectly twice);
      // the app's reconnect-after-reset flow handles the port drop. Same-mode
      // writes stay a no-op ack (idempotent, the app's restore flow relies on
      // it); the menu's mode change already reset (finishChangeMode), so this
      // is consistent behavior, not new.
      if (previousComMode != inputCommunicationMode) {
        softwareReset();  // ack already printed; the reset flow is bench-proven
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

bool storeModePair(int operatingMode, int communicationMode) {
  if (operatingMode < CONF_OPERATING_MODE_MIN ||
      operatingMode > CONF_OPERATING_MODE_SETTABLE_MAX ||
      communicationMode < CONF_COM_MODE_MIN ||
      communicationMode > CONF_COM_MODE_MAX ||
      (operatingMode == CONF_OPERATING_MODE_GAMEPAD && communicationMode == CONF_COM_MODE_BLE)) {
    return false;
  }
  JsonObject settings = mem.readObject(CONF_SETTINGS_FILE);
  if (!mem.isLastReadOk() || settings.isNull()) return false;
  settings["OM"] = operatingMode;
  settings["CM"] = communicationMode;
  return mem.writeObject(CONF_SETTINGS_FILE, "OM/CM", settings);
}

void setModePreset(bool responseEnabled, bool apiEnabled, int inputPreset) {
  int targetOperatingMode = CONF_OPERATING_MODE_NONE;
  int targetCommunicationMode = CONF_COM_MODE_NONE;
  switch (inputPreset) {
    case CONF_MODE_PRESET_MOUSE_USB:
      targetOperatingMode = CONF_OPERATING_MODE_MOUSE;
      targetCommunicationMode = CONF_COM_MODE_USB;
      break;
    case CONF_MODE_PRESET_MOUSE_BLE:
      targetOperatingMode = CONF_OPERATING_MODE_MOUSE;
      targetCommunicationMode = CONF_COM_MODE_BLE;
      break;
    case CONF_MODE_PRESET_GAMEPAD_USB:
      targetOperatingMode = CONF_OPERATING_MODE_GAMEPAD;
      targetCommunicationMode = CONF_COM_MODE_USB;
      break;
    default:
      printResponseInt(responseEnabled, apiEnabled, false, 3, "MP,1", true, inputPreset);
      return;
  }

  const bool changed = (g_operatingMode != targetOperatingMode) ||
                       (g_comMode != targetCommunicationMode);
  if (!storeModePair(targetOperatingMode, targetCommunicationMode)) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "MP,1", true, inputPreset);
    return;
  }

  printResponseInt(responseEnabled, apiEnabled, true, 0, "MP,1", true, inputPreset);
  if (changed) softwareReset();
}

void setModePreset(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  setModePreset(responseEnabled, apiEnabled, optionalParameter.toInt());
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
  // 4.5.7 review: the live global used to be incremented BEFORE
  // setCommunicationMode validated, and the wrap landed on
  // CONF_COM_MODE_NONE (0) — a mode the setter accepts and persists but
  // whose hand-off switch has no case (no interface begun) and whose boot
  // path begins no HID: one extra press of a CHANGE_MODE-mapped switch
  // silently killed the cursor and survived a power cycle. The toggle now
  // computes the candidate locally, wraps BLE -> USB (never None), and
  // leaves the global to setCommunicationMode's success path (a rejected
  // BLE-in-gamepad toggle no longer leaves g_comMode claiming a mode the
  // interfaces never entered).
  int nextComMode = (g_comMode >= CONF_COM_MODE_MAX) ? CONF_COM_MODE_USB : (g_comMode + 1);
  setCommunicationMode(responseEnabled, apiEnabled, nextComMode);
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
    if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempSoundMode);
    }
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
    if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempLightMode);
    }
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
// Description: This function sets the state of light mode.
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
    setLedDefault();  // Apply OFF/status changes immediately, not on a later unrelated event
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
    if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempLightLevel);
    }
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

    // Feedback belongs to a successful setting change only. The old
    // unconditional block blinked even when validation/persistence failed.
    setLedState(LED_ACTION_BLINK,
                LED_CLR_RED,
                CONF_LED_ALL,
                1,
                1000,
                led.getLedBrightness());
    performLedAction(ledCurrentState);
  }
  else {
    printResponseInt(responseEnabled, apiEnabled, false, 3, "LL,1", true, inputLightLevel);

  }

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
  // 4.9.1 (read-truth doctrine, the HA/HB 4.9.0 precedent): a FAILED read
  // must not act on the value it yields. readInt converts a failed read to
  // 0, and the old shape applied that 0 to the LIVE gate — a transient
  // settings read failure silently re-opened a caregiver-disabled
  // mouthpiece (fail-open, the opposite of every other guard's direction)
  // while the wire said SUCCESS. The live-apply now runs only on a read
  // that actually happened, and the failed-read answer is the IN-FORCE
  // gate — what the app displays is what the device enforces.
  int tempSapDisabled = g_sapDisabled ? 1 : 0;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if ((stored != 0) && (stored != 1)) {
        stored = CONF_SAP_DISABLED_DEFAULT;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, stored);  // heal a stored value — the read is ok here
      }
      g_sapDisabled = (stored == 1);
      tempSapDisabled = stored;
    }
  } else {
    tempSapDisabled = CONF_SAP_DISABLED_DEFAULT;
    g_sapDisabled = (tempSapDisabled == 1);
  }
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
  // 4.9.1 read-truth doctrine (see getSapDisabled): a failed read answers
  // the in-force gate and leaves it untouched — the old shape applied the
  // read-failure 0 live, silently lifting a caregiver menu restriction.
  int tempMenuButtonsOnly = g_menuButtonsOnly ? 1 : 0;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if ((stored != 0) && (stored != 1)) {
        stored = CONF_MENU_BUTTONS_ONLY_DEFAULT;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, stored);
      }
      g_menuButtonsOnly = (stored == 1);
      tempMenuButtonsOnly = stored;
    }
  } else {
    tempMenuButtonsOnly = CONF_MENU_BUTTONS_ONLY_DEFAULT;
    g_menuButtonsOnly = (tempMenuButtonsOnly == 1);
  }
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
  // 4.9.1 read-truth doctrine (see getSapDisabled): a failed read answers
  // the in-force lock and leaves it untouched — the old shape applied the
  // read-failure 0 live, silently unlocking the mode on a transient read
  // failure.
  int tempModeLocked = g_modeLocked ? 1 : 0;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if ((stored != 0) && (stored != 1)) {
        stored = CONF_MODE_LOCK_DEFAULT;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, stored);
      }
      g_modeLocked = (stored == 1);
      tempModeLocked = stored;
    }
  } else {
    tempModeLocked = CONF_MODE_LOCK_DEFAULT;
    g_modeLocked = (tempModeLocked == 1);
  }
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
  // 4.9.1 read-truth doctrine (see getSapDisabled): a failed read answers
  // the in-force hold gate and leaves it untouched.
  int tempCursorHold = g_freezeCursorDuringSap ? 1 : 0;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if ((stored != 0) && (stored != 1)) {
        stored = CONF_CURSOR_HOLD_DEFAULT;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, stored);
      }
      g_freezeCursorDuringSap = (stored == 1);
      tempCursorHold = stored;
    }
  } else {
    tempCursorHold = CONF_CURSOR_HOLD_DEFAULT;
    g_freezeCursorDuringSap = (tempCursorHold == 1);
  }
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
  // 4.9.1 read-truth doctrine (see getSapDisabled): the live window apply
  // runs only on a read that actually happened; a failed read answers the
  // in-force window (0 = the compiled default, the same shape the live
  // value uses).
  int tempWindow = (g_inputWindowShortMs > 0) ? (int)g_inputWindowShortMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (stored < 0) {
        stored = 0;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, 0);
      }
      g_inputWindowShortMs = (stored > 0) ? (unsigned long)stored : 0;
      sanitizeInputWindows();
      tempWindow = (g_inputWindowShortMs > 0) ? (int)g_inputWindowShortMs : CONF_INPUT_WINDOW_SHORT_DEFAULT_MS;
    }
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
  // The reset-to-default (0) path is validated like any other value
  // (4.5.7 review): the old `inputWindowMs == 0 ||` arm short-circuited the
  // ordering check, so a legal sequence (WS,1:200; WL,1:400; WS,1:0)
  // produced effective short=1000 > long=400 — an inverted window set
  // where every quick puff matched the MAX row instead of the short one.
  bool rangeOk = (inputWindowMs == 0) ||
                 (inputWindowMs >= CONF_INPUT_WINDOW_SHORT_MIN_MS && inputWindowMs <= CONF_INPUT_WINDOW_SHORT_MAX_MS);
  if (rangeOk && newEff < longEff && mem.writeInt(CONF_SETTINGS_FILE, commandKey, (int)inputWindowMs)) {
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
  // 4.9.1 read-truth doctrine (see getInputWindowShort).
  int tempWindow = (g_inputWindowLongMs > 0) ? (int)g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (stored < 0) {
        stored = 0;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, 0);
      }
      g_inputWindowLongMs = (stored > 0) ? (unsigned long)stored : 0;
      sanitizeInputWindows();
      tempWindow = (g_inputWindowLongMs > 0) ? (int)g_inputWindowLongMs : CONF_INPUT_WINDOW_LONG_DEFAULT_MS;
    }
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
  // Reset path validated like any other value — see setInputWindowShort (4.5.7 review).
  bool rangeOk = (inputWindowMs == 0) ||
                 (inputWindowMs >= CONF_INPUT_WINDOW_LONG_MIN_MS && inputWindowMs <= CONF_INPUT_WINDOW_LONG_MAX_MS);
  if (rangeOk && newEff > shortEff && newEff < maxEff && mem.writeInt(CONF_SETTINGS_FILE, commandKey, (int)inputWindowMs)) {
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
  // 4.9.1 read-truth doctrine (see getInputWindowShort).
  int tempWindow = (g_inputWindowMaxMs > 0) ? (int)g_inputWindowMaxMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
  if (CONF_API_ENABLED) {
    int stored = mem.readInt(CONF_SETTINGS_FILE, commandKey);
    if (mem.isLastReadOk()) {
      if (stored < 0) {
        stored = 0;
        mem.writeInt(CONF_SETTINGS_FILE, commandKey, 0);
      }
      g_inputWindowMaxMs = (stored > 0) ? (unsigned long)stored : 0;
      sanitizeInputWindows();
      tempWindow = (g_inputWindowMaxMs > 0) ? (int)g_inputWindowMaxMs : CONF_INPUT_WINDOW_MAX_DEFAULT_MS;
    }
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
  // Reset path validated like any other value — see setInputWindowShort (4.5.7 review).
  bool rangeOk = (inputWindowMs == 0) ||
                 (inputWindowMs >= CONF_INPUT_WINDOW_MAX_MIN_MS && inputWindowMs <= CONF_INPUT_WINDOW_MAX_MAX_MS);
  if (rangeOk && newEff > longEff && mem.writeInt(CONF_SETTINGS_FILE, commandKey, (int)inputWindowMs)) {
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
    if (mem.isLastReadOk()) {  // 4.9.1: live-apply truth gate — see getSapDisabled
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
        // factory mapping instead of loading the forbidden cells. The check
        // is column-aware (4.5.7 review): a gamepad-column DRAG/SCROLL cell
        // wedges the whole input path — those fall back too.
        if (!switchRemapAllowed(cells[0], gamepad) || !switchRemapAllowed(cells[1], gamepad) || !switchRemapAllowed(cells[2], gamepad)) {
          for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
        }
      } else {
        for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
      }
      for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) switchRemapApplyCell(jack, w, gamepad, cells[w]);
    } else {
      // 4.9.1: a failed read must not rewrite the live table (the old shape
      // fell into the stored-0 path and re-applied the compiled defaults
      // over the user's remap) — and the answer must be what is in force.
      for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapLiveCell(jack, w, gamepad);
    }
  } else {
    for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) cells[w] = switchRemapDefaultCell(jack, w, gamepad);
    for (int w = 0; w < CONF_SWITCH_REMAP_ROWS_PER_JACK; w++) switchRemapApplyCell(jack, w, gamepad, cells[w]);
  }
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
    // Column-aware allowlist (4.5.7 review): the gamepad columns reject
    // DRAG/SCROLL — mouse hold modes that wedge every input in gamepad
    // mode (see CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN in LSConfig.h).
    accepted = switchRemapAllowed(cells[0], gamepad) && switchRemapAllowed(cells[1], gamepad) && switchRemapAllowed(cells[2], gamepad);
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
        // 4.9.1: a safe-mode boot owns its menu (see activateMenu's gate) —
        // answer the refusal honestly instead of the SUCCESS-ack-for-a-drop
        // convention the transient frozen-window presses use: safe mode is
        // a STATE, not a transient, and a bare SUCCESS would tell the app
        // the full menu opened when the safe menu is still registered (the
        // CH,0/CH,5 poll would correct the mirror, but the wire should say
        // why). activateMenu itself also refuses — this is the named wire
        // half.
        if (g_safeModeEnabled) {
          printResponseInt(responseEnabled, apiEnabled, false, 3, "CH,1", true, inputMenuControl);
          serialTxEnqueueLine("ERROR: menu open refused — safe mode owns the display (use the Hub buttons: Restart / Factory Reset)");
          break;
        }
        screen.activateMenu();
        break;     
      case CONF_MENU_CONTROL_SELECT:
        g_menuInputHoldMs = 0xFFFFFFFFUL;  // serial presses bypass the hold-to-confirm gate (no hold concept on the wire)
        screen.selectMenuItem();
        break;
      case CONF_MENU_CONTROL_NEXT:
        screen.nextMenuItem();
        break;     
      case CONF_MENU_CONTROL_CLOSE:
        // Freeze-aware close (4.5.5 follow-up): a close arriving while a
        // scheduled transition or boot dwell page owns the display is
        // dropped — executing it early would flip the menu-active flag off
        // before the pending continuation registers its menu.
        screen.closeMenu();
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
//              32 mode-locked, 551 calib confirm, …), row (the SELECTABLE-row
//              index — the same index selectMenuItem() switches on; add the
//              page's _cursorStart offset for the text-array row).
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

//***BLUETOOTH HOSTS (upstream #16, firmware 4.8.0+)***//
// Function   : getBluetoothHost
//
// Description: The read half of multi-host switching. BT,0:0 answers the bond
//              count; BT,0:<i> answers row i (1-based, MAC-sorted — the same
//              order the Hub page shows) as
//              "SUCCESS,0:BT,0:<i>:<name>:<state>" where state 1 = this host
//              is the connected one. Names are sanitized by LSHostSwitch.h
//              (',' ':' '/' and non-printables -> '_') so the delimited wire
//              grammar can never be broken by a host's chosen Bluetooth name.
//
// Parameters :  responseEnabled / apiEnabled : the canonical response controls
//               optionalParameter : "0" (count) or a 1-based row index
//
// Return     : void
//*********************************//
void getBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  String commandKey = "BT,0";
  if (needsInitialization) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 0);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_NOT_INITIALIZED, 0);
    return;
  }
  lsHostRow rows[LSHOST_ENUM_CAP];
  const uint8_t count = lsHostEnumerate(rows, LSHOST_ENUM_CAP);
  const int row = optionalParameter.toInt();
  if (row < 0) {
    // Negative is out of contract ("0 = count, 1..n = row") — answer FAIL,3
    // like BT,1/BT,2 instead of silently treating it as the count read.
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, row);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_RANGE, row);
    return;
  }
  if (row <= 0) {
    printResponseInt(responseEnabled, apiEnabled, true, 0, commandKey, true, count);
    return;
  }
  if (row > count || row > LSHOST_ENUM_CAP) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, row);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_RANGE, row);
    return;
  }
  String rowValue;
  rowValue.reserve(strlen(rows[row - 1].name) + 10);
  rowValue += row;
  rowValue += ':';
  rowValue += rows[row - 1].name;
  rowValue += ':';
  rowValue += rows[row - 1].active;
  printResponseString(responseEnabled, apiEnabled, true, 0, commandKey, true, rowValue);
}

//***SWITCH BLUETOOTH HOST FUNCTION***//
// Function   : switchBluetoothHost
//
// Description: BT,1:<i> — disconnect the current host and (re)advertise so
//              host i reconnects (the Q4 sequence: disconnect, advertising
//              restarts via restartOnDisconnect, target reconnects on its own
//              schedule). NEVER resets the device and never touches OM/CM —
//              the deliberate-reset mode-switch contract is untouched. The
//              ack follows the ACTION (unlike MP's ack-then-reset, where the
//              reset would eat the response): a failed disconnect must be
//              able to answer FAIL,3. Connection/list pushes ride
//              btFeedbackLoop's transition detection.
//
// Parameters :  responseEnabled / apiEnabled : the canonical response controls
//               optionalParameter : the 1-based host row to switch to
//
// Return     : void
//*********************************//
void switchBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  String commandKey = "BT,1";
  if (needsInitialization) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 0);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_NOT_INITIALIZED, 0);
    return;
  }
  // The frozen-window gate is a serial-path concern (the menu path's own
  // selectMenuItem is already inputIsFrozen-guarded); lsHostExecuteSwitch
  // re-checks everything else at execution time (the TOCTOU gate).
  if (screen.inputIsFrozen()) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, optionalParameter.toInt());
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_FROZEN, optionalParameter.toInt());
    return;
  }
  const int targetRow = optionalParameter.toInt();
  // NULL identity: the serial index contract (the app re-reads BT,0 after
  // every BT,4 push; the menu paths pass the identities they rendered).
  // For the serial path, open the whitelist window too (bench B4; the menu
  // path goes through its own confirm flow, which also opens the window).
  lsHostSwitchOutcome oc;
  const int decision = lsHostExecuteSwitch(targetRow, NULL, &oc);
  if (decision != LSHOST_ALLOW) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, targetRow);
    lsHostRefusalLine(commandKey, decision, targetRow);
    return;
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, commandKey, true, targetRow);
  // Bench observability (2026-11-07, restructured by the 4.8.7 review): the
  // window's fate is NAMED on the wire — AFTER the response frame (the old
  // executor enqueued its INFO lines first, so an unsolicited INFO: preceded
  // the command's own ack). If the window opened, lsHostSwitchWindowTick
  // closes it when the target's identity connects or it times out; every
  // fallback already resumed open advertising. No blocking here — the wire
  // contract acks first, transitions follow via MANUAL,0:BT,5.
  if (oc.windowOpened) {
    serialTxEnqueueLine(String("INFO: BT,1 window opened (target row ") + targetRow
                        + " target=" + oc.wlText + " irk=" + oc.hasIrk + ")");
  } else if (oc.armFailed) {
    serialTxEnqueueLine("INFO: BT,1 window FALLBACK (arm failed) — open advertising");
  } else {
    serialTxEnqueueLine(String("INFO: BT,1 window FALLBACK (begin rc=0x")
                        + String(oc.beginRc, HEX) + " target=" + oc.wlText + " irk=" + oc.hasIrk
                        + " conn=" + g_switchWindowFailConn + " waited=" + g_switchWindowFailWaitedMs
                        + "ms) — open advertising");
  }
}

//***FORGET BLUETOOTH HOST FUNCTION***//
// Function   : forgetBluetoothHost
//
// Description: BT,2:<i> — remove one bond file (the host's stored keys). The
//              lib's reconnect path answers a missing LTK with a NULL
//              sec_info reply, so a forgotten host that still holds keys
//              re-pairs (Just Works) on its next connect — the asymmetry the
//              guides must document (bench step B6). Forgetting the ACTIVE
//              host disconnects it first. List change pushes BT,4 directly.
//
// Parameters :  responseEnabled / apiEnabled : the canonical response controls
//               optionalParameter : the 1-based host row to forget
//
// Return     : void
//*********************************//
void forgetBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  String commandKey = "BT,2";
  if (needsInitialization) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 0);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_NOT_INITIALIZED, 0);
    return;
  }
  const int targetRow = optionalParameter.toInt();
  const int decision = lsHostExecuteForget(targetRow, NULL);  // NULL identity: the serial index contract — see switchBluetoothHost
  if (decision != LSHOST_ALLOW) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, targetRow);
    lsHostRefusalLine(commandKey, decision, targetRow);
    return;
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, commandKey, true, targetRow);
  lsHostPushState(true);  // the list changed — the app learns immediately (BT,4, and BT,5 if the active host left); forced: a forget landing before the first poll pass still delivers its contractual push
}

//***PAIR NEW BLUETOOTH HOST FUNCTION***//
// Function   : pairNewBluetoothHost
//
// Description: BT,3:1 — make sure open advertising is running so a NEW host
//              can find and pair with the device (auto-accept Just Works,
//              unchanged posture). Refuses while a host is connected: a new
//              pairing needs advertising, advertising needs no connection.
//              The pair itself completes host-side; the resulting connect
//              transition pushes BT,5/BT,4 via btFeedbackLoop.
//
// Parameters :  responseEnabled / apiEnabled : the canonical response controls
//
// Return     : void
//*********************************//
void pairNewBluetoothHost(bool responseEnabled, bool apiEnabled) {
  String commandKey = "BT,3";
  if (needsInitialization) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 0);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_NOT_INITIALIZED, 0);
    return;
  }
  // Same calibration gate as BT,1/BT,2's executors: a pairing completing
  // mid-calibration is exactly the event churn those flows refuse to allow.
  // (No frozen-window gate: unlike BT,1, this action draws nothing — the
  // menu path's own pair-new runs under selectMenuItem's frozen gate.)
  if (g_calibrationRunning || g_centerRunning) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 1);
    lsHostRefusalLine(commandKey, LSHOST_REFUSE_CALIBRATION, 0);
    return;
  }
  if (Bluefruit.connected() != 0) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 1);
    serialTxEnqueueLine("ERROR: BT,3 refused — a host is connected; switch away first");
    return;
  }
  // A switch window owns the radio (4.9.0): its bouncer drops every connect
  // that is not the armed target, so a peer pair-new exists to admit would be
  // bounced for the window's remainder behind a SUCCESS ack. Refuse with the
  // named line; pair-new supersedes nothing — the pending switch keeps its
  // semantics, and the caregiver can retry after the window closes (60 s
  // bound) or after the switch completes.
  if (lsHostSwitchWindowOpen()) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 1);
    serialTxEnqueueLine("ERROR: BT,3 refused — a switch window is waiting for its target; retry in a moment");
    return;
  }
  // Honest ack (4.9.0): start() can refuse in the post-disconnect SD-teardown
  // window (bench 4.8.5/4.8.10) — acking SUCCESS while nothing is on air left
  // a new host with no device to find and the app believing pair-new armed.
  if (!lsHostEnsureAdvertising()) {
    printResponseInt(responseEnabled, apiEnabled, false, 3, commandKey, true, 2);
    serialTxEnqueueLine("ERROR: BT,3 refused — the radio would not start advertising (SD busy); retry in a moment");
    return;
  }
  printResponseInt(responseEnabled, apiEnabled, true, 0, commandKey, true, 1);
}

void pairNewBluetoothHost(bool responseEnabled, bool apiEnabled, String optionalParameter) {
  if (optionalParameter.length() == 1 && optionalParameter.toInt() == 1) {
    pairNewBluetoothHost(responseEnabled, apiEnabled);
  }
}

//***HOST-SWITCH EXECUTORS (shared by the serial endpoints and the Hub pages)***//
// Each re-enumerates and re-decides AT EXECUTION TIME — the TOCTOU gate (the
// 4.6.6 LK-confirm lesson): between a menu page rendering and its confirm
// select, hosts can pair/forget and a drag can start, so the row indices the
// caller captured are re-validated here against fresh enumeration. Returns
// LSHOST_ALLOW or the refusal reason (the same enum the pure layer uses).

int lsHostExecuteSwitch(int targetRow, const char* expectedMac12,
                        lsHostSwitchOutcome* out) {
  if (needsInitialization) return LSHOST_REFUSE_NOT_INITIALIZED;
  if (out != NULL) {
    out->windowOpened = false;
    out->armFailed = false;
    out->targetMac12[0] = '\0';
    out->addrType = -1;
    out->hasIrk = 0;
    out->beginRc = 0;
    out->wlText[0] = '\0';
  }
  lsHostRow rows[LSHOST_ENUM_CAP];
  const uint8_t count = lsHostEnumerate(rows, LSHOST_ENUM_CAP);
  int activeRow = 0;
  for (uint8_t i = 0; i < count && i < LSHOST_ENUM_CAP; i++) {
    if (rows[i].active) { activeRow = i + 1; break; }
  }
  lsHostSwitchState st;
  st.bleInitialized = 1;
  st.calibrationRunning = (g_calibrationRunning || g_centerRunning) ? 1 : 0;
  st.latchHeld = (outputAction == CONF_ACTION_DRAG || outputAction == CONF_ACTION_SCROLL) ? 1 : 0;
  st.menuInputFrozen = 0;  // the frozen-window gate is the caller's concern (selectMenuItem / the BT,1 handler)
  st.rowCount = count;
  st.activeRow = activeRow;
  const int decision = lsHostSwitchDecision(&st, targetRow);
  if (decision != LSHOST_ALLOW) return decision;
  // Row-identity check (4.8.1, lsHostRowIdentityMatches): a MAC-sorted table
  // renumbers in-range when the bond set changes between a page render and
  // this execution — the range check alone would hit the WRONG host. Menu
  // callers pass the identity they rendered; the serial path passes NULL
  // (the documented index contract, the app re-reads on every BT,4).
  if (targetRow >= 1 && targetRow <= (int)count
      && !lsHostRowIdentityMatches(&rows[targetRow - 1], expectedMac12)) {
    return LSHOST_REFUSE_RANGE;  // "Host list changed" — the same refusal shape as a stale index
  }
  // Disconnect ANY connected host, bonded or not: activeRow keys on the bond
  // file, so a connected-but-not-yet-bonded peer (the seconds-wide window
  // between CONNECTED and the lib's deferred bond save) used to skip the
  // disconnect entirely — the switch acked SUCCESS while the old host held
  // the link and the target could never take it. The switch's job is to free
  // the radio connection, whatever its bonding state.
  // Multi-host switching (upstream #16) — whitelist contingency (bench B4):
  // instead of resuming stock open advertising immediately, open a bounded
  // whitelist window that admits ONLY the target host (identity address +
  // IRK, read via the lib's bond loader). The just-disconnected host — which
  // used to race back in sub-second on macOS (B4: 0/5 away-from-Mac) — cannot
  // connect while the window is open. Sequence: CANCEL any stale window
  // (4.8.7 review: a second switch over an open window left the stale flag
  // armed with the old timestamp — its backstop later stop/restarted live
  // advertising and could report a phantom "target won"), ARM before the
  // disconnect, SUSPEND the stock disconnect auto-restart (4.8.7 review: the
  // DISCONNECTED handler's FP_ANY restart was a sub-second open window the
  // old host won — with it suspended NOTHING is on air until the window's
  // directed advertise), then BEGIN after the disconnect. Any SD refusal
  // disarms, restores the auto-restart and falls back to open advertising —
  // the pre-contingency behavior — never a wedge. btFeedbackLoop's
  // lsHostSwitchWindowTick closes the window only when the ARMED TARGET's
  // identity connects (a non-target is named on the wire, never reported as
  // a successful switch) or the bound elapses. The window's fate rides the
  // outcome struct — the wire INFO lines belong to the serial caller (post-
  // ack), not to this executor (the menu path emits no phantom BT,1 lines).
  lsHostCancelSwitchWindow();
  int wlAddrType = -1, wlHasIrk = 0;
  // 4.9.0: the arm runs for EVERY switch — the outcome struct is reporting
  // only. Coupling the arm to out != NULL meant a future caller that omitted
  // the outcome silently lost the B4 contingency (the pre-4.8.3 wrong-host
  // race, with no wire line to say so).
  const bool armed = lsHostArmSwitchWindow(rows[targetRow - 1].mac, &wlAddrType, &wlHasIrk);
  if (armed) lsHostSuspendAutoRestart();
  if (lsHostAnyConnected() && !lsHostDisconnectCurrent()) {
    if (armed) {
      lsHostRestoreAutoRestart();
      lsHostDisarmSwitchWindow();
    }
    return LSHOST_REFUSE_RADIO;
  }
  uint32_t beginRc = 0;
  const bool opened = armed && ((beginRc = lsHostBeginSwitchWindow()) == NRF_SUCCESS);
  if (out != NULL) {
    out->windowOpened = opened;
    out->armFailed = !armed;
    out->addrType = wlAddrType;
    out->hasIrk = wlHasIrk;
    out->beginRc = beginRc;
    strncpy(out->wlText, g_switchWlText, sizeof(out->wlText) - 1);
    out->wlText[sizeof(out->wlText) - 1] = '\0';
    if (opened) {
      strncpy(out->targetMac12, rows[targetRow - 1].mac, LSHOST_MAC_CHARS);
      out->targetMac12[LSHOST_MAC_CHARS] = '\0';
    }
  }
  if (!opened && !armed) {
    lsHostEnsureAdvertising();  // stock open advertising (idempotent when restartOnDisconnect already runs)
  }
  return LSHOST_ALLOW;
}

int lsHostExecuteForget(int targetRow, const char* expectedMac12) {
  if (needsInitialization) return LSHOST_REFUSE_NOT_INITIALIZED;
  lsHostRow rows[LSHOST_ENUM_CAP];
  const uint8_t count = lsHostEnumerate(rows, LSHOST_ENUM_CAP);
  int activeRow = 0;
  for (uint8_t i = 0; i < count && i < LSHOST_ENUM_CAP; i++) {
    if (rows[i].active) { activeRow = i + 1; break; }
  }
  lsHostSwitchState st;
  st.bleInitialized = 1;
  st.calibrationRunning = (g_calibrationRunning || g_centerRunning) ? 1 : 0;
  st.latchHeld = 0;        // a forget of an inactive host touches no radio state
  st.menuInputFrozen = 0;
  st.rowCount = count;
  st.activeRow = activeRow;
  const int decision = lsHostForgetDecision(&st, targetRow);
  if (decision != LSHOST_ALLOW) return decision;
  // Row-identity check — see lsHostExecuteSwitch. For the DESTRUCTIVE forget
  // this is the difference between "stale index refuses" and "renumbered
  // index erases the wrong host's bond".
  if (!lsHostRowIdentityMatches(&rows[targetRow - 1], expectedMac12)) {
    return LSHOST_REFUSE_RANGE;
  }
  // A forget supersedes any in-flight switch window (4.9.0): BT,2 has no
  // frozen gate and can land mid-window. Left open, the window kept the radio
  // directed at the just-forgotten bond — pair-new was dead for the window's
  // remainder (the bouncer dropped every new host), and the forgotten host
  // (keys intact on ITS side) could reconnect, match the armed bytes, and
  // close the window reporting a successful "switch" toward a bond that no
  // longer exists. Cancelling restores the stock radio state; the forget
  // then owns the world. Deliberately AFTER the decision gates: a REFUSED
  // forget must not disturb a window its caller never owned.
  // (4.9.0 review) cancel does NOT resume advertising (its other caller — a
  // new switch — re-owns the radio), and a forget of an INACTIVE host never
  // disconnects, so no DISCONNECTED event would restart advertising either:
  // the radio sat silently non-discoverable with no wire or OLED trace.
  // Resume open advertising here when nothing is connected — the connected
  // path keeps restartOnDisconnect's own post-disconnect restart.
  lsHostCancelSwitchWindow();
  if (!Bluefruit.connected()) lsHostEnsureAdvertising();
  if (rows[targetRow - 1].active && !lsHostDisconnectCurrent()) return LSHOST_REFUSE_RADIO;
  if (!lsHostForgetByMac(rows[targetRow - 1].mac)) return LSHOST_REFUSE_RADIO;
  return LSHOST_ALLOW;
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
    if (mem.isLastReadOk()) {  // heal a stored value only — never rewrite from a failed read (getModelNumber pattern)
      mem.writeInt(CONF_SETTINGS_FILE, commandKey, tempDebugMode);
    }
  }

  // NO setDebugState here (4.5.7 review): a READ re-applied the debug
  // timers — every DM,0 (the EX dump, the app's connect load) enabled the
  // joystick/scroll poll timers, undoing toggleSafeMode's shutdown for the
  // rest of a safe boot and re-enabling the poll a running calibration
  // deliberately disabled. The getter is a pure read; initDebug and
  // setDebugMode own the state change.

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
//              Every replayed getter is the identical call the app's
//              connect-time load makes (the documented heal/apply convention
//              applies: getters like CA,0 recompute radius/deadzone state
//              and heal out-of-range stored values exactly as a connect
//              load would), so this is safe to run at any time — no input,
//              screen or output state changes beyond those conventions.
//
//              4.8.1: the dump replays BT,0:0 (the exposed bond count) at
//              the end — the host table is bond state, not a setting, but a
//              multi-host support ticket needs the count; individual rows
//              are read with BT,0:<i> (the dump stays one call per
//              endpoint). Refused on a non-BLE boot the same way a direct
//              BT,0 is.
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
  // Upstream #58: the hard-band keys live in /settings2.dat (the getters
  // read their own file). Like EVERY getter this frame replays, these are
  // the app's connect-load convention, not pure reads: an out-of-range or
  // margin-violating stored value heals (and its MANUAL/SUCCESS lines ride
  // the same frame) — a failed read answers the compiled default and the
  // heal write is refused (the read-truth doctrine).
  getHardSipPressureThreshold(responseEnabled, apiEnabled);
  getHardPuffPressureThreshold(responseEnabled, apiEnabled);
  getHardSipAction(responseEnabled, apiEnabled);
  getHardPuffAction(responseEnabled, apiEnabled);
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
  // 4.8.1: the exposed bond count (BT,0:0) rides the dump — the app's own
  // connect load re-reads the rows individually with BT,0:<i> right after,
  // so the frame only needs the count to notice "hosts exist".
  getBluetoothHost(responseEnabled, apiEnabled, "0");

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
bool resetSettings(bool responseEnabled, bool apiEnabled) {
  // Reset only the settings document (unlike FR, this does not format the
  // whole filesystem). One atomic replacement avoids the factory-reset
  // chain's many intermediate states. Internal migration callers pass
  // apiEnabled=false and continue booting; the serial wrapper below resets
  // after the acknowledgement so every live subsystem reloads the defaults.
  bool resetOk = mem.writeAll(CONF_SETTINGS_FILE, CONF_SETTINGS_JSON);
  printResponseInt(responseEnabled, apiEnabled, resetOk, resetOk ? 0 : 3, "RS,1", true, resetOk ? 0 : 1);
  return resetOk;
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
    if (resetSettings(responseEnabled, apiEnabled) && apiEnabled) {
      softwareReset();
    }
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

  // Exempt the whole reset chain from the hang witness (4.5.7 review): the
  // reset work below (filesystem format + ONE atomic default-settings seed —
  // the pre-4.5.7 shape was ~22 sequential per-key writes, which is why this
  // exemption exists) can cross the witness's 3 s
  // loop-stall threshold on slow flash — and the chain unconditionally ends
  // in softwareReset(), which sets this flag anyway; setting it here covers
  // the writes with the same deliberate-shutdown semantics. A witness SREQ
  // mid-reset would abort it with the FS formatted and most defaults never
  // applied.
  g_shutdownInProgress = true;

  // Set all LEDs to red to indicate factory reset process
  setLedState(LED_ACTION_ON, LED_CLR_RED, CONF_LED_ALL, 0, 0, CONF_LED_BRIGHTNESS_MAX);
  performLedAction(ledCurrentState);

  // Factory reset process 
  if (USB_DEBUG) Serial.println("FACTORY RESET TRIGGERED");
  
  bool resetOk = resetMemory();  // Format and atomically seed one complete default settings document
  printResponseInt(responseEnabled, apiEnabled, resetOk, resetOk ? 0 : 3, "FR,1", true, resetOk ? 1 : 0);

  // Clear all LEDs to indicate factory reset process is finished 
  setLedState(LED_ACTION_OFF, LED_CLR_NONE, CONF_JOY_CALIB_LED_NUMBER, 0, 0, CONF_LED_BRIGHTNESS_MAX);                           
  performLedAction(ledCurrentState);  

  if (USB_DEBUG) Serial.println("Activating software reset");

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
  // 4.6.6: inside the Bluefruit.begin() window the SD may already be enabled
  // while g_softDeviceEnabled is still false — a raw GPREGRET write here
  // (either the SVC bypass below or the core's enterSerialDfu path) would be
  // an APP_MEMACC kill. The window closes the moment begin() returns
  // (sub-second); UM arrives via serial, which cannot be served before the
  // CDC stack is up long after the window in practice — wait it out bounded
  // rather than faulting the boot.
  {
    const unsigned long umWaitDeadlineMs = millis() + 2000;
    while (lipsyncRetentionUnsafe() && (long)(umWaitDeadlineMs - millis()) > 0) {
      delay(10);
    }
  }
  if (lipsyncRetentionUnsafe()) {
    // 4.9.1: fail SAFE, not fail open. Past the deadline the SD state is
    // still ambiguous (inside the begin() window with the flag not yet
    // raisable) — the g_softDeviceEnabled branch below would take the raw
    // enterSerialDfu() path and fault (APP_MEMACC), the fault handler's
    // GPREGRET overwrite would clobber the magic, and the device would
    // reboot into the app while Connect waits for a bootloader that never
    // comes. Name the refusal; a retry once the init settles answers again.
    printResponseInt(responseEnabled, apiEnabled, false, 3, "UM,1", true, 1);
    serialTxEnqueueLine("ERROR: update mode refused — the BLE init window is still open; retry in a moment");
    return;
  }
  // With the SoftDevice live, prepare the DFU retention magic through its
  // sanctioned SVCs before promising success. The old code ignored both
  // return values, acknowledged, and reset even if the magic was not stored
  // — which simply rebooted back into the app while Connect waited for a
  // bootloader that never appeared.
  if (g_softDeviceEnabled) {
    uint32_t clearRc = sd_power_gpregret_clr(0, 0xFF);
    uint32_t setRc = (clearRc == NRF_SUCCESS)
                   ? sd_power_gpregret_set(0, 0x4E)  // DFU_MAGIC_SERIAL_ONLY_RESET
                   : clearRc;
    if (clearRc != NRF_SUCCESS || setRc != NRF_SUCCESS) {
      printResponseInt(responseEnabled, apiEnabled, false, 3, "UM,1", true, 1);
      return;
    }
  }

  // Ack BEFORE anything else: the reset below kills USB, and the host must
  // already hold the confirmation when the port departs. The bounded dwell
  // then gives the ack time to reach the host and the splash's framebuffer
  // push time to complete before the OLED loses its driver.
  //
  // 4.9.4 (round 5, the ownership sweep continued): close any OPEN SWITCH
  // WINDOW before the reset. A window holds live radio state that a reset
  // does not unwind cleanly — g_switchWindowOpen true, the stock
  // DISCONNECTED auto-restart SUSPENDED (lsHostSuspendAutoRestart), an armed
  // target IRK and a bounce counter — and the window's own deferred-
  // advertising-restart flag (g_switchAdvRestartPending) is armed for a
  // radio that is about to vanish. Worse, the window's tick and the DFU
  // magic write both own the retention registers: the window path writes
  // GPREGRET2 while enterUpdateMode writes GPREGRET via the DFU magic, and a
  // bounce landing in the dwell could enqueue INFO lines over the ack's
  // frame. Closing the window first restores the stock radio state
  // (auto-restart re-armed, open advertising resumed when nothing is
  // connected) so the reset happens from exactly the state a normal UM does.
  // Deliberately BEFORE the ack: a close that resumed advertising is
  // invisible to the host, but an ack followed by radio work could interleave
  // on the wire.
  if (lsHostSwitchWindowOpen()) {
    lsHostCloseSwitchWindow();
  }

  printResponseInt(responseEnabled, apiEnabled, true, 0, "UM,1", true, 1);

  if (g_displayConnected) {
    screen.updateModePage();
  }

  // 4.9.4: the dwell must DRAIN the response queue — the same fix 4.9.1 gave
  // softwareReset()'s dwell, back-ported to the one reset path it missed. The
  // UM,1 ack above rides the TX queue like every other response: with a host
  // that stopped reading the IN pipe (or a full CDC FIFO), serialTxService
  // wrote 0 bytes and the reset below killed the queue with the ack still in
  // it — the port dropped with no response, and the app cannot distinguish
  // "entering the bootloader" from a refused/dead command (its update flow
  // keys the bootloader hand-off on this ack). Servicing every 10 ms hands
  // the queue the whole dwell.
  {
    const unsigned long updateDwellDeadlineMs = millis() + CONF_UPDATE_MODE_DWELL_MS;
    while ((long)(updateDwellDeadlineMs - millis()) > 0) {
      serialTxService();
      delay(10);
    }
  }

  // THE ARMED WATCHDOG SURVIVES THE RESET (4.9.0 — the DFU class): the WDT
  // cannot be stopped and survives warm resets (the inherited-instance feed
  // at boot is built on exactly that), and the stock bootloader never feeds
  // it. Without this reload the bootloader inherited a PARTIALLY-CONSUMED
  // budget (up to 30 s minus command processing); the reload hands it a
  // fresh full CONF_WATCHDOG_TIMEOUT_SEC. THE HONEST CEILING (4.9.0 review
  // correction): the ceiling CANNOT be extended here — nRF52 PS, WDT:
  // "After it is started, the watchdog's configuration registers, which
  // comprise registers CRV, RREN, and CONFIG, will be blocked for further
  // configuration." A CRV write while running is silently ignored, so the
  // bootloader's budget is at most one full 30 s. That covers the
  // production update path (LipSync Connect streams the image immediately
  // after the UM ack + 1200-baud touch; the transfer is a few seconds over
  // USB CDC); the serial-monitor manual path (SETTINGS + UM,1:1, then a
  // hand-driven flash) must begin within ~30 s — documented, not silently
  // claimed. A true extension would need a bootloader-side feed (a separate
  // vendored binary, out of scope) — do NOT re-add a CRV write assuming it
  // works.
  if (NRF_WDT->RUNSTATUS != 0) {
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
  }

  // 4.5.27 (Fable 5.1 consult, hazard 1): the core's enterSerialDfu() writes
  // the DFU magic to NRF_POWER->GPREGRET RAW (wiring.c:85) — a POWER write
  // the SoftDevice's watch unit flags when the stack is up, and the fault
  // handler's own GPREGRET overwrite would clobber the magic, rebooting into
  // the app instead of the bootloader. With the SD up, set the magic through
  // the sanctioned SVCs and reset ourselves; with it down (the normal
  // USB-mode update flow), the core's path is exactly right.
  if (g_softDeviceEnabled) {
    NVIC_SystemReset();
  } else {
    enterSerialDfu();
  }
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
    String line;
    line.reserve(responseCommand.length() + responseParameter.length() + 24);
    if (responseStatus) {
      line += apiEnabled ? "SUCCESS" : "MANUAL";
    } else {
      line += "FAIL";
    }
    line += ',';
    line += String(responseNumber);
    line += ':';
    line += responseCommand;
    if (responseParameterEnabled) {
      line += ':';
      line += responseParameter;
    }
    serialTxEnqueueLine(line);
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

// (printResponseIntPoint and printResponseIntPointArray REMOVED — 4.8.1
// review: dead code, zero call sites; each was a second wire format waiting
// to drift from its float twin, the readAll rationale. Restore only with a
// real consumer.)

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
