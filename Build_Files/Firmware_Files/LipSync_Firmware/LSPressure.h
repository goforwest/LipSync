/* 
* File: LSPressure.h
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
#ifndef _LSPRESSURE_H
#define _LSPRESSURE_H

#include <Adafruit_LPS35HW.h>   // Tube pressure sensor
#include <Adafruit_LPS2X.h>     // Ambient pressure sensor
#include <Adafruit_Sensor.h>     // Generic sensor

#define LPS22_I2CADDR 0x5C      // Modified LPS22 address

#define PRESS_BUFF_SIZE 5       // The size of pressure Buffer
#define PRESS_SAP_BUFF_SIZE 12  // The size of sip and puff state buffer 

#define PRESS_REF_TOLERANCE 0.1   // The change in reference pressure (hPa) that would initiate reference pressure update 
                                  // It's only used in differential mode

#define PRESS_SAP_DEFAULT_THRESHOLD 3.0 // hPa hecto Pascals

#define PRESS_FILTER_NONE 0
#define PRESS_FILTER_AVERAGE 1

#define PRESS_MODE_NONE 0
#define PRESS_MODE_ABS 1              // Absolute pressure mode
#define PRESS_MODE_DIFF 2             // Differential pressure mode

#define PRESS_MODE_MIN 1              // Minimum and maximum mode numbers 
#define PRESS_MODE_MAX 2              // Used for switching pressure mode

// Sip and puff secondary states 
#define PRESS_SAP_SEC_STATE_WAITING 0   // Waiting : No sip or puff
#define PRESS_SAP_SEC_STATE_STARTED 1   // Started : Sip or puff being performed  (Off->On)
#define PRESS_SAP_SEC_STATE_RELEASED 2  // Released : Sip or puff was just released (On->Off)

#define PRESS_SAP_ACTION_TIMEOUT 60000  // Reset timer

#define PRESS_BAD_READ_LIMIT 3         // Consecutive invalid readings before the sensors are marked unhealthy
#define PRESS_REPROBE_INTERVAL_MS 2000  // Spacing between sensor re-probes while unhealthy (keeps a dead
                                        // or flaky sensor from stalling the main loop on every poll)
#define PRESS_PROBE_GOOD_LIMIT 2        // Consecutive valid re-probes before the sensors are marked healthy again
#define PRESS_OFFSET_ATTEMPT_LIMIT 10   // updatePressure() ticks spent gathering offset samples before giving up

// Pressure structure 
typedef struct {
  float sapPressureAbs;                 // Stand-alone I2C Sensor reading [hPa]
  float ambientPressure;                // Ambient Pressure [hPa]
  float sapPressure;                    // Pressure difference used in sip and puff processing [hPa]
} pressureStruct;

extern bool g_mouthpiecePressureSensorConnected;  // Mouthpiece pressure sensor connection state
extern bool g_ambientPressureSensorConnected;     // Ambient pressure sensor connection state


class LSPressure {
  public:
    LSPressure();                                       // Constructor
    void begin();
    void clear();
    void update();                                      // Update the pressure buffer and sip and puff buffer with new readings
    void setPressureMode(int mode);                     // Set Sip and Puff Pressure Mode : ABS or DIFF pressure
    int getPressureMode();                              // Get Sip and Puff Pressure Mode : ABS or DIFF pressure
    void setRefTolerance(float value);                  // Set reference pressure change tolerance value that is used to update the reference pressure
    float getRefTolerance();                            // Get reference pressure change tolerance value that is used to update the reference pressure
    float getOffsetPressure();                          // Get Offset pressure: (sapPressureAbs- ambientPressure)
    void setOffsetPressure(float offsetPressure);       // Set Offset pressure: 
    void startOffsetUpdate();                           // Begin a stepped offset measurement: five valid (sap - ambient)
                                                        // samples gathered one per update() tick, then averaged
    void setSipThreshold(float s);                      // Set sip threshold
    void setPuffThreshold(float p);                     // Set puff threshold
    void updatePressure();                              // Update the pressure buffer with new readings 
    void updateState();                                 // Update the and puff buffer with new states 
    float getSapPressureAbs();                          // Get last main pressure from pressure buffer
    float getAmbientPressure();                         // Get last reference pressure from pressure buffer
    float getSapPressure();                             // Get the Pressure Difference sapPressure = (sapPressureAbs- ambientPressure- offsetPressure)
    pressureStruct getAllPressure();                    // Get the latest pressure values
    inputStateStruct getState();                        // Get the latest sip and puff state
    bool isSensorsHealthy();                            // True while both pressure sensors return valid readings (false = gated: re-probing slowly, sip/puff cannot fire)

  private: 
      Adafruit_LPS35HW _lps35hw = Adafruit_LPS35HW();     // Create an object of Adafruit_LPS35HW class for Sip and Puff pressure
      Adafruit_LPS22 _lps22;                              // Create an object of Adafruit_LPS2X class for ambient pressure
      sensors_event_t _lps22Pressure;                     // Ambient temperature event object 
      sensors_event_t _lps22Temperature;                  // Ambient pressure event object                           
      LSCircularBuffer <pressureStruct> _pressureBuffer;  // Create a buffer of type pressureStruct to push pressure readings
      LSCircularBuffer <inputStateStruct> _sapBuffer;     // Create a buffer of type inputStateStruct to push sap states
      int _pressureMode = PRESS_MODE_DIFF;                // Pressure Mode: DIFF or ABS pressure
      float _sapPressureAbs = 0.0f;                       // Main Pressure reading (Sip and Puff Absolute) [hPa]
      float _ambientPressure = 0.0f;                      // Reference Pressure reading (Ambient Absolute) [hPa]
      float _offsetPressure = 0.0f;                       // Offset Pressure [hPa] (Difference between two sensors)
      float _sapPressure = 0.0f;                          // Calculated Pressure Difference sapPressure = (sapPressureAbs- ambientPressure- offsetPressure)
      float _refTolVal = PRESS_REF_TOLERANCE;             // The tolerance value in hPa used to check and update reference pressure
      LSTimer <void> _sapStateTimer;                      // Timer used to measure time for each sip and puff action. 
      int _sapStateTimerId = -1;                           // The id for the sap state timer
      inputStateStruct _sapCurrState = {PRESS_SAP_MAIN_STATE_NONE, PRESS_SAP_SEC_STATE_WAITING, 0};
      inputStateStruct _sapPrevState = {PRESS_SAP_MAIN_STATE_NONE, PRESS_SAP_SEC_STATE_WAITING, 0};
      float _sipThreshold = PRESS_SAP_DEFAULT_THRESHOLD;   // Sip Threshold
      float _puffThreshold = PRESS_SAP_DEFAULT_THRESHOLD;  // Puff Threshold
      int _sapMainState = PRESS_SAP_MAIN_STATE_NONE;       // The value which represents the current main state (example: PRESS_SAP_MAIN_STATE_PUFF)
      bool _sensorsHealthy;                               // Both sensors return valid readings (false until begin() correlates it with the sensor-detection globals — see the constructor)
      bool _begun;                                        // begin() ran, so the Adafruit driver objects are safe to call. A never-begun LPSressure holds NULL i2c_dev pointers and ANY driver call faults (bench 2026-10-28: PV,0:0 on a missing-sensor boot HardFaulted inside Adafruit_I2CDevice::write_then_read — pm=2 pt=7 fp=0xE000, twice). Every driver I/O this class makes is behind this gate.
      unsigned long _lastProbeMs;                         // Timestamp of the last unhealthy-mode re-probe
      uint8_t _badReadCount;                              // Consecutive invalid readings while healthy
      uint8_t _goodProbeCount;                            // Consecutive valid re-probes while unhealthy
      bool _offsetUpdateRunning;                          // Offset measurement in progress (stepped by updatePressure())
      uint8_t _offsetAttempts;                            // Ticks spent gathering offset samples
      uint8_t _offsetValidCount;                          // Valid offset samples gathered
      float _offsetSampleSum;                             // Running sum of valid (sap - ambient) samples
      void markSensorsUnhealthy();                        // Disable sip and puff and stop reading I2C until a re-probe succeeds
};


//*********************************//
// Function   : LSPressure 
// 
// Description: Construct LSPressure
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
LSPressure::LSPressure()
{
  _pressureBuffer.begin(PRESS_BUFF_SIZE);        // Initialize Pressure Buffer
  _sapBuffer.begin(PRESS_SAP_BUFF_SIZE);         // Initialize Sip and Puff Action Buffer 
  // 4.6.1: both start false. The old `_sensorsHealthy = true` default was
  // half of the PV crash: begin() is the only code that correlates this
  // flag with the sensor-detection globals, and on a missing-sensor boot
  // initSipAndPuff() never runs — so the object stayed "healthy" forever
  // with drivers that were never begin_I2C()'d, and the first ps.update()
  // (PV,0:0, or DEBUG,1:2) took the healthy read path straight into the
  // NULL i2c_dev chain. An object that has not begun cannot be healthy.
  _sensorsHealthy = false;
  _begun = false;
  _lastProbeMs = 0;
  _badReadCount = 0;
  _goodProbeCount = 0;
  _offsetUpdateRunning = false;
  _offsetAttempts = 0;
  _offsetValidCount = 0;
  _offsetSampleSum = 0.00;
}

//*********************************//
// Function   : begin 
// 
// Description: Initialize LSPressure with default settings 
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSPressure::begin()
{
  // lps35hw Pressure sensor setup
  if (!_lps35hw.begin_I2C())
  {
    Serial.println(F("ERROR: Couldn't find LPS35HW chip"));  // TODO 2025-Feb-21 This should also generate an error
    g_mouthpiecePressureSensorConnected = false;
  }
  else
  {
    if (USB_DEBUG) {Serial.println("USBDEBUG: LSPressure::begin LPS35HW Found."); }
    _lps35hw.setDataRate(LPS35HW_RATE_25_HZ);  // Options: 1 Hz, 10Hz, 25Hz, 50Hz, 75Hz
    g_mouthpiecePressureSensorConnected = true;
  }

  // LPS22 Pressure sensor setup
  if (!_lps22.begin_I2C(LPS22_I2CADDR))
  {
    Serial.println(F("ERROR: Couldn't find LPS22 chip"));  // TODO 2025-Feb-21 This should also generate an error
    g_ambientPressureSensorConnected = false;
  } 
  else
  {
    if (USB_DEBUG) {Serial.println("USBDEBUG: LSPressure::begin LPS22 Found.");}
    _lps22.setDataRate(LPS22_RATE_25_HZ);         // Options: 1-shot,
    g_ambientPressureSensorConnected = true;
  }

  //setFilterMode(PRESS_FILTER_NONE);         // Set the default filter mode to none  //  TODO 2025-Feb-22 Currently unused
  setPressureMode(PRESS_MODE_DIFF);         // Set the default pressure mode to differential mode (i.e., mouthpiece and ambient)
  setRefTolerance(PRESS_REF_TOLERANCE);     // Set the default tolerance value to update reference pressure //TODO 2025-Feb-22 No longer used?
  
  // Set default pressure thresholds
  setSipThreshold(PRESS_SAP_DEFAULT_THRESHOLD);
  setPuffThreshold(PRESS_SAP_DEFAULT_THRESHOLD);

  //delay(5);
  //_lps35hw.setDataRate(LPS35HW_RATE_25_HZ);  // Options: 1 Hz, 10Hz, 25Hz, 50Hz, 75Hz
  //_lps22.setDataRate(LPS22_RATE_25_HZ);      // Options: 1-shot, 

  clear();      // Clear buffers and make sure no sip and puff action is set as previous actions

  delay(20);
  _sensorsHealthy = (g_mouthpiecePressureSensorConnected && g_ambientPressureSensorConnected);
  startOffsetUpdate();  // Gather offset samples one per update() tick. The old boot-time offset
                        // read was a blocking 5-reading loop that could freeze startup for up to
                        // 15 s when a sensor misbehaved; until it completes, the offset is 0 and
                        // sip and puff stay inert.
  _begun = true;         // Drivers are now safe (i2c_dev allocated even on a failed begin_I2C) —
                        // sets the gate update() checks before any driver I/O
}

//*********************************//
// Function   : clear 
// 
// Description: Clear pressure buffer and restart state timer
// 
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSPressure::clear()
{
  _pressureBuffer.clear();
  while(_pressureBuffer.getLength() < PRESS_BUFF_SIZE)
  {    
    _pressureBuffer.pushElement({0.0, 0.0, 0.0});  // Clear Pressure Buffer by pushing zero readings to buffers 
  }

  // Push initial state to state Queue
  _sapBuffer.clear();
  _sapCurrState = _sapPrevState = {PRESS_SAP_MAIN_STATE_NONE, PRESS_SAP_SEC_STATE_WAITING, 0};
  _sapBuffer.pushElement(_sapCurrState);

  // Reset and start the timer   
  if (_sapStateTimerId >= 0) _sapStateTimer.deleteTimer(_sapStateTimerId);
  _sapStateTimerId = _sapStateTimer.startTimer();

}

// setFilterMode/_filterMode removed (4.5.7 review): dead since 4.2.0 — the
// mode was stored and never read (the PRESS_FILTER_* defines remain
// documented above for the differential-mode semantics).

//*********************************//
// Function   : getPressureMode 
// 
// Description: Get the pressure mode ( ABS = 1 or DIFF = 2)
// 
// Arguments :  void
// 
// Return     : mode : int : Each pressure mode has a number 
//*********************************//
int LSPressure::getPressureMode()
{
  return _pressureMode;
}

//*********************************//
// Function   : setPressureMode 
// 
// Description: Set the pressure mode ( ABS = 1 or DIFF = 2)
// 
// Arguments :  mode : int : Each pressure mode has a number 
// 
// Return     : void
//*********************************//
void LSPressure::setPressureMode(int mode)
{
  _pressureMode = mode;
}

//*********************************//
// Function   : setRefTolerance 
// 
// Description: Set the reference pressure change tolerance (hPa)
// 
// Arguments :  value : float : The pressure change in hPa
// 
// Return     : void
//*********************************//
void LSPressure::setRefTolerance(float inputrefTolVal)
{
  _refTolVal = inputrefTolVal;
}

//*********************************//
// Function   : getRefTolerance 
// 
// Description: Get the reference pressure change tolerance (hPa)
// 
// Arguments :  void
// 
// Return     : value : float : The pressure change in hPa
//*********************************//
float LSPressure::getRefTolerance()
{
  return _refTolVal;
}

//*********************************//
// Function   : startOffsetUpdate 
// 
// Description: Begin a stepped offset-pressure measurement. Five valid
//              (sap - ambient) samples are gathered one per update() tick by
//              updatePressure() and then averaged. This replaces the old
//              blocking 5-reading loop, which spun up to 3 s per reading on
//              a misbehaving sensor (15 s of frozen main loop per
//              re-measure, at boot and once a minute while idle).
//              
// Arguments :  void
// 
// Return :    void
//*********************************//
void LSPressure::startOffsetUpdate()
{
  if (_offsetUpdateRunning) return;  // Already gathering samples

  _offsetUpdateRunning = true;
  _offsetAttempts = 0;
  _offsetValidCount = 0;
  _offsetSampleSum = 0.00;
}

//*********************************//
// Function   : markSensorsUnhealthy 
// 
// Description: Stop trusting the pressure sensors: disable sip and puff so
//              stale buffer values can never fire an action, and leave the
//              I2C bus alone except for a slow re-probe (a flaky sensor can
//              no longer stall the poll loop; a wedged bus hangs at most one
//              transaction, which the watchdog recovers).
//              
// Arguments :  void
// 
// Return :    void
//*********************************//
void LSPressure::markSensorsUnhealthy()
{
  _sensorsHealthy = false;
  _badReadCount = 0;
  _goodProbeCount = 0;
  _lastProbeMs = millis();
  _sapMainState = PRESS_SAP_MAIN_STATE_NONE;  // Stale data must never trigger sip or puff
  Serial.println(F("ERROR: Pressure sensors not responding — sip and puff disabled until they recover"));
}

//*********************************//
// Function   : getOffsetPressure 
// 
// Description: Return the offset pressure (hPa)

// Arguments :  void
// 
// Return     : tempOffsetPressure : float : The offset pressure value in hPa
//*********************************//
float LSPressure::getOffsetPressure()  
{
  return _offsetPressure;  // Return offset value
}

//*********************************//
// Function   : setOffsetPressure 
// 
// Description: Set the offset pressure value (hPa)
//              
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSPressure::setOffsetPressure(float offsetPressure)
{
  _offsetPressure = offsetPressure;
}

//*********************************//
// Function   : setSipThreshold 
// 
// Description: Set sip pressure threshold in hPa
// Arguments :  s : float : Sip pressure threshold
// 
// Return     : void
//*********************************//
void LSPressure::setSipThreshold(float s)
{
  _sipThreshold = s;
}

//*********************************//
// Function   : setPuffThreshold 
// 
// Description: Set puff pressure threshold in hPa
//
// Arguments :  p : float : Puff pressure threshold
// 
// Return     : void
//*********************************//
void LSPressure::setPuffThreshold(float p)
{
  _puffThreshold = p;
}

//*********************************//
// Function   : update 
// 
// Description: Update pressure buffer , and sip&puff buffer
//
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSPressure::update()
{
  // Never begun = no driver I/O of any kind (the _begun gate). The getters
  // still answer their member values (zeros), so a PV,0:0 on a degraded
  // boot reports honestly instead of faulting; updateState() is skipped
  // too — there is no live reading to run a state machine on.
  if (!_begun) return;
  updatePressure();
  updateState();
}

//*********************************//
// Function   : updatePressure 
// 
// Description: Update pressure buffer
//
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSPressure::updatePressure()
{
  unsigned long nowMs = millis();

  // Unhealthy: leave the I2C bus alone except for a slow re-probe. A dead or
  // flaky sensor can no longer stall this loop; recovery is automatic once
  // the sensor answers again.
  if (!_sensorsHealthy) {
    if (nowMs - _lastProbeMs < PRESS_REPROBE_INTERVAL_MS) return;
    _lastProbeMs = nowMs;
    float probeAbs = _lps35hw.readPressure();
    _lps22.getEvent(&_lps22Pressure, &_lps22Temperature);
    // Plausibility range, not just "> 0" (4.5.7 review): both sensors read
    // absolute ~1013 hPa — a garbage-positive pair (the flaky-ACK failure
    // mode the bench has recorded on unseated cables) passed the old test
    // as "valid" and could fire phantom sip/puff events.
    if (probeAbs > CONF_PRESS_ABS_PLAUSIBLE_MIN && probeAbs < CONF_PRESS_ABS_PLAUSIBLE_MAX
        && _lps22Pressure.pressure > CONF_PRESS_ABS_PLAUSIBLE_MIN && _lps22Pressure.pressure < CONF_PRESS_ABS_PLAUSIBLE_MAX) {
      if (++_goodProbeCount >= PRESS_PROBE_GOOD_LIMIT) {
        _sensorsHealthy = true;
        _goodProbeCount = 0;
        _badReadCount = 0;
        startOffsetUpdate();  // Offsets drift while the sensors were down — re-measure
      }
    } else {
      _goodProbeCount = 0;
    }
    return;
  }

  _sapPressureAbs = _lps35hw.readPressure();        // Update mouthpiece pressure value

  // If pressure mode is differential
  float tempAmbientPressure = 0.00;
  bool ambientReadingValid = (_pressureMode != PRESS_MODE_DIFF);
  if(_pressureMode == PRESS_MODE_DIFF) {
    _lps22.getEvent(&_lps22Pressure, &_lps22Temperature);
    tempAmbientPressure = _lps22Pressure.pressure;  // Set a temporary reference value to new reference pressure reading

    if(tempAmbientPressure > CONF_PRESS_ABS_PLAUSIBLE_MIN && tempAmbientPressure < CONF_PRESS_ABS_PLAUSIBLE_MAX) { // If ambient pressure is plausible, update the ambient pressure value
      _ambientPressure = tempAmbientPressure;
      ambientReadingValid = true;
    }
  }

  // Make sure pressure readings are valid (plausibility range — see the re-probe above)
  bool validReading = (_sapPressureAbs > CONF_PRESS_ABS_PLAUSIBLE_MIN && _sapPressureAbs < CONF_PRESS_ABS_PLAUSIBLE_MAX
                       && ambientReadingValid);

  // Stepped offset measurement: this tick's reading IS the sample, so the old
  // 5-read blocking re-measure (up to 15 s of frozen main loop on a bad
  // sensor) costs one I2C transaction per tick instead.
  if (_offsetUpdateRunning) {
    _offsetAttempts++;
    if (validReading) {
      if (_pressureMode == PRESS_MODE_DIFF) {
        _offsetSampleSum += _sapPressureAbs - _ambientPressure;
      } else {  // Absolute mode: offset is zero and the reading is its own reference
        _ambientPressure = _sapPressureAbs;
      }
      _offsetValidCount++;
    }
    if (_offsetValidCount >= PRESS_BUFF_SIZE || _offsetAttempts >= PRESS_OFFSET_ATTEMPT_LIMIT) {
      _offsetUpdateRunning = false;
      if (_offsetValidCount > 0) {
        setOffsetPressure(_offsetSampleSum / _offsetValidCount);
      } else {
        markSensorsUnhealthy();  // Nothing valid came back — stop hammering the bus
      }
    }
  }

  if (validReading) {
    _badReadCount = 0;
    _sapPressure = _sapPressureAbs - _ambientPressure - _offsetPressure;              // Calculate the pressure difference 
    _pressureBuffer.pushElement({_sapPressureAbs, _ambientPressure, _sapPressure});   // Push new pressure values to pressure buffer 
  } else if (++_badReadCount >= PRESS_BAD_READ_LIMIT) {
    markSensorsUnhealthy();  // A few consecutive bad readings — stop reading I2C and re-probe slowly
  }
}

//*********************************//
// Function   : updateState 
// 
// Description: Update the sip and puff state buffer
//
// Arguments :  void
// 
// Return     : void
//*********************************//
void LSPressure::updateState()  //  TODO 2025-Feb-22 This code should be abstracted out as inputHandler as it is repeated for buttons and switches
{
  _sapStateTimer.run();                       // Update main state timer
  _sapPrevState = _sapBuffer.getLastElement();  // Get the previous state

  float pressureValue = getSapPressure();    // Get latest pressure value

  // check for sip and puff conditions
  if (!_sensorsHealthy) {
    _sapMainState = PRESS_SAP_MAIN_STATE_NONE; // Never act on readings from unhealthy sensors
  } else if (_offsetUpdateRunning) {
    // Boot/re-probe offset gather (4.5.7 review): until the offset lands,
    // sapPressure is the RAW inter-sensor offset — exactly the quantity the
    // offset exists to cancel — and on a device whose raw offset exceeds a
    // low user threshold the old code entered PUFF/SIP STARTED on the first
    // tick, then the offset landed ~250 ms later, the pressure collapsed,
    // and the RELEASED fired the short-window action (a phantom click a few
    // hundred ms after boot). "Until it completes, sip and puff stay inert"
    // is now actually true.
    _sapMainState = PRESS_SAP_MAIN_STATE_NONE;
  } else if (pressureValue > _puffThreshold) {
    _sapMainState = PRESS_SAP_MAIN_STATE_PUFF; // Puff detected
  }
  else if (pressureValue < -1 * _sipThreshold) {

    _sapMainState = PRESS_SAP_MAIN_STATE_SIP; // Sip detected
  }
  else if (_sapPrevState.mainState == PRESS_SAP_MAIN_STATE_PUFF
           && _sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED
           && pressureValue > _puffThreshold * CONF_PRESS_RELEASE_FRACTION) {
    // Release hysteresis (4.5.7 review): the engage boundary used to be the
    // release boundary, so sensor noise while holding near a threshold
    // oscillated NONE<->PUFF and emitted a RELEASED (a phantom click, or a
    // drag that ended on its own) per oscillation. A held puff stays a puff
    // until the pressure falls back through CONF_PRESS_RELEASE_FRACTION of
    // the engage threshold.
    _sapMainState = PRESS_SAP_MAIN_STATE_PUFF;
  }
  else if (_sapPrevState.mainState == PRESS_SAP_MAIN_STATE_SIP
           && _sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED
           && pressureValue < -1 * _sipThreshold * CONF_PRESS_RELEASE_FRACTION) {
    _sapMainState = PRESS_SAP_MAIN_STATE_SIP;  // Same hysteresis, sip side
  }
  else {                                      // Neither sip nor puff detected
    _sapMainState = PRESS_SAP_MAIN_STATE_NONE;
  }

  // Sensor fault mid-hold must not emit an action-bearing RELEASED (4.5.7
  // review): when the sensors go unhealthy while a sip/puff is HELD,
  // _sapMainState is forced NONE above, and the transition block below
  // would read prev = {PUFF/SIP, STARTED} and emit {PUFF/SIP, RELEASED,
  // elapsed} — which evaluateOutputAction (it consults no health flag)
  // fires as a real action: a phantom click/drag/menu the instant a cable
  // is jostled mid-puff. Settle straight to idle instead; the hold was
  // interrupted by a fault, not released. (markSensorsUnhealthy already
  // zeroed _sapMainState; this suppresses the spurious release event.)
  if (!_sensorsHealthy && _sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED) {
    _sapCurrState = {PRESS_SAP_MAIN_STATE_NONE, PRESS_SAP_SEC_STATE_WAITING, 0};
    _sapBuffer.pushElement(_sapCurrState);
    _sapStateTimer.restartTimer(_sapStateTimerId);
    return;
  }

  // Update the state using logic
  // None:None, Sip:Sip, Puff:Puff
  if(_sapPrevState.mainState == _sapMainState
     && _sapPrevState.secondaryState != PRESS_SAP_SEC_STATE_RELEASED){ // Current state is same as previous, and the previous was not a just-emitted release
    // 4.5.7 review — restore parity with LSInput (which has had this
    // `!= RELEASED` guard all along): without it, a re-DETECTED same action
    // after a release (an unsteady/wavering puff crossing the hysteresis
    // boundary, a double-puff, or boundary noise) matched here and
    // updateLastElement'd {PUFF, RELEASED, elapsed}, LATCHING the RELEASED
    // secondary state — and because evaluateOutputAction fires on
    // (mainState, RELEASED, elapsed window) with no caller-side edge
    // detection, it re-fired the release action on EVERY 50 ms poll as
    // elapsed swept the action windows (a burst of phantom clicks / menu
    // activations) while a genuine new puff went unrecognized. With the
    // guard, a same-action-after-release correctly falls to the transition
    // block below and re-arms STARTED (a fresh, single action).
    _sapCurrState = {_sapMainState, _sapPrevState.secondaryState, _sapStateTimer.elapsedTime(_sapStateTimerId)};
    _sapBuffer.updateLastElement(_sapCurrState);
  } 
  else {  // None:Sip , None:Puff , Sip:None, Puff:None 
      // State: Sip or puff
      // Previous state: {none, waiting, time} Note: There can't be sip or puff and waiting 
      // New state: {Sip or puff, started, 0}
      if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_WAITING) {
        _sapCurrState = {_sapMainState, PRESS_SAP_SEC_STATE_STARTED, 0};
      } 
      // State: none
      // Previous state: {Sip or puff, started, time} Note: There can't be none and started 
      // New state: {Sip or puff, released, time}      
      else if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED){
        _sapCurrState = {_sapPrevState.mainState, PRESS_SAP_SEC_STATE_RELEASED, _sapPrevState.elapsedTime};
      }
      // State: None
      // Previous state: {Sip or puff, released, time}
      // New state: {none, waiting, 0}
      else if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_RELEASED && _sapMainState == PRESS_SAP_MAIN_STATE_NONE) {
        _sapCurrState = {_sapMainState, PRESS_SAP_SEC_STATE_WAITING, 0};
      }
      // State: Sip or puff
      // Previous state: {none, released, time}
      // New state: {Sip or puff, started, 0}
      else if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_RELEASED && _sapMainState != PRESS_SAP_MAIN_STATE_NONE) {
        _sapCurrState = {_sapMainState, PRESS_SAP_SEC_STATE_STARTED, 0};
      }      
      // Push the new state   
      _sapBuffer.pushElement(_sapCurrState);
      // Reset and start the timer
      _sapStateTimer.restartTimer(_sapStateTimerId);  
  }

  // No action in 1 minute : reset timer — ONLY while idle (4.5.7 review):
  // the unconditional form restarted the stopwatch mid-hold, wrapping a
  // >60 s hold's elapsed back to (T - 60 s) so the release classified into
  // the wrong window (a 63 s sealed-tube puff fired START_MENU; a 61 s one
  // engaged drag), and re-measured the offset DURING the hold — the new
  // offset absorbed the held pressure, zeroed the reading ~250 ms later,
  // and emitted a phantom RELEASED that fired the short-window action,
  // then suppressed legitimate puffs until the next idle re-measure. The
  // WAITING guard below is the original intent (its remains were left in
  // a comment by the 4.2.0 rewrite); the LSInput sibling kept it all
  // along.
  if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_WAITING
    && _sapStateTimer.elapsedTime(_sapStateTimerId) > PRESS_SAP_ACTION_TIMEOUT) {
      if (_sensorsHealthy) startOffsetUpdate();  // Stepped re-measure (was the blocking updateOffsetPressure()); skipped while unhealthy
      _sapStateTimer.restartTimer(_sapStateTimerId);    // Reset and start the timer
  }
}

//*********************************//
// Function   : getSapPressureAbs 
// 
// Description: Get the last main pressure reading from the pressure buffer 
// Arguments :  void
// 
// Return     : pressure : float : Last main pressure reading
//*********************************//
float LSPressure::getSapPressureAbs()
{
  return _pressureBuffer.getLastElement().sapPressureAbs;
}

//*********************************//
// Function   : getAmbientPressure 
// 
// Description: Get the last reference pressure reading from the pressure buffer 
// Arguments :  void
// 
// Return     : pressure : float : Last reference pressure reading
//*********************************//
float LSPressure::getAmbientPressure()
{
  return _pressureBuffer.getLastElement().ambientPressure;
}

//*********************************//
// Function   : getAmbientPressure 
// 
// Description: Get the last pressure difference from the pressure buffer 
// Arguments :  void
// 
// Return     : pressure : float : Last pressure difference from pressure buffer 
//*********************************//
float LSPressure::getSapPressure()
{
  return _pressureBuffer.getLastElement().sapPressure;
}


//*********************************//
// Function   : getAllPressureure 
// 
// Description: Get the last pressure readings including the main ,reference, and difference pressure values from the pressure buffer 
// Arguments :  void
// 
// Return     : pressure : pressureStruct : Last pressure values from pressure buffer 
//*********************************//
pressureStruct LSPressure::getAllPressure()
{
  return _pressureBuffer.getLastElement();
}


//*********************************//
// Function   : getState
//
// Description: Get the last sip and puff state from sap buffer
// Arguments :  void
//
// Return     : state : inputStateStruct : Last sip and puff state
//*********************************//
inputStateStruct LSPressure::getState()
{
  return _sapBuffer.getLastElement();
}


//*********************************//
// Function   : isSensorsHealthy
//
// Description: Sensor-health gate state for the bus-health diagnostic
//              endpoint (BH): false means the sensors stopped returning
//              valid readings and the I2C bus is being left alone except
//              for a slow re-probe — sip and puff cannot fire in that
//              state, so a support readout must be able to see it.
// Arguments :  void
//
// Return     : healthy : bool : both pressure sensors return valid readings
//*********************************//
bool LSPressure::isSensorsHealthy()
{
  return _sensorsHealthy;
}


#endif
