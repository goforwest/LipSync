/* 
* File: LSWatchdog.h
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
#ifndef _LSWATCHDOG_H
#define _LSWATCHDOG_H

#include <nrf_wdt.h>
#include <hal/nrf_power.h>  //  Defines reset reasons for NRF52 (https://github.com/particle-iot/nrf5_sdk/blob/master/modules/nrfx/hal/nrf_power.h)

const uint32_t WATCHDOG_RESET_MASK = 0x2; //POWER_RESETREAS_DOG_Msk; //  0x2;
extern bool g_watchdogReset;
extern uint8_t g_watchdogBootCount;  // Consecutive watchdog-reset boots this power cycle (tracked in GPREGRET2)
extern uint8_t g_hangBootStreak;    // Consecutive witnessed loop-hang deaths (GPREGRET2 bits 0-1 — the functional budget)
extern unsigned long g_watchdogLastFeedMs;  // Loop-liveness witness (bench 2026-10-18): ms timestamp of the last feed
extern volatile bool g_watchdogArmed;        // True from initWatchdog() until the retry limit stops re-arming (ISR-read by the TIMER3 witness)
extern volatile uint32_t g_witnessSecondsUnfed;  // Witness-ISR <-> loop handshake (see LipSync_Firmware.ino)
extern uint32_t g_lastRebootReason;

//*********************************//
// Watchdog Functions
//*********************************//

//***CHECK RESET REASON FUNCTION***//
// Function   : initWatchdog
//
// Description: This function checks why the microcontroller was reset / powered.
//
// Parameters : void
//
// Return     : void
//****************************************//
void checkResetReason() {
  // Check the internal register for why the device was reset 
   
  // Determine last reset reason
    g_lastRebootReason = readResetReason(); // from Adafruit_nRF52_Arduino/cores/NRF/wiring.c

  	
  // Check why the device was reset
  if(USB_DEBUG) { Serial.print("Reset reason: "); }
  if ((g_lastRebootReason & NRF_POWER_RESETREAS_RESETPIN_MASK) == NRF_POWER_RESETREAS_RESETPIN_MASK) { 
    if(USB_DEBUG) { Serial.println("Reset Pin Reset"); }
  } else if ((g_lastRebootReason & NRF_POWER_RESETREAS_DOG_MASK) == NRF_POWER_RESETREAS_DOG_MASK) {
    if(USB_DEBUG) { Serial.println("Watchdog Reset"); }
    g_watchdogReset = true;
  } else if ((g_lastRebootReason & NRF_POWER_RESETREAS_SREQ_MASK) == NRF_POWER_RESETREAS_SREQ_MASK) {
    if(USB_DEBUG) { Serial.println("Software Reset"); }
  } else {
    if(USB_DEBUG) { Serial.println("Other reset"); }
  }

  // THE HANG-WITNESS STREAK (4.5.7 review) — the settling budget that
  // actually works on this platform. The old consecutive-watchdog-boot
  // counter never accumulated: the hardware watchdog reset WIPES
  // GPREGRET2 (bench-proven, 4.5.6 — every DOG-successor read the trail
  // at 0), so the "3-strike" budget was decorative; it is kept documented
  // as such. But the TIMER3 hang witness resets via SREQ, which PRESERVES
  // GPREGRET2 — so a streak of consecutive witnessed loop-hang deaths CAN
  // accumulate in bits 0-1:
  //   - Previous boot died in a witnessed hang (hang marker bits 2-4 set
  //     at death, still unread — setup() consumes them AFTER this
  //     function): streak = min(streak + 1, 3).
  //   - Previous boot ended any other way (clean power-cycle, reset pin,
  //     software reset, watchdog): streak = 0.
  // checkSafeMode() settles the device in safe mode at
  // CONF_WATCHDOG_BOOT_RETRY_LIMIT straight witnessed-hang deaths, and
  // g_watchdogBootCount mirrors the streak so setup()'s existing
  // "stop re-arming" gate covers the settled boot. A boot that stays up
  // for CONF_WATCHDOG_BOOT_CLEAR_MS clears the streak (watchdogLoop), so
  // isolated hangs never exhaust the budget. All writes touch only their
  // own bits; the trail lives in bits 5-7.
  uint8_t prevHangMarker = (uint8_t)((NRF_POWER->GPREGRET2 >> 2) & 0x07UL);
  if (!g_watchdogReset && prevHangMarker != 0) {
    uint8_t hangStreak = (uint8_t)(NRF_POWER->GPREGRET2 & 0x03UL) + 1;
    if (hangStreak > 3) hangStreak = 3;
    NRF_POWER->GPREGRET2 = (NRF_POWER->GPREGRET2 & ~0x03UL) | hangStreak;
    g_hangBootStreak = hangStreak;
  } else {
    NRF_POWER->GPREGRET2 &= ~0x03UL;  // clear the streak — the previous boot did not die in a witnessed hang
    g_hangBootStreak = 0;
  }
  // The watchdog-reset counter itself: a DOG reset wipes the register, so
  // this always counts from zero (the documented-decorative 4.2.0 budget —
  // the streak above is the functional settling path). The hang streak
  // feeds this value so setup()'s retry-limit gate covers both classes.
  if (g_watchdogReset) {
    g_watchdogBootCount = (uint8_t)(NRF_POWER->GPREGRET2 & 0x03UL);
    if (g_watchdogBootCount > 3) g_watchdogBootCount = 3;
    if (g_watchdogBootCount == 0) g_watchdogBootCount = 1;
  } else {
    g_watchdogBootCount = g_hangBootStreak;
  }
}

//***INITIALIZE WATCHDOG FUNCTION***//
// Function   : initWatchdog
//
// Description: This function initializes the hardware watchdog
//
// Parameters : void
//
// Return     : void
//****************************************//
void initWatchdog() {
  if (USB_DEBUG) { Serial.println("USBDEBUG: initWatchdog()"); }
  // Configure watchdog timer with a defined timeout
  NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) |  // Keep running during halt
                    (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);   // Keep running in sleep mode
  NRF_WDT->CRV = CONF_WATCHDOG_TIMEOUT_SEC * 32768;                   // Convert timeout in seconds to watchdog cycles (1 cycle = 1/32.768kHz)
  NRF_WDT->RREN |= WDT_RREN_RR0_Msk;                                  // Enable reload register 0
  NRF_WDT->TASKS_START = 1;                                           // Start watchdog
  g_watchdogArmed = true;
}

//***WATCHDOG LOOP FUNCTION***//
// Function   : watchdogLoop
//
// Description: This function resets the watchdog timer to prevent reset
//
// Parameters : void
//
// Return     : void
//****************************************//
void watchdogLoop(void) {
  //if (USB_DEBUG) { Serial.println("USBDEBUG: watchDogLoop"); }

  g_watchdogLastFeedMs = millis();
  g_witnessSecondsUnfed = 0;  // The loop is alive — reset the witness ISR's un-fed-seconds counter (4.5.7 review; a single aligned volatile store)
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;  // Feed (reset) the watchdog timer

  // A boot that stays up this long proves any hang was transient — restore
  // the full hang-streak budget for the next one (and the watchdog boot
  // count, for the DOG class).
  static bool watchdogBootCountCleared = false;
  if (!watchdogBootCountCleared && millis() > CONF_WATCHDOG_BOOT_CLEAR_MS) {
    NRF_POWER->GPREGRET2 &= ~0x03UL;  // clear only the counter bits (the trail bits 5-7 stay)
    watchdogBootCountCleared = true;
  }
}


#endif
