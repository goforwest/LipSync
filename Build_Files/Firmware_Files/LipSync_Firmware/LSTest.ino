/* 
* File: LSTestS.ino
* Firmware: LipSync
* Developed by: Neil Squire Society / Makers Making Change Program
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


// To add tests:
// 1. Create the CONF_TEST_MODE_{name} in LSConfig.h
// 2. Increment the CONF_TEST_MODE_MAX number
// 3. Add the switch case to the activateTest function
// 4. Implement the test below.

static void testLog(const char* line) {
  serialTxEnqueueLine(line, strlen(line));
}

static void testLog(const String& line) {
  serialTxEnqueueLine(line);
}

static void testDelay(unsigned long durationMs) {
  const unsigned long started = millis();
  while (millis() - started < durationMs) {
    buzzer.update();
    serialTxService();
    if (g_watchdogArmed) watchdogLoop();
    delay(10);
  }
}

//***ACTIVATE TEST FUNCTION***//
// Function   : activateTest
//
// Description: This function activates different tests
//
// Parameters : inputTest : int : The id of the test to activate
//
// Return     : void
//****************************************//
void activateTest(int inputTest) {

  // Test mode deliberately stalls the main loop for many seconds at a time
  // (LED sweeps, sound delays, the watchdog starvation test) — the TIMER3
  // hang witness would SREQ the device ~4 s into tests that are SUPPOSED
  // to take longer (4.5.7 review). Same exemption class as softwareReset's
  // shutdown: a deliberate stall is not a hang. Cleared before "Test
  // Complete" so normal operation resumes.
  g_shutdownInProgress = true;

  switch (inputTest) {
    case (CONF_TEST_MODE_LED):
      testLED();
      break;

    case (CONF_TEST_MODE_BUZZER):
      testBuzzer();
      break;

    case (CONF_TEST_MODE_WATCHDOG):
      testWatchdog();
      break;

  } // end switch inputTest
  // 4.6.6: the test stalled the loop for many seconds without pumping the
  // poll pools (testDelay deliberately feeds only the buzzer/serial/
  // watchdog). The idealized timer anchors make every missed interval due at
  // once — without this re-anchor, the moment the test ends, run() repays
  // the debt one fire per loop pass: a multi-second burst of back-to-back
  // 20 ms I2C/pressure/input/screen polls right as the user resumes input.
  // Anchor to now so normal operation resumes on a clean one-interval
  // cadence. (The watchdog-test case ends in its reset; anchoring first is
  // harmless there.)
  pollTimer.anchorAllToNow();
  ledStateTimer.anchorAllToNow();
  calibrationTimer.anchorAllToNow();
  actionTimer.anchorAllToNow();
  g_shutdownInProgress = false;
  testLog("Test Complete");
}


//***TEST LED FUNCTION***//
// Function   : testLED
//
// Description: This function tests the LED patterns
//
// Parameters : void
//
// Return     : void
//****************************************//
void testLED() {

  //setLedColor(int ledNumber, int ledColorNumber, int ledBrightness)

  testLog("LED TEST ACTIVATED");
  testLog("TEST_MODE_LED: All LED OFF");
  led.clearLedAll();
  testDelay(1000);

  testLog("TEST_MODE_LED: Left LED ON");
  led.clearLedAll();
  led.setLedColor(CONF_LED_LEFT, LED_CLR_RED, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: Middle LED ON");
  led.clearLedAll();
  led.setLedColor(CONF_LED_MIDDLE, LED_CLR_RED, CONF_LED_BRIGHTNESS);
  testDelay(1000);


  testLog("TEST_MODE_LED: Right LED ON");
  led.clearLedAll();
  led.setLedColor(CONF_LED_RIGHT, LED_CLR_RED, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: ALL LED OFF");
  led.clearLedAll();


  testLog("TEST_MODE_LED: Micro LED Blue");
  led.clearLedAll();
  led.setLedColor(CONF_LED_MICRO, LED_CLR_BLUE, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: Micro LED Purple");
  led.clearLedAll();
  led.setLedColor(CONF_LED_MICRO, LED_CLR_PURPLE, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: Micro LED Red");
  led.clearLedAll();
  led.setLedColor(CONF_LED_MICRO, LED_CLR_RED, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: Micro LED Orange");
  led.clearLedAll();
  led.setLedColor(CONF_LED_MICRO, LED_CLR_ORANGE, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: Micro LED Yellow");
  led.clearLedAll();
  led.setLedColor(CONF_LED_MICRO, LED_CLR_YELLOW, CONF_LED_BRIGHTNESS);
  testDelay(1000);

  testLog("TEST_MODE_LED: Left LED Brightness");
  led.clearLedAll();

  int brightnessDirection = 1;

  for (int brightness = 0; brightness > -1; brightness = brightness + brightnessDirection) {
    led.setLedColor(CONF_LED_LEFT, LED_CLR_RED, brightness);
    if (brightness == 255) {
      brightnessDirection = -1;  // Switch direction at peak
    }
    testDelay(10);
  }
  testDelay(1000);


  testLog("TEST_MODE_LED: Error 3");
  led.clearLedAll();
  
  int blinks = 3;
  ledError(blinks);

  // imitate operation loop
  while (ledTimerId[CONF_TIMER_LED_ERROR] >= 0 &&
         ledStateTimer.isEnabled(ledTimerId[CONF_TIMER_LED_ERROR]) == true) {
    ledStateTimer.run();  // Timer for lights
    buzzer.update();
    serialTxService();
    if (g_watchdogArmed) watchdogLoop();
    delay(10);
  }

  testDelay(1000);
  testLog("TEST_MODE_LED: LED DEFAULT");
  setLedDefault();  // set LEDs to default
 
  testLog("TEST_MODE_LED: TEST COMPLETE");
}

//***TEST BUZZER FUNCTION***//
// Function   : testBuzzer
//
// Description: This function tests the buzzer sounds
//
// Parameters : void
//
// Return     : void
//****************************************//
void testBuzzer() {

  testLog("BUZZER TEST ACTIVATED");

  testLog("TEST_MODE_BUZZER: Playing startup sound.");
  buzzer.playStartupSound();
  testDelay(1000);

  testLog("TEST_MODE_BUZZER: Playing ready sounds.");
  buzzer.playReadySound();
  testDelay(1000);

  testLog("TEST_MODE_BUZZER: Playing error sound.");
  buzzer.playErrorSound();
  testDelay(1000);

  testLog("TEST_MODE_BUZZER: Playing corner calibration sound.");
  buzzer.calibCornerTone();
  testDelay(1000);

  testLog("TEST_MODE_BUZZER: Playing center calibration sound.");
  buzzer.calibCenterTone();
  testDelay(1000);

  testLog("TEST_MODE_BUZZER: Playing shutdown sound.");
  buzzer.playShutdownSound();
  testDelay(1000);

  // TODO Test Sound levels
  // TODO test enable/disable sound

  testLog("SOUND TEST COMPLETE");
}  // end testBuzzer()

//***TEST WATCHDOG FUNCTION***//
// Function   : testWatchdog
//
// Description: This function tests the watchdog
//
// Parameters : void
//
// Return     : void
//****************************************//
void testWatchdog() {

  testLog("WATCHDOG TEST ACTIVATED");

  testLog("TEST_MODE_WATCHDOG: Delaying longer than watchdog reset...");
  testLog(String("TEST_MODE_WATCHDOG: CONF_WATCHDOG_TIMEOUT_SEC:") + String(CONF_WATCHDOG_TIMEOUT_SEC));

  unsigned long watchdogTestStart = millis();
  unsigned long currentTime = millis();

  unsigned long watchdogTestTimeout = CONF_WATCHDOG_TIMEOUT_SEC*1000+5000;

  while (currentTime - watchdogTestStart < watchdogTestTimeout) {
    delay(1000);
    testLog(String((currentTime-watchdogTestStart)/1000));
    currentTime = millis();
  }
  testLog("TEST_MODE_WATCHDOG:TEST FAILED"); // Should never reach this if watchdog is enabled
  
  testLog("WATCHDOG TEST COMPLETE");
}  // end testBuzzer()
