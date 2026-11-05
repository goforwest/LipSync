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
#include "LSApiDeclarations.h"
#include "LSPressureClassify.h"
// Pin the pure classifier's release fraction to the config constant: the
// classifier is host-compiled without LSConfig.h and carries its own copy —
// a config change that skips this assert would leave the shipped release
// boundary at the old value while every reader of the macro believes
// otherwise. Float-precision comparison (the macro is a double literal).
static_assert((float)LSPRESSURE_CLASSIFY_RELEASE_FRACTION == (float)CONF_PRESS_RELEASE_FRACTION,
              "LSPRESSURE_CLASSIFY_RELEASE_FRACTION must equal CONF_PRESS_RELEASE_FRACTION");
#include "LSPressureEngageFilter.h"
#include "LSPressureOffsetFilter.h"

#define LPS22_I2CADDR 0x5C      // Modified LPS22 address
#define LIPSYNC_PRESS_STATUS 0x27       // P_DA + T_DA status byte on both pressure sensors
#define LIPSYNC_PRESS_STATUS_P_DA 0x01  // A fresh pressure conversion is available

enum PressureReadResult : uint8_t {
  PRESS_READ_FAILED = 0,
  PRESS_READ_STALE,
  PRESS_READ_FRESH
};

#define PRESS_BUFF_SIZE 5       // The size of pressure Buffer
#define PRESS_SAP_BUFF_SIZE 12  // The size of sip and puff state buffer 

#define PRESS_SAP_DEFAULT_THRESHOLD 3.0 // hPa hecto Pascals

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
#define PRESS_STALE_TIMEOUT_MS 500      // No complete fresh pair for this long means conversion has stalled
#define PRESS_REPROBE_INTERVAL_MS 2000  // Spacing between sensor re-probes while unhealthy (keeps a dead
                                        // or flaky sensor from stalling the main loop on every poll)
#define PRESS_PROBE_GOOD_LIMIT 2        // Consecutive valid re-probes before the sensors are marked healthy again
#define PRESS_OFFSET_ATTEMPT_LIMIT 10   // updatePressure() ticks spent gathering offset samples before giving up
#define PRESS_SAP_ENGAGE_SAMPLES 2      // Two matching 50 ms polls reject a one-sample phantom click
#define PRESS_OFFSET_STABILITY_HPA 0.25 // A baseline candidate must stay within this spread
#define PRESS_OFFSET_NEUTRAL_FRACTION 0.25 // Established offsets only refresh very near neutral
#define PRESS_OFFSET_NEUTRAL_MAX_HPA 1.0 // Never treat a large pressure shift as neutral just because thresholds are high
#define PRESS_OFFSET_INITIAL_MAX_HPA 3.0 // Boot baseline magnitude ceiling: at/under the DEFAULT regular threshold, above the sensor pair's plausible inter-sensor bias (each part is a +/-1 hPa class) — a held gesture this strong can never become "neutral"; see LSPressureOffsetFilter.h for the exact residual with lowered user thresholds
#define PRESS_OFFSET_RETRY_REPORT_WINDOWS 120 // Consecutive failed boot-baseline windows (~60 s at the 50 ms poll) before one ERROR line names the stuck neutral
#define PRESS_UNHEALTHY_REPORT_MIN_INTERVAL_MS 5000UL // Minimum spacing of the "sensors not responding" ERROR line (a flapping cable re-enters the unhealthy state every re-probe cycle)

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
    float getOffsetPressure();                          // Get Offset pressure: (sapPressureAbs- ambientPressure)
    void setOffsetPressure(float offsetPressure);       // Set Offset pressure: 
    float offsetNeutralLimit();                         // The shared "very near neutral" band (min(ST,PT)*0.25 capped at PRESS_OFFSET_NEUTRAL_MAX_HPA) — one definition for both the collector and the 60 s refresh gate
    void startOffsetUpdate();                           // Begin a stepped offset measurement: five valid (sap - ambient)
                                                        // samples gathered one per update() tick, then averaged
    void setSipThreshold(float s);                      // Set sip threshold
    void setPuffThreshold(float p);                     // Set puff threshold
    void setHardSipThreshold(float t);                  // Set hard sip threshold (upstream #58; validated at the API layer)
    void setHardPuffThreshold(float t);                 // Set hard puff threshold (upstream #58)
    float getHardSipThreshold();                        // Get hard sip threshold
    float getHardPuffThreshold();                       // Get hard puff threshold
    float getSipPressureThreshold();                    // Get regular sip threshold (for hard-band validation)
    float getPuffPressureThreshold();                   // Get regular puff threshold (for hard-band validation)
    void updatePressure();                              // Update the pressure buffer with new readings 
    void updateState();                                 // Update the and puff buffer with new states 
    float getSapPressureAbs();                          // Get last main pressure from pressure buffer
    float getAmbientPressure();                         // Get last reference pressure from pressure buffer
    float getSapPressure();                             // Get the Pressure Difference sapPressure = (sapPressureAbs- ambientPressure- offsetPressure)
    pressureStruct getAllPressure();                    // Get the latest pressure values
    inputStateStruct getState();                        // Get the latest sip and puff state
    bool isSensorsHealthy();                            // True while both pressure sensors return valid readings (false = gated: re-probing slowly, sip/puff cannot fire)
    bool takeHoldCancelPending();                       // Take-and-clear: a sensor fault interrupted an ACTIVE sip/puff hold.
                                                        // The fault path settles the state machine to idle WITHOUT emitting a
                                                        // RELEASED (no phantom action), so the consumer must end the engaged
                                                        // drag/scroll latch itself — the only code that releases a hold keys on
                                                        // RELEASED, and a latch that never sees it strands the held mouse button
                                                        // and blocks every later action dispatch (canOutputAction=false) until
                                                        // recovery. Call once per pressureLoop pass.

  private: 
      Adafruit_LPS35HW _lps35hw = Adafruit_LPS35HW();     // Create an object of Adafruit_LPS35HW class for Sip and Puff pressure
      Adafruit_LPS22 _lps22;                              // Create an object of Adafruit_LPS2X class for ambient pressure
      LSCircularBuffer <pressureStruct> _pressureBuffer;  // Create a buffer of type pressureStruct to push pressure readings
      LSCircularBuffer <inputStateStruct> _sapBuffer;     // Create a buffer of type inputStateStruct to push sap states
      int _pressureMode = PRESS_MODE_DIFF;                // Pressure Mode: DIFF or ABS pressure
      float _sapPressureAbs = 0.0f;                       // Main Pressure reading (Sip and Puff Absolute) [hPa]
      float _ambientPressure = 0.0f;                      // Reference Pressure reading (Ambient Absolute) [hPa]
      float _offsetPressure = 0.0f;                       // Offset Pressure [hPa] (Difference between two sensors)
      float _sapPressure = 0.0f;                          // Calculated Pressure Difference sapPressure = (sapPressureAbs- ambientPressure- offsetPressure)
      LSTimer <void> _sapStateTimer;                      // Timer used to measure time for each sip and puff action. 
      int _sapStateTimerId = -1;                           // The id for the sap state timer
      inputStateStruct _sapCurrState = {PRESS_SAP_MAIN_STATE_NONE, PRESS_SAP_SEC_STATE_WAITING, 0};
      inputStateStruct _sapPrevState = {PRESS_SAP_MAIN_STATE_NONE, PRESS_SAP_SEC_STATE_WAITING, 0};
      float _sipThreshold = PRESS_SAP_DEFAULT_THRESHOLD;   // Sip Threshold
      float _puffThreshold = PRESS_SAP_DEFAULT_THRESHOLD;  // Puff Threshold
      float _hardSipThreshold = CONF_HARD_SIP_THRESHOLD;   // Hard sip threshold (upstream #58) — must exceed _sipThreshold (API-enforced margin)
      float _hardPuffThreshold = CONF_HARD_PUFF_THRESHOLD; // Hard puff threshold (upstream #58) — must exceed _puffThreshold (API-enforced margin)
      // Thresholds pinned at hold START (4.8.1): continuation and the release
      // boundary run on these snapshots, not the live values. A live ST/PT
      // reconfiguration mid-hold (the app pushes settings on connect — a
      // caregiver opening Connect while the user holds a puff is realistic)
      // used to re-test the in-flight hold against the NEW threshold and emit
      // an action-bearing RELEASED the user did not perform (a phantom click),
      // or to extend a hold below its old engage level. The classifier's own
      // contract ("class held for life", hard-sticky down) now holds for the
      // regular bands too: one gesture, the thresholds it started with. Fresh
      // entries always classify against the live values.
      float _activeSipThreshold = PRESS_SAP_DEFAULT_THRESHOLD;
      float _activePuffThreshold = PRESS_SAP_DEFAULT_THRESHOLD;
      float _activeHardSipThreshold = CONF_HARD_SIP_THRESHOLD;
      float _activeHardPuffThreshold = CONF_HARD_PUFF_THRESHOLD;
      int _sapMainState = PRESS_SAP_MAIN_STATE_NONE;       // The value which represents the current main state (example: PRESS_SAP_MAIN_STATE_PUFF)
      bool _sensorsHealthy;                               // Both sensors return valid readings (false until begin() correlates it with the sensor-detection globals — see the constructor)
      bool _begun;                                        // begin() ran, so the Adafruit driver objects are safe to call. A never-begun LPSressure holds NULL i2c_dev pointers and ANY driver call faults (bench 2026-10-28: PV,0:0 on a missing-sensor boot HardFaulted inside Adafruit_I2CDevice::write_then_read — pm=2 pt=7 fp=0xE000, twice). Every driver I/O this class makes is behind this gate.
      unsigned long _lastProbeMs;                         // Timestamp of the last unhealthy-mode re-probe
      uint8_t _badReadCount;                              // Consecutive invalid readings while healthy
      uint8_t _goodProbeCount;                            // Consecutive valid re-probes while unhealthy
      unsigned long _lastFreshPairMs;                     // Fresh-conversion liveness (P_DA on both sensors)
      bool _offsetUpdateRunning;                          // Offset measurement in progress (stepped by updatePressure())
      bool _offsetBaselineValid;                          // A complete stable baseline exists; later refreshes must remain neutral relative to it
      bool _offsetRequiredBeforeSensing;                  // Boot/recovery must complete a safe baseline before sip/puff can resume
      uint8_t _bootOffsetRetryWindows = 0;                // Consecutive failed boot-baseline windows — names a stuck neutral on the wire once per minute instead of failing in silence (the 4.6.6 destroy-in-silence lesson)
      unsigned long _lastUnhealthyReportMs = 0;           // Last "sensors not responding" line (rate-limited by PRESS_UNHEALTHY_REPORT_MIN_INTERVAL_MS)
      LSPressureOffsetFilter _offsetFilter{PRESS_BUFF_SIZE, PRESS_OFFSET_ATTEMPT_LIMIT, PRESS_OFFSET_STABILITY_HPA, PRESS_OFFSET_INITIAL_MAX_HPA};
      // (requiredSamples intentionally reuses PRESS_BUFF_SIZE: the offset
      // baseline is a 5-sample stable run, matching the pressure ring depth
      // and the documented "five valid samples" behavior. Changing
      // PRESS_BUFF_SIZE for buffering reasons also changes the baseline
      // length and the ~250 ms collect budget — keep them deliberately
      // coupled or split the constant if they ever diverge.)
      bool _currentSampleValid;                           // This poll produced a fresh plausible pressure pair (never count stale buffer data as another engagement sample)
      LSPressureEngageFilter _engageFilter{PRESS_SAP_ENGAGE_SAMPLES}; // New holds require consecutive same-direction samples
      unsigned long _sapEngageElapsedBiasMs;              // Time already spent in the consecutive-sample qualifier, included in hold-window timing
      bool _holdCancelPending;                            // Set when a fault settles an ACTIVE hold to idle (see takeHoldCancelPending)
      void markSensorsUnhealthy();                        // Disable sip and puff and stop reading I2C until a re-probe succeeds
      PressureReadResult readFreshPressure(uint8_t address, float* pressureHpa); // Exact status + 24-bit pressure read shared by both sensors
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
  _lastFreshPairMs = 0;
  _offsetUpdateRunning = false;
  _offsetBaselineValid = false;
  _offsetRequiredBeforeSensing = true;
  _currentSampleValid = false;
  _sapEngageElapsedBiasMs = 0;
  _holdCancelPending = false;
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

  setPressureMode(PRESS_MODE_DIFF);         // Set the default pressure mode to differential mode (i.e., mouthpiece and ambient)
  
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
  _lastFreshPairMs = millis();
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
  _engageFilter.reset();
  _sapEngageElapsedBiasMs = 0;

  // Reset and start the timer   
  if (_sapStateTimerId >= 0) _sapStateTimer.deleteTimer(_sapStateTimerId);
  _sapStateTimerId = _sapStateTimer.startTimer();

}

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
  // 4.6.6, tightened 4.8.1: only PRESS_MODE_DIFF is wired. The ABS math
  // (ambient := the mouthpiece reading, difference = rawDifference - 0)
  // algebraically zeroes _sapPressure — with ABS active, NO sip or puff could
  // ever cross any threshold: a fully silent input device. PRESS_MODE_NONE
  // was equally silent (the non-DIFF path folded the mouthpiece reading into
  // ambient), so the guard is now a whitelist rather than a single refusal:
  // any mode that is not the one wired implementation is a no-op. No shipped
  // caller passes anything else (begin() passes DIFF; no API endpoint or
  // menu path exists) — this is dead-code landmine disposal, not a live
  // defect. The refusal is a silent store-refusal; the getter keeps
  // reporting the mode actually in force.
  if (mode != PRESS_MODE_DIFF) {
    return;
  }
  _pressureMode = mode;
}

//*********************************//
// Function   : setHardSipThreshold / setHardPuffThreshold
//
// Description: Upstream #58: the hard band thresholds. The API layer
//              validates hard >= regular + CONF_PRESS_HARD_MARGIN_HPA and
//              heals out-of-order stored values at boot; this setter trusts
//              its caller (an out-of-order pair makes the hard band empty
//              in the classifier — fail-safe, the feature goes inert).
//
// Arguments :  t : float : the new hard threshold [hPa]
//
// Return     : void
//*********************************//
void LSPressure::setHardSipThreshold(float t)
{
  _hardSipThreshold = t;
}

void LSPressure::setHardPuffThreshold(float t)
{
  _hardPuffThreshold = t;
}

float LSPressure::getHardSipThreshold()
{
  return _hardSipThreshold;
}

float LSPressure::getHardPuffThreshold()
{
  return _hardPuffThreshold;
}

float LSPressure::offsetNeutralLimit()
{
  // One definition (4.8.1 review): the collector (startOffsetUpdate) and the
  // 60 s refresh gate both need the same conservative band. The filter's own
  // begin() clamps a non-positive limit to 0.01; with the API-enforced
  // threshold floor (2.0 hPa) this is always >= 0.5.
  float neutralLimit = min(_sipThreshold, _puffThreshold) * PRESS_OFFSET_NEUTRAL_FRACTION;
  return min(neutralLimit, (float)PRESS_OFFSET_NEUTRAL_MAX_HPA);
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
  _offsetFilter.begin(_offsetPressure, _offsetBaselineValid, offsetNeutralLimit());
  _engageFilter.reset();  // A pre-refresh one-sample candidate cannot survive the inert collection window
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
  _sapMainState = PRESS_SAP_MAIN_STATE_NONE;  // Defense-in-depth (the operative suppression is updateState's _sensorsHealthy gate + the fault-settle branch; this store is overwritten on the next qualify())
  _currentSampleValid = false;
  _offsetUpdateRunning = false;
  _offsetRequiredBeforeSensing = true;
  _offsetFilter.cancel();
  _engageFilter.reset();
  _sapEngageElapsedBiasMs = 0;
  // Bounded reporting (4.8.1 review): a flapping cable re-enters this path
  // every re-probe cycle and used to enqueue the line on EVERY entry. One
  // report per minimum interval keeps the wire honest without spamming the
  // response queue during an I2C storm.
  static const char sensorError[] = "ERROR: Pressure sensors not responding — sip and puff disabled until they recover";
  const unsigned long nowMs = millis();
  // The FIRST report after boot is unconditional (_lastUnhealthyReportMs == 0
  // is the sentinel): a failure in the first seconds must not be suppressed.
  // The rate limit then bounds a FLAPPING cable that re-enters the state on
  // every recovery cycle.
  if (_lastUnhealthyReportMs == 0
      || (nowMs - _lastUnhealthyReportMs) >= PRESS_UNHEALTHY_REPORT_MIN_INTERVAL_MS) {
    _lastUnhealthyReportMs = nowMs;
    serialTxEnqueueLine(sensorError, sizeof(sensorError) - 1);
  }
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
// Function   : getSipPressureThreshold / getPuffPressureThreshold
//
// Description: Accessors for the current regular thresholds (needed so the
//              hard-threshold setter can validate hard >= regular + margin).
//
// Return     : float : the current threshold [hPa]
//*********************************//
float LSPressure::getSipPressureThreshold()
{
  return _sipThreshold;
}

float LSPressure::getPuffPressureThreshold()
{
  return _puffThreshold;
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
  // getSapPressure() returns the last buffered value. Mark it stale until
  // this poll actually produces a complete plausible sensor pair, otherwise
  // one valid over-threshold transient followed by an invalid I2C read would
  // be observed twice and falsely satisfy the two-sample engage qualifier.
  _currentSampleValid = false;

  // Unhealthy: leave the I2C bus alone except for a slow re-probe. A dead or
  // flaky sensor can no longer stall this loop; recovery is automatic once
  // the sensor answers again.
  if (!_sensorsHealthy) {
    if (nowMs - _lastProbeMs < PRESS_REPROBE_INTERVAL_MS) return;
    _lastProbeMs = nowMs;
    float probeAbs = 0.0f;
    float probeAmbient = 0.0f;
    PressureReadResult probeAbsResult = readFreshPressure(LPS35HW_I2CADDR_DEFAULT, &probeAbs);
    PressureReadResult probeAmbientResult = readFreshPressure(LPS22_I2CADDR, &probeAmbient);
    // Plausibility range, not just "> 0" (4.5.7 review): both sensors read
    // absolute ~1013 hPa — a garbage-positive pair (the flaky-ACK failure
    // mode the bench has recorded on unseated cables) passed the old test
    // as "valid" and could fire phantom sip/puff events.
    if (probeAbsResult == PRESS_READ_FRESH && probeAmbientResult == PRESS_READ_FRESH
        && probeAbs > CONF_PRESS_ABS_PLAUSIBLE_MIN && probeAbs < CONF_PRESS_ABS_PLAUSIBLE_MAX
        && probeAmbient > CONF_PRESS_ABS_PLAUSIBLE_MIN && probeAmbient < CONF_PRESS_ABS_PLAUSIBLE_MAX) {
      if (++_goodProbeCount >= PRESS_PROBE_GOOD_LIMIT) {
        _sensorsHealthy = true;
        _goodProbeCount = 0;
        _badReadCount = 0;
        _lastFreshPairMs = nowMs;
        _offsetRequiredBeforeSensing = true;
        startOffsetUpdate();  // Offsets drift while the sensors were down — re-measure
      }
    } else {
      _goodProbeCount = 0;
    }
    return;
  }

  float tempSapPressureAbs = 0.0f;
  float tempAmbientPressure = 0.0f;
  PressureReadResult sapRead = readFreshPressure(LPS35HW_I2CADDR_DEFAULT, &tempSapPressureAbs);
  // Only DIFF is wired (setPressureMode refuses everything else), so the
  // ambient read is unconditional — the old ABS shortcut branch (ambient :=
  // the mouthpiece reading) was unreachable dead code whose math would
  // algebraically zero the sip/puff signal.
  PressureReadResult ambientRead = readFreshPressure(LPS22_I2CADDR, &tempAmbientPressure);

  // A cooperative timer that is catching up can invoke two nominal 50 ms
  // pressure ticks only a millisecond apart. Reading output registers alone
  // would count the SAME 25 Hz conversion twice and let one transient satisfy
  // the two-sample engagement qualifier. The P_DA status bit is cleared by
  // the complete output read, so only two distinct sensor conversions can be
  // accepted as two samples. A merely-stale read is not a sensor-health
  // failure; failed/short transactions and implausible fresh data still are.
  const bool pairFresh = (sapRead == PRESS_READ_FRESH && ambientRead == PRESS_READ_FRESH);
  const bool transactionFailed = (sapRead == PRESS_READ_FAILED || ambientRead == PRESS_READ_FAILED);
  const bool pairPlausible = pairFresh
                          && tempSapPressureAbs > CONF_PRESS_ABS_PLAUSIBLE_MIN
                          && tempSapPressureAbs < CONF_PRESS_ABS_PLAUSIBLE_MAX
                          && tempAmbientPressure > CONF_PRESS_ABS_PLAUSIBLE_MIN
                          && tempAmbientPressure < CONF_PRESS_ABS_PLAUSIBLE_MAX;
  const bool validReading = pairPlausible;
  const bool invalidFreshReading = pairFresh && !pairPlausible;
  if (validReading) {
    _sapPressureAbs = tempSapPressureAbs;
    _ambientPressure = tempAmbientPressure;
    _lastFreshPairMs = nowMs;
  }

  // Stepped offset measurement: this tick's reading IS the sample, so the old
  // 5-read blocking re-measure (up to 15 s of frozen main loop on a bad
  // sensor) costs one I2C transaction per tick instead. A complete stable
  // run is required. Once a baseline exists, any sample outside a tight
  // neutral band aborts the refresh and RETAINS the old offset: absorbing a
  // sip/puff into the reference made release/weak inputs appear reversed
  // (upstream #88).
  if (_offsetUpdateRunning) {
    // rawDifference = the two sensors' disagreement (the quantity the offset
    // cancels). The ABS alternative is refused at the setter — see above.
    float rawDifference = 0.0f;
    if (validReading) {
      rawDifference = _sapPressureAbs - _ambientPressure;
    }
    LSPressureOffsetFilter::Result offsetResult = _offsetFilter.addSample(validReading, rawDifference);
    if (offsetResult == LSPressureOffsetFilter::COMPLETE) {
      setOffsetPressure(_offsetFilter.resultOffset());
      _offsetBaselineValid = true;
      _offsetRequiredBeforeSensing = false;
      _offsetUpdateRunning = false;
      _bootOffsetRetryWindows = 0;
    } else if (offsetResult == LSPressureOffsetFilter::ACTIVITY_REJECTED
               || offsetResult == LSPressureOffsetFilter::ATTEMPTS_EXHAUSTED) {
      // Keep the last complete baseline. A routine 60-second refresh waits
      // for the next idle window; after sensor recovery, remain inert and
      // retry until readings return near the old neutral. Otherwise the first
      // post-recovery sample could be interpreted as a real input while the
      // offset is explicitly known to be unresolved.
      _offsetUpdateRunning = false;
      if (_offsetRequiredBeforeSensing) startOffsetUpdate();
    } else if (offsetResult == LSPressureOffsetFilter::RETRYING_INITIAL
               && _offsetRequiredBeforeSensing) {
      // A boot/recovery baseline keeps failing its bounded collection windows.
      // Either the user is pressing across every window or the sensors' neutral
      // difference is beyond the initial plausibility ceiling — both leave
      // sensing permanently inert, which must not happen in silence (the 4.6.6
      // destroy-in-silence lesson). One ERROR line per minute of continuous
      // failure; the counter resets when a baseline completes or the class
      // begins sensing again.
      if (++_bootOffsetRetryWindows == PRESS_OFFSET_RETRY_REPORT_WINDOWS) {
        _bootOffsetRetryWindows = 0;
        serialTxEnqueueLine("ERROR: pressure neutral not established — release the mouthpiece; sip and puff stay disabled until a resting baseline completes",
                            sizeof("ERROR: pressure neutral not established — release the mouthpiece; sip and puff stay disabled until a resting baseline completes") - 1);
      }
    }
  }

  if (validReading) {
    _badReadCount = 0;
    _sapPressure = _sapPressureAbs - _ambientPressure - _offsetPressure;              // Calculate the pressure difference 
    _pressureBuffer.pushElement({_sapPressureAbs, _ambientPressure, _sapPressure});   // Push new pressure values to pressure buffer 
    _currentSampleValid = true;
  } else if ((transactionFailed || invalidFreshReading) && ++_badReadCount >= PRESS_BAD_READ_LIMIT) {
    markSensorsUnhealthy();  // A few consecutive bad readings — stop reading I2C and re-probe slowly
  } else if (!transactionFailed && !pairFresh && nowMs - _lastFreshPairMs >= PRESS_STALE_TIMEOUT_MS) {
    // P_DA normally arrives every 40 ms at 25 Hz. A catch-up pass a
    // millisecond after a valid sample is harmlessly stale; half a second
    // without one complete pair means a converter has stopped advancing even
    // though its address still ACKs. Cancel any held input and enter the same
    // bounded re-probe path as a failed transaction.
    markSensorsUnhealthy();
  }
}

// The upstream pressure helpers cannot report a complete I2C verdict: LPS2X
// ignores its BusIO read booleans, while LPS35HW's scalar register read maps a
// failure onto an ordinary number. Read each sensor through the same bounded,
// exact-length Wire path. Starting at STATUS also proves P_DA before exposing
// the following 24-bit output and prevents one conversion from being consumed
// twice by timer catch-up passes.
PressureReadResult LSPressure::readFreshPressure(uint8_t address, float* pressureHpa)
{
  if (!_begun || pressureHpa == nullptr) return PRESS_READ_FAILED;

  Wire.beginTransmission(address);
  Wire.write((uint8_t)LIPSYNC_PRESS_STATUS);
  if (Wire.endTransmission(false) != 0) return PRESS_READ_FAILED;

  const uint8_t expected = 4;
  uint8_t received = Wire.requestFrom(address, expected, (uint8_t)true);
  if (received != expected || Wire.available() < expected) return PRESS_READ_FAILED;

  const uint8_t status = (uint8_t)Wire.read();
  uint32_t raw = (uint32_t)(uint8_t)Wire.read();
  raw |= (uint32_t)(uint8_t)Wire.read() << 8;
  raw |= (uint32_t)(uint8_t)Wire.read() << 16;
  if ((status & LIPSYNC_PRESS_STATUS_P_DA) == 0) return PRESS_READ_STALE;
  if (raw & 0x00800000UL) raw |= 0xFF000000UL;
  *pressureHpa = (float)((int32_t)raw) / 4096.0f;
  return isfinite(*pressureHpa) ? PRESS_READ_FRESH : PRESS_READ_FAILED;
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

  // Detect the raw sip/puff direction first. A NEW hold is qualified below;
  // an already-active hold keeps the existing release hysteresis unchanged.
  int detectedMainState = PRESS_SAP_MAIN_STATE_NONE;
  if (!_sensorsHealthy) {
    detectedMainState = PRESS_SAP_MAIN_STATE_NONE; // Never act on readings from unhealthy sensors
  } else if (_offsetUpdateRunning || !_offsetBaselineValid || _offsetRequiredBeforeSensing) {
    // Boot/re-probe offset gather (4.5.7 review): until the offset lands,
    // sapPressure is the RAW inter-sensor offset — exactly the quantity the
    // offset exists to cancel — and on a device whose raw offset exceeds a
    // low user threshold the old code entered PUFF/SIP STARTED on the first
    // tick, then the offset landed ~250 ms later, the pressure collapsed,
    // and the RELEASED fired the short-window action (a phantom click a few
    // hundred ms after boot). "Until it completes, sip and puff stay inert"
    // is now actually true.
    detectedMainState = PRESS_SAP_MAIN_STATE_NONE;
  } else if (!_currentSampleValid) {
    // A failed poll leaves the previous value in the circular buffer. It must
    // not count as another "consecutive" engagement sample. Preserve an
    // already-active hold through the first two transient failures (the third
    // marks the sensors unhealthy and cancels it without an action-bearing
    // RELEASED), but reset any pending new-hold qualification immediately.
    if (_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED) {
      detectedMainState = _sapPrevState.mainState;
    }
  } else {
    // Main-state classification (4.7.0 / upstream #58): the pure classifier
    // in LSPressureClassify.h — fresh hard-band entry, hold continuation
    // (class held for life: hard-sticky down, no mid-hold upgrade), then the
    // fresh regular bands with the 4.5.7 release hysteresis subsumed by the
    // continuation rules. The engage filter below still qualifies every NEW
    // hold (two consecutive same-direction fresh samples), so a hard event
    // means "started the gesture hard" with full phantom-click protection.
    // 4.8.1: an in-flight hold (prev STARTED) classifies against the
    // thresholds SNAPPED at its START, so a live ST/PT/HS/HP reconfiguration
    // mid-hold can neither release it early (the phantom-click class) nor
    // extend it — fresh entries use the live values.
    const bool holdContinuing = (_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED);
    detectedMainState = lipsyncClassifySapMainState(
        pressureValue,
        holdContinuing ? _activeSipThreshold : _sipThreshold,
        holdContinuing ? _activePuffThreshold : _puffThreshold,
        holdContinuing ? _activeHardSipThreshold : _hardSipThreshold,
        holdContinuing ? _activeHardPuffThreshold : _hardPuffThreshold,
        _sapPrevState.mainState, _sapPrevState.secondaryState,
        PRESS_SAP_SEC_STATE_STARTED);
  }

  const bool holdAlreadyActive = (_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED);
  _sapMainState = _engageFilter.qualify(detectedMainState, holdAlreadyActive);

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
    _sapEngageElapsedBiasMs = 0;
    _sapStateTimer.restartTimer(_sapStateTimerId);
    // 4.6.6: the suppression above is only half of the fault contract. The
    // engaged DRAG/SCROLL hold (left button down, canOutputAction=false,
    // scroll poll swapped in) is released ONLY by a RELEASED event, and this
    // path by construction never emits one — so the latch used to survive
    // the fault: held mouse button, every later action dispatch dead, and
    // the only escapes a hub-button press or a full post-recovery re-puff.
    // Flag the consumer (pressureLoop -> cancelSapOutputHold) to end the
    // hold the same way the hold-release branch does — LED default + latch
    // release, still with NO action-bearing event on the wire.
    _holdCancelPending = true;
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
    unsigned long stateElapsed = _sapStateTimer.elapsedTime(_sapStateTimerId);
    if (_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED) {
      stateElapsed += _sapEngageElapsedBiasMs;
    }
    _sapCurrState = {_sapMainState, _sapPrevState.secondaryState, stateElapsed};
    _sapBuffer.updateLastElement(_sapCurrState);
  } 
  else {  // None:Sip , None:Puff , Sip:None, Puff:None 
      // State: Sip or puff
      // Previous state: {none, waiting, time} Note: There can't be sip or puff and waiting 
      // New state: {Sip or puff, started, 0}
      if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_WAITING) {
        _sapEngageElapsedBiasMs = (_sapMainState == PRESS_SAP_MAIN_STATE_NONE)
                                ? 0
                                : (unsigned long)(PRESS_SAP_ENGAGE_SAMPLES - 1) * CONF_PRESSURE_POLL_RATE;
        _sapCurrState = {_sapMainState, PRESS_SAP_SEC_STATE_STARTED, _sapEngageElapsedBiasMs};
      } 
      // State: none
      // Previous state: {Sip or puff, started, time} Note: There can't be none and started 
      // New state: {Sip or puff, released, time}      
      else if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_STARTED){
        // Use the live stopwatch at the release poll. The previous cached
        // value was one 50 ms sample behind; add the qualifier interval so
        // two-sample safety does not shift the configured action windows.
        _sapCurrState = {_sapPrevState.mainState, PRESS_SAP_SEC_STATE_RELEASED,
                         _sapStateTimer.elapsedTime(_sapStateTimerId) + _sapEngageElapsedBiasMs};
      }
      // State: None
      // Previous state: {Sip or puff, released, time}
      // New state: {none, waiting, 0}
      else if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_RELEASED && _sapMainState == PRESS_SAP_MAIN_STATE_NONE) {
        _sapCurrState = {_sapMainState, PRESS_SAP_SEC_STATE_WAITING, 0};
        _sapEngageElapsedBiasMs = 0;
      }
      // State: Sip or puff
      // Previous state: {Sip or puff, released, time}
      // New state: {Sip or puff, started, 0}
      // Defensive arm: with PRESS_SAP_ENGAGE_SAMPLES == 2 a RELEASED element
      // survives exactly one poll, the qualifier was reset at the release,
      // and the engage filter can only yield NONE on that tick — the
      // {RELEASED -> NONE} arm above always wins first. This re-arm path only
      // becomes live if the engage qualifier is ever reconfigured to a single
      // sample; it keeps that shape correct (qualifier-interval bias included)
      // instead of relying on the old comment's impossible "{none, released}"
      // previous state.
      else if(_sapPrevState.secondaryState == PRESS_SAP_SEC_STATE_RELEASED && _sapMainState != PRESS_SAP_MAIN_STATE_NONE) {
        _sapEngageElapsedBiasMs = (unsigned long)(PRESS_SAP_ENGAGE_SAMPLES - 1) * CONF_PRESSURE_POLL_RATE;
        _sapCurrState = {_sapMainState, PRESS_SAP_SEC_STATE_STARTED, _sapEngageElapsedBiasMs};
      }      
      // Push the new state   
      _sapBuffer.pushElement(_sapCurrState);
      // A STARTED hold carries the thresholds it started with for its whole
      // life (the classification snapshot above) — pin them here, at the only
      // two transitions that mint a STARTED element (WAITING->STARTED and
      // RELEASED->STARTED). Continuation and the release boundary then run on
      // these snapshots; a mid-hold ST/PT/HS/HP write applies from the NEXT
      // fresh entry.
      if (_sapCurrState.secondaryState == PRESS_SAP_SEC_STATE_STARTED) {
        _activeSipThreshold = _sipThreshold;
        _activePuffThreshold = _puffThreshold;
        _activeHardSipThreshold = _hardSipThreshold;
        _activeHardPuffThreshold = _hardPuffThreshold;
      }
      // Reset and start the timer
      _sapStateTimer.restartTimer(_sapStateTimerId);  
  }

  // No action in 1 minute : reset timer — ONLY while genuinely neutral
  // (4.5.7/4.6.5 review):
  // the unconditional form restarted the stopwatch mid-hold, wrapping a
  // >60 s hold's elapsed back to (T - 60 s) so the release classified into
  // the wrong window (a 63 s sealed-tube puff fired START_MENU; a 61 s one
  // engaged drag), and re-measured the offset DURING the hold — the new
  // offset absorbed the held pressure, zeroed the reading ~250 ms later,
  // and emitted a phantom RELEASED that fired the short-window action,
  // then suppressed legitimate puffs until the next idle re-measure. The
  // The old WAITING check inspected PREVIOUS state, so a new input beginning
  // on the 60-second boundary still started an offset refresh. The refresh
  // then averaged the user's applied pressure into neutral. Require the
  // state after this poll to be idle, a fresh sample, and pressure inside
  // the same conservative neutral band used by the collector.
  const float neutralLimitNow = offsetNeutralLimit();
  if(_sapCurrState.secondaryState == PRESS_SAP_SEC_STATE_WAITING
    && _currentSampleValid
    && fabsf(pressureValue) <= neutralLimitNow
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
// Function   : getSapPressure 
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
// Function   : getAllPressure 
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
  return _sensorsHealthy && !_offsetRequiredBeforeSensing;
}


//*********************************//
// Function   : takeHoldCancelPending
//
// Description: Take-and-clear the fault-interrupted-hold flag (see the
//              declaration). The consumer MUST end the engaged hold when
//              this returns true — the fault path suppresses the RELEASED
//              event the hold-release logic keys on.
//
// Arguments :  void
//
// Return     : bool : true exactly once per fault-interrupted hold
//*********************************//
bool LSPressure::takeHoldCancelPending()
{
  const bool pending = _holdCancelPending;
  _holdCancelPending = false;
  return pending;
}


#endif
