/* 
* File: LSConfig.h
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

#include "LSUtils.h"

// Debugging
#define USB_DEBUG  0 // Set this to 0 for best performance
#define SHOW_CONNECTION_TIME 0 // Set to 0 for production

#define CONF_ENABLE_WATCHDOG 1 // Set to 1 to enable hardware watchdog

//***DO NOT CHANGE***//
#define CONF_LIPSYNC_MODEL 1                // 1: Lipsync w/ Hub

#define CONF_LIPSYNC_VERSION_MAJOR  4
#define CONF_LIPSYNC_VERSION_MINOR  5
#define CONF_LIPSYNC_VERSION_REV    12

const String CONF_LIPSYNC_VERSION_STRING = String(CONF_LIPSYNC_VERSION_MAJOR) + "." + String(CONF_LIPSYNC_VERSION_MINOR) + "." + String(CONF_LIPSYNC_VERSION_REV);

// I2C Devices
#define I2CADDR_DISPLAY 0x3D  // Display address (61)
#define I2CADDR_LPS22   0x5C  // Modified LPS22 address (92)
#define I2CADDR_LPS35HW 0x5D  // LPS35HW address (93)
#define I2CADDR_TLV493D 0x5E  // 3D Hall Effect Sensor (94)


// Input pins
#define CONF_BUTTON1_PIN 9                  // Pushbutton S1
#define CONF_BUTTON2_PIN 3                  // Pushbutton S2
#define CONF_SWITCH1_PIN A0                 // 3.5 mm jack SW1
#define CONF_SWITCH2_PIN A1                 // 3.5 mm jack SW2
#define CONF_SWITCH3_PIN A2                 // 3.5 mm jack SW3

// Input buttons and switch values 
#define CONF_BUTTON_NUMBER 2
#define CONF_SWITCH_NUMBER 3

// Output LEDS
#define CONF_LED_LEFT_PIN 7                 // left
#define CONF_LED_MIDDLE_PIN 8               // middle
#define CONF_LED_RIGHT_PIN 6                // right

// Onboard LEDs
#define CONF_LED_MICRO_RED LED_RED          // Xiao NRF52840 User LED Red
#define CONF_LED_MICRO_GREEN LED_GREEN      // Xiao NRF52840 User LED Green
#define CONF_LED_MICRO_BLUE LED_BLUE        // Xiao NRF52840 User LED Blue

// Output Buzzer
#define CONF_BUZZER_PIN 10

// Operating Mode Values
// 4.5.7 review: MIN is 1, not 0 — mode 0 ("None") has no beginComOpMode
// case and no sender (the app only ever sends 1-3); a stored 0 used to
// boot a device whose com-mode switch to BLE then polled an uninitialized
// Bluefruit stack every second (the exact 1 s SoftDevice stall the 4.5.x
// BLE-poll guard was written to prevent). The boot load clamps a stored 0
// to the default mouse mode.
#define CONF_OPERATING_MODE_MIN 1
#define CONF_OPERATING_MODE_NONE 0
#define CONF_OPERATING_MODE_MOUSE 1
#define CONF_OPERATING_MODE_GAMEPAD 2
#define CONF_OPERATING_MODE_SAFE 3
#define CONF_OPERATING_MODE_MAX 3
// The highest USER-SETTABLE / STORABLE operating mode (4.5.7 review): SAFE
// (3) is an internal runtime state set by checkSafeMode/toggleSafeMode and
// is NEVER a valid stored value — beginComOpMode has no SAFE case on a
// normal boot, so a persisted OM=3 would come up with g_operatingMode=SAFE
// but g_safeModeEnabled=false: no HID interface, all-red LEDs, no cursor,
// and NOT the recoverable safe menu. The setter and the boot-load clamp
// bound to GAMEPAD, not MAX.
#define CONF_OPERATING_MODE_SETTABLE_MAX CONF_OPERATING_MODE_GAMEPAD

#define CONF_OPERATING_MODE_DEFAULT CONF_OPERATING_MODE_MOUSE  // Default mode = Mouse
                                          // 0 = Operating Mode null
                                          // 1 = Mouse
                                          // 2 = Gamepad  

// Communication mode values 
#define CONF_COM_MODE_MIN  0
#define CONF_COM_MODE_NONE 0 
#define CONF_COM_MODE_USB  1
#define CONF_COM_MODE_BLE  2
#define CONF_COM_MODE_MAX  2

#define CONF_COM_MODE_DEFAULT CONF_COM_MODE_USB

#define CONF_LED_NONE 0
#define CONF_LED_LEFT 1
#define CONF_LED_MIDDLE 2
#define CONF_LED_RIGHT 3
#define CONF_LED_MICRO 4
#define CONF_LED_ALL 5

#define CONF_LED_BRIGHTNESS 150

// LED color codes
#define LED_CLR_NONE 0
#define LED_CLR_BLUE 1
#define LED_CLR_PURPLE 2
#define LED_CLR_MAGENTA 3
#define LED_CLR_PINK 4
#define LED_CLR_RED 5
#define LED_CLR_ORANGE 6
#define LED_CLR_YELLOW 7
#define LED_CLR_GREEN 8
#define LED_CLR_TEAL 9
#define LED_CLR_WHITE 10

#define LED_ACTION_NONE 0
#define LED_ACTION_OFF 1
#define LED_ACTION_ON 2
#define LED_ACTION_BLINK 3
#define LED_ACTION_BLINKFAST 4

// Display
#define CONF_DISPLAY_ENABLED false

// Button Input States
#define INPUT_MAIN_STATE_NONE           0
#define INPUT_MAIN_STATE_S1_PRESSED     1
#define INPUT_MAIN_STATE_S2_PRESSED     2
#define INPUT_MAIN_STATE_S12_PRESSED    3
#define INPUT_MAIN_STATE_S3_PRESSED     4
#define INPUT_MAIN_STATE_S13_PRESSED    5
#define INPUT_MAIN_STATE_S23_PRESSED    6
#define INPUT_MAIN_STATE_S123_PRESSED   7


// Output action numbers
#define CONF_ACTION_NOTHING 0              // No action
#define CONF_ACTION_LEFT_CLICK 1           // Generates a short left click
#define CONF_ACTION_RIGHT_CLICK 2          // Generates a short right click
#define CONF_ACTION_MIDDLE_CLICK 3         // Generates a short middle click
#define CONF_ACTION_DRAG 4                 // Initiates drag mode, holding down left click until cancelled
#define CONF_ACTION_SCROLL 5               // Initiates scroll mode. Vertical motion generates mouse scroll wheel movement.
#define CONF_ACTION_B1_PRESS 6             // Generates a Button 1 press or button X1(Left USB)/View(Right USB) press in XAC  
#define CONF_ACTION_B2_PRESS 7             // Generates a Button 2 press or button X2(Left USB)/Menu(Right USB) press in XAC    
#define CONF_ACTION_B3_PRESS 8             // Generates a Button 3 press or button LS(Left USB)/RS(Right USB) press in XAC
#define CONF_ACTION_B4_PRESS 9             // Generates a Button 4 press or button LB(Left USB)/RB(Right USB) press in XAC 
#define CONF_ACTION_B5_PRESS 10            // Generates a Button 5 press or button A(Left USB)/X(Right USB) press in XAC
#define CONF_ACTION_B6_PRESS 11            // Generates a Button 6 press or button B(Left USB)/Y(Right USB) press in XAC
#define CONF_ACTION_B7_PRESS 12            // Generates a Button 7 press or button View(Left USB)/X1(Right USB) press in XAC  
#define CONF_ACTION_B8_PRESS 13            // Generates a Button 8 press or button Menu(Left USB)/X2(Right USB) press in XAC 
#define CONF_ACTION_CURSOR_CENTER 14       // Center the joystick 
#define CONF_ACTION_CURSOR_CALIBRATION 15  // Initiates the joystick calibration.
#define CONF_ACTION_DEC_SPEED 16           // Decrease cursor movement speed
#define CONF_ACTION_INC_SPEED 17           // Increase cursor movement speed
#define CONF_ACTION_CHANGE_MODE 18         // Change communication mode
#define CONF_ACTION_START_MENU 19          // Start menu on display
#define CONF_ACTION_STOP_MENU 20           // Close menu on display
#define CONF_ACTION_NEXT_MENU_ITEM 21      // Move to next item in menu
#define CONF_ACTION_SELECT_MENU_ITEM 22    // Select current item in menu 
#define CONF_ACTION_RESET 23               // Software Reset
#define CONF_ACTION_FACTORY_RESET 24       // Factory Reset


// Flash Memory settings - Don't change
#define CONF_SETTINGS_FILE    "/settings.txt"
#define CONF_SETTINGS_JSON    "{\"MN\":0,\"VN1\":4,\"VN2\":1,\"VN3\":0,\"ID\":0,\"OM\":1,\"CM\":1,\"SS\":5,\"SL\":5,\"ST\":3.0,\"PT\":3.0,\"AV\":0,\"IZ\":0.07,\"OZ\":0.95,\"CA0\":[0.0,0.0],\"CA1\":[-13.0,13.0],\"CA2\":[13.0,13.0],\"CA3\":[13.0,-13.0],\"CA4\":[-13.0,-13.0],\"SM\":1,\"LM\":1,\"LL\":5,\"DM\":0,\"SD\":0,\"MA\":0,\"LK\":0,\"CF\":0,\"WS\":0,\"WL\":0,\"WM\":0,\"SA\":0,\"SB\":0,\"SC\":0,\"GA\":0,\"GB\":0,\"GC\":0}"

// Serialized-settings size budget (bench follow-up 2026-10-18): firmware
// 4.1.0 — the oldest firmware the app can downgrade a device to — reads
// the settings file with a 512-byte buffer and truncates anything longer,
// silently, on its next write. The file sat at ~383 B through 4.4.x, which
// is why every 4.1.0 downgrade round-trip kept working; 4.5.0's six switch
// mappings (SA/SB/SC/GA/GB/GC, ~+80 B fully populated) ate most of that
// headroom, and one more feature of that size would make a downgrade eat
// the tail of the file. 4.1.0 cannot be fixed, so the budget is enforced on
// OUR side: LSMemory refuses (FAIL,3 from the API layer) any write whose
// serialized form would push the file past this size, the EX,0:0 dump
// reports the live file size so the bench can watch the headroom, and the
// app-side host test (LipSync_Connect tests/settings-size.test.mjs) fails
// when a new key's default alone busts the budget. 480 leaves a margin
// under the 512-byte cliff for value-width growth (a threshold at 150.0, a
// 5-digit timing window, 7-digit packed switch triples) while giving every
// currently-shipped key set ~95 bytes of room.
#define CONF_SETTINGS_FILE_BUDGET 480

// Input restriction settings (roadmap item 11) — every one defaults to 0 =
// the unrestricted legacy behavior, so a settings file written by older
// firmware (missing the keys) reads 0 and keeps working exactly as before.
#define CONF_SAP_DISABLED_DEFAULT      0  // SD: 1 = sip & puff inputs disabled (upstream #21)
#define CONF_MENU_BUTTONS_ONLY_DEFAULT 0  // MA: 1 = only the hub buttons may open/navigate the menu (upstream #77)
#define CONF_MODE_LOCK_DEFAULT         0  // LK: 1 = the operating mode is locked; inputs cannot change it (upstream #68)
#define CONF_CURSOR_HOLD_DEFAULT       0  // CF: 1 = cursor movement freezes while a sip or puff is in progress (upstream #44)

// Input timing windows (roadmap item 15, upstream #62/#9): the short/long/
// very-long input window boundaries in ms. Stored 0 = the compiled default
// below — which is also what a settings file written by older firmware reads
// for the missing keys, so an update keeps the factory feel exactly.
#define CONF_INPUT_WINDOW_SHORT_DEFAULT_MS 1000
#define CONF_INPUT_WINDOW_LONG_DEFAULT_MS  3000
#define CONF_INPUT_WINDOW_MAX_DEFAULT_MS   8000
// Settable ranges (the setter also enforces short < long < max ordering).
#define CONF_INPUT_WINDOW_SHORT_MIN_MS 200
#define CONF_INPUT_WINDOW_SHORT_MAX_MS 2500
#define CONF_INPUT_WINDOW_LONG_MIN_MS  400
#define CONF_INPUT_WINDOW_LONG_MAX_MS  6000
#define CONF_INPUT_WINDOW_MAX_MIN_MS   1000
#define CONF_INPUT_WINDOW_MAX_MAX_MS  10000

// Sip & puff threshold level scale (roadmap item 15, upstream #22): the Hub
// menu steps through these coarse levels instead of ±1 hPa, so the
// mouthpiece-driven menu path is practical; the serial API and app keep the
// fine hPa granularity.
const float PRESS_THRESHOLD_LEVEL_TABLE[] = { 2.0, 3.0, 4.0, 5.0, 7.0, 10.0, 15.0, 22.0, 32.0, 45.0 };
#define PRESS_THRESHOLD_LEVEL_NUM 10

// Input source classes for evaluateOutputAction()'s restriction gates
#define INPUT_CLASS_SAP     0  // Mouthpiece sip & puff
#define INPUT_CLASS_BUTTON  1  // Hub buttons S1/S2
#define INPUT_CLASS_SWITCH  2  // External assistive switches

// Polling rates for each module
#define CONF_JOYSTICK_POLL_RATE 20          // 20 ms 
#define CONF_SCROLL_POLL_RATE 30            // 30 ms
#define CONF_PRESSURE_POLL_RATE 50          // 50 ms
#define CONF_INPUT_POLL_RATE 20             // 20 ms
#define CONF_BT_FEEDBACK_POLL_RATE 1000     // 1s         
#define CONF_DEBUG_POLL_RATE 100            // 100 ms
#define CONF_SCREEN_POLL_RATE 20            // 20 ms
#define CONF_USB_POLL_RATE 1000             // Check USB connection every 1 second
#define CONF_WATCHDOG_POLL_RATE 5000        // Reset watchdog timer every 5 seconds

#define CONF_BUTTON_PRESS_DELAY 150         // 150 ms - Duration of single button press in gamepad mode

#define CONF_WATCHDOG_TIMEOUT_SEC 30        // 30 second hardware watchdog

#define CONF_WATCHDOG_BOOT_RETRY_LIMIT 3    // Consecutive watchdog-reset boots before the watchdog stops
                                            // re-arming (device settles in safe mode instead of resetting forever)
#define CONF_WATCHDOG_BOOT_CLEAR_MS 120000  // Uptime that restores the full watchdog-reset retry budget


// Safe Boot Mode
#define CONF_SAFE_MODE_REASON_WATCHDOG 1
#define CONF_SAFE_MODE_REASON_INPUT 2
#define CONF_SAFE_MODE_REASON_HARDWARE 3
// 4.5.7 review: three-or-more consecutive boots died in a TIMER3-witnessed
// loop hang (SREQ resets preserve the retention streak — see
// LSWatchdog.h's hang-streak block). The boot rests in safe mode with the
// watchdog/witness not re-armed.
#define CONF_SAFE_MODE_REASON_HANG 4

// Hub Menu
#define CONF_SPLASH_SCREEN_DURATION 10000   // 10 seconds - how long the splash screen stays on on startup
#define CONF_SAFEMODE_MENU_TIMEOUT 3000 // 3 seconds between screens
#define CONF_MENU_TIMEOUT  300000           // 300 seconds (5 minutes) - duration of inactivity after which the screen turns off 
// 0 is NOT a control: controlHubMenu's switch has no case for it, and a
// value with no action must answer FAIL,3 — not a SUCCESS that did nothing
// (the app's remote only ever sends 1-4).
#define CONF_MENU_CONTROL_MIN 1
#define CONF_MENU_CONTROL_OPEN 1
#define CONF_MENU_CONTROL_SELECT 2
#define CONF_MENU_CONTROL_NEXT 3
#define CONF_MENU_CONTROL_CLOSE 4
#define CONF_MENU_CONTROL_MAX 4

// 4.5.7 review: menu input frozen for the center-reset flow's whole timer
// chain (start delay + 5 readings + completion margin), so a mid-flow Next
// cannot redraw the menu over the "do not move joystick" instruction and a
// mid-flow Select cannot hit selectMenuItem's unhandled page id.
#define CONF_CENTER_RESET_FREEZE_MS 3000

// Bench fix (4.5.7): destructive menu confirms (safe-menu Restart / Factory
// Reset and their confirm pages) require the select to be HELD for at least
// this long — the bench caught phantom input walking the safe menu into a
// factory reset (repeated ~40 ms ghost clicks), and a deliberate destructive
// action should be a deliberate hold anyway. The threshold must stay BELOW
// the short window in force (a release at/over it maps to the long-window
// row and never reaches selectMenuItem), so the gate computes it as half the
// effective short window, floored at CONF_MENU_CONFIRM_HOLD_MIN_MS — above
// any glitch class (a phantom click is 2-3 poll ticks), below any legal
// window. Serial CH,1 presses bypass (no hold concept on the wire).
#define CONF_MENU_CONFIRM_HOLD_MIN_MS 150

// Upper bound on a honored menu-input freeze (4.5.7 review): the longest
// real freeze is the ~3 s dwell/transition/center windows, so a remaining
// freeze longer than this must be a stale/never-set _inputFrozenUntil
// (which the signed-delta wrap would otherwise read as frozen for ~24.9
// days). Margin well above the real maximum; see LSScreen::inputIsFrozen.
#define CONF_SCREEN_INPUT_FREEZE_MAX_MS 15000


#define CONF_USB_HID_TIMEOUT  5000           // 5 seconds - timeout for connecting to USB and continuing with the program
#define CONF_USB_HID_INIT_DELAY 2000         // initial delay before attempting to reconnect to usb again
#define CONF_USB_REENUM_AFTER_MS 3000        // a persistent HID outage this old gets a forced re-enumeration (bench
                                             // 2026-10-16: post-flash boots come up configured-but-HID-dead — serial
                                             // alive, every mouse report refused — and the host never fixes it itself)
#define CONF_USB_REENUM_MAX 3                // forced re-enumerations per episode; further ticks only keep the page
#define CONF_USB_REENUM_DWELL_MS 500         // bus-dark window per forced re-enumeration — 100 ms was not always long
                                             // enough for the host to tear the old configuration down (bench 2026-10-17:
                                             // a wedge survived three 100 ms cycles), while a real restart's ~4 s always
                                             // heals; this borrows part of that margin without the reboot cost
#define CONF_USB_RESTART_MAGIC 0x4C535255    // "LSRU" — the .noinit once-guard value: set right before the USB-recovery
                                             // restart, checked at the next boot so the auto-restart can never loop

#define CONF_UPDATE_MODE_DWELL_MS 500

// 4.5.10 bench fix: the BLE init (Bluefruit.begin -> SoftDevice enable ->
// GATT table -> HID service -> advertising) runs on ITS OWN FreeRTOS task
// with this statically-allocated stack. The core's loop task (which runs
// setup() and loop()) gets a hardcoded 4096-byte stack with no functioning
// overflow detection, and the BLE boot's kilobytes of extra call depth
// silently corrupted it -> delayed HardFault -> the BLE boot loop (bench
// 2026-10-25: gamepad boots work, BLE boots fault at ~2.6-4 s with
// rst=0x4, pm=0, pt=4 every cycle). 16 KB matches the depth the stackup
// needs with wide margin; .bss cost against 202 KB free.
#define CONF_BLE_INIT_TASK_STACK_BYTES 16384
// Priority 2 == TASK_PRIO_NORMAL (rtos.h): above the loop task (1), below
// the SoftDevice event handler — the init runs promptly but never preempts
// SD critical sections.
#define CONF_BLE_INIT_TASK_PRIORITY 2
#define CONF_BLE_DEVICE_NAME_SZ 32  // "LS_" + 8-hex-char UID + NUL, with margin

// 4.5.11: on BLE boots the USB attach waits for the BLE init task's verdict
// (the SoftDevice enable is the fragile call, and Seeed's core lacks the
// Adafruit USE_TINYUSB coordination hooks between an attached USB device and
// the SD enable — the stock boot order is SD first). On a FAILED init the
// attach still happens after this fallback so serial recovery (CM,1:1) is
// always reachable.
#define CONF_BLE_INIT_USB_ATTACH_FALLBACK_MS 10000  // Bounded dwell in enterUpdateMode() between the ack, the "Update mode" splash
                                        // and the reset into the bootloader: long enough for the ack to reach the host
                                        // over USB and for the OLED framebuffer push to complete (bounded by Wire), so
                                        // the splash is the frame the OLED keeps while the bootloader owns the device

// Polling Timer IDs for each module
#define CONF_TIMER_JOYSTICK 0
#define CONF_TIMER_PRESSURE 1
#define CONF_TIMER_INPUT 2
#define CONF_TIMER_BLUETOOTH 3
#define CONF_TIMER_DEBUG 4
#define CONF_TIMER_SCROLL 5
#define CONF_TIMER_SCREEN 6
#define CONF_TIMER_USB 7
#define CONF_TIMER_WATCHDOG 8
#define CONF_TIMER_NUM 9               // Registrations at boot (ids 0..8) — the
                                        // setup() loop checks each landed a real slot

#define CONF_TIMER_LED_STARTUP 0
#define CONF_TIMER_LED_IBM 1
#define CONF_TIMER_LED_BLINK 2
#define CONF_TIMER_LED_BT 3
#define CONF_TIMER_LED_ERROR 4


// Joystick values 
#define CONF_JOY_CURSOR_SPEED_LEVEL_MIN 1
#define CONF_JOY_CURSOR_SPEED_LEVEL_MAX 10



// Debug mode values 
#define CONF_DEBUG_MODE_NONE      0 
#define CONF_DEBUG_MODE_JOYSTICK  1
#define CONF_DEBUG_MODE_PRESSURE  2
#define CONF_DEBUG_MODE_BUTTON    3
#define CONF_DEBUG_MODE_SWITCH    4
#define CONF_DEBUG_MODE_SAP       5

#define CONF_DEBUG_MODE_MIN 0
#define CONF_DEBUG_MODE_MAX 5

// Debug Default settings
#define CONF_DEBUG_MODE_DEFAULT  0        // Default debug mode state = Off 
                                          // 0 = Debug mode is Off
                                          // 1 = Joystick debug mode is On
                                          // 2 = Pressure debug mode is On
                                          // 3 = Buttons debug mode is On
                                          // 4 = Switch debug mode is On
                                          // 5 = Sip & Puff state debug mode is On


// Internal Test
#define CONF_TEST_MODE_MIN 1
#define CONF_TEST_MODE_LED 1
#define CONF_TEST_MODE_BUZZER 2
#define CONF_TEST_MODE_WATCHDOG 3
#define CONF_TEST_MODE_MAX 3


// Sound Modes                                                                                              
#define CONF_SOUND_MODE_OFF     0   // Sound off
#define CONF_SOUND_MODE_BASIC   1   // Minimal sounds
#define CONF_SOUND_MODE_ALL     2   // All sounds

#define CONF_SOUND_MODE_MIN     0
#define CONF_SOUND_MODE_MAX     2
#define CONF_SOUND_MODE_DEFAULT CONF_SOUND_MODE_BASIC


// Light Modes
#define CONF_LIGHT_MODE_OFF     0   // LEDs off
#define CONF_LIGHT_MODE_BASIC   1   // Minimal LEDs
#define CONF_LIGHT_MODE_ALL     2   // All LEDs
#define CONF_LIGHT_MODE_STATE   3   // LEDs keyed to input state, not action (roadmap item 16, upstream #43)

#define CONF_LIGHT_MODE_MIN     0
#define CONF_LIGHT_MODE_MAX     3
#define CONF_LIGHT_MODE_DEFAULT CONF_LIGHT_MODE_BASIC

// Input-state LED feedback (the LM=3 tier): while any input is held, the
// LEDs show WHICH WINDOW the hold is in, on two redundant channels — the
// three face LEDs FILL left -> middle -> right as the hold crosses each
// boundary (1/2/3: they are physically single-color, so a count is the
// one message they can carry), and the micro LED next to the USB-C port
// shows the window's COLOR (the Hub's single RGB LED). Actions are not
// one-to-one with inputs (#43), so the window is the honest thing to
// show; the fill progression also tells the user what a release will do
// without memorizing any mapping.
#define CONF_INPUT_STATE_LED_SHORT_COLOR LED_CLR_GREEN
#define CONF_INPUT_STATE_LED_LONG_COLOR  LED_CLR_YELLOW
#define CONF_INPUT_STATE_LED_MAX_COLOR    LED_CLR_RED

// Light Brightness Levels
#define CONF_LED_BRIGHTNESS_LEVEL_DEFAULT   5
#define CONF_LED_BRIGHTNESS_LEVEL_MIN       0
#define CONF_LED_BRIGHTNESS_LEVEL_MAX       10

#define CONF_LED_BRIGHTNESS_MAX 255

//***CAN BE CHANGED***//
// API
#define CONF_API_ENABLED true               // Enable or Disable API

// Startup Default settings
#define CONF_STARTUP_LED_STEP_TIME 500      // Time for each color

// Joystick Deadzone Default settings
#define CONF_JOY_DEADZONE_MIN 0.0
#define CONF_JOY_DEADZONE_INNER_DEFAULT 0.07
#define CONF_JOY_DEADZONE_OUTER_DEFAULT 0.95
#define CONF_JOY_DEADZONE_MAX 1.0

// Joystick full calibration points and related LED feedback settings
#define CONF_JOY_CALIB_CORNER_DEFAULT 13.0
#define CONF_JOY_CALIB_START_DELAY 1000              // Number of milliseconds to delay full joystick calibration once triggered
#define CONF_JOY_CALIB_START_LED_COLOR LED_CLR_RED   // Joystick Calibration process start and end color
#define CONF_JOY_CALIB_STEP_DELAY 1500               // Number of milliseconds to delay between corner measurements
#define CONF_JOY_CALIB_LED_NUMBER CONF_LED_ALL
#define CONF_JOY_CALIB_LED_COLOR LED_CLR_RED         // LED color for joystick calibration process 
#define CONF_JOY_CALIB_STEP_BLINK 1
#define CONF_JOY_CALIB_STEP_BLINK_DELAY 150
#define CONF_JOY_CALIB_READING_DELAY 200
#define CONF_JOY_CALIB_STEP_BLINK_COLOR LED_CLR_RED  // The color indicates the joystick Calibration about to start
#define CONF_JOY_CALIB_READING_NUMBER 10             // Number of readings to measure (and then average) for each calibration point  

#define CONF_JOY_CALIB_ERROR 10  // flag to display message stating there was an error with one or more corner calibrations
#define CONF_JOY_CALIB_CORNER_MIN 3  // Minimum value for a corner coordinate when completing full calibration. Less than this will be set to default.

// Joystick center initialization and related LED feedback settings 
#define CONF_JOY_INIT_START_DELAY 1000  // Number of milliseconds to delay joystick neutral calibration once triggered
#define CONF_JOY_INIT_LED_NUMBER CONF_LED_ALL
#define CONF_JOY_INIT_LED_COLOR LED_CLR_RED  // LED color for center initialization step
#define CONF_JOY_INIT_STEP_BLINK 1
#define CONF_JOY_INIT_STEP_BLINK_DELAY 150
#define CONF_JOY_INIT_STEP_BLINK_COLOR LED_CLR_RED // LED color for Center initialization start blink color 
#define CONF_JOY_INIT_READING_DELAY 100
#define CONF_JOY_INIT_READING_NUMBER 5

// Center-reset flow initiator flag (4.5.4, per-flow ack correctness): the
// center flow's timer payload carries the step number in the low bits and
// this flag bit when the HOST requested the center over the serial API
// (IN,1:1). The completion prints SUCCESS only when ITS OWN chain carries
// the flag — MANUAL otherwise — so two overlapping centers (an app
// IN,1:1 racing the boot auto-center or the Hub menu's Center Reset) each
// ack with their own correct prefix instead of sharing one global that
// whichever-fires-first consumes. Step numbers stay in the low 7 bits
// (the flow only ever uses 0 and 1).
#define CONF_CENTER_INIT_API_FLAG 0x80

// Full-calibration flow initiator flag (the 4.5.2 initiator contract's
// calibration half): the calibration chain's timer payload carries the step
// number in the low bits and this bit when the HOST started the calibration
// over the serial API (CA,1:1). The per-corner lines print SUCCESS only for
// a host-initiated run; a Hub-menu-initiated run prints MANUAL,0:CA,n with
// the same data — an unsolicited SUCCESS on the wire can falsely resolve a
// pending command waiter, exactly the straggler class 4.5.2 fixed for the
// center flow. Steps stay in the low 7 bits (the flow only ever uses 0-6).
#define CONF_CALIB_API_FLAG 0x80

// Joystick cursor speed change and related LED feedback settings 
#define CONF_JOY_CURSOR_SPEED_LEVEL_DEFAULT 5  // Default cursor speed level
#define CONF_JOY_SPEED_CHANGE_LED_DELAY 150
#define CONF_JOY_SPEED_CHANGE_LED_BLINK 1
#define CONF_JOY_SPEED_DEC_LED_NUMBER CONF_LED_LEFT // Which LED indicates decrease in cursor speed level
#define CONF_JOY_SPEED_DEC_LED_COLOR LED_CLR_RED // Decrease speed color
#define CONF_JOY_SPEED_INC_LED_NUMBER CONF_LED_RIGHT // Which LED indicates increase in cursor speed level
#define CONF_JOY_SPEED_INC_LED_COLOR LED_CLR_RED  // Increase speed color
#define CONF_JOY_SPEED_LIMIT_LED_DELAY 50
#define CONF_JOY_SPEED_LIMIT_LED_BLINK 3
#define CONF_JOY_SPEED_LIMIT_LED_COLOR LED_CLR_RED // Out of range limit speed color

#define CONF_JOY_OUTPUT_XY_MAX  1024
#define CONF_JOY_OUTPUT_XY_MAX_GAMEPAD  127

// Scroll level change and related LED feedback settings 
#define CONF_SCROLL_CHANGE_LED_DELAY  150
#define CONF_SCROLL_CHANGE_LED_BLINK  1
#define CONF_SCROLL_DEC_LED_NUMBER    CONF_LED_LEFT
#define CONF_SCROLL_DEC_LED_COLOR     LED_CLR_PURPLE  // Decrease scroll level color
#define CONF_SCROLL_INC_LED_NUMBER    CONF_LED_RIGHT
#define CONF_SCROLL_INC_LED_COLOR     LED_CLR_PURPLE  // Increase scroll level color
#define CONF_SCROLL_LIMIT_LED_DELAY   50
#define CONF_SCROLL_LIMIT_LED_BLINK   3
#define CONF_SCROLL_LIMIT_LED_COLOR   LED_CLR_RED // Out of range limit scroll level color

// Scroll level values 
#define CONF_SCROLL_LEVEL_DEFAULT 5
#define CONF_SCROLL_LEVEL_MIN 1
#define CONF_SCROLL_LEVEL_MAX 10
#define CONF_SCROLL_MOVE_MIN  1
#define CONF_SCROLL_MOVE_MAX  16
#define CONF_SCROLL_MOVE_BASE 1

// Joystick cursor acceleration change 
#define CONF_JOY_ACCELERATION_LEVEL_MAX 10
#define CONF_JOY_ACCELERATION_LEVEL_MIN -10
#define CONF_JOY_ACCELERATION_LEVEL_DEFAULT 0

// Sip and Puff Default settings
#define CONF_SIP_THRESHOLD 3.0                    // hPa
#define CONF_PUFF_THRESHOLD 3.0                   // hPa
#define CONF_PRESS_MIN_THRESHOLD 2.0              // hPa
#define CONF_PRESS_MAX_THRESHOLD 150.0            // hPa

// Pressure plausibility range (4.5.7 review): both pressure sensors read
// absolute ~1013 hPa; anything outside this band is garbage from a flaky
// ACK — the "> 0.00" validity test let garbage-positive pairs pass as
// healthy readings and fire phantom sip/puff events (the exact
// garbage-ACK failure mode the bench has recorded on unseated cables).
#define CONF_PRESS_ABS_PLAUSIBLE_MIN 300.0   // hPa
#define CONF_PRESS_ABS_PLAUSIBLE_MAX 1200.0  // hPa

// Sip/puff release hysteresis (4.5.7 review): a hold ends only when the
// pressure falls back through this fraction of the engage threshold.
// The engage boundary used to be the release boundary, so sensor noise
// while holding near a threshold oscillated NONE<->PUFF/SIP, emitting a
// RELEASED per oscillation — repeated clicks and drags that ended on
// their own. 70% is the standard mouth-interface release band. The
// hysteresis only ever HOLDS an active hold (prev STARTED) — from a
// RELEASED state, re-engagement requires the FULL threshold again, or a
// pressure that merely re-crosses the release band re-armed a phantom
// hold at 70% of the engage threshold (and fired a second action when
// it later drained).
#define CONF_PRESS_RELEASE_FRACTION 0.7
                                                                                      
// Sip and puff main states 
#define PRESS_SAP_MAIN_STATE_NONE 0   // No action 
#define PRESS_SAP_MAIN_STATE_SIP 1    // Sip action sapPressure < -sip threshold
#define PRESS_SAP_MAIN_STATE_PUFF 2   // Puff action sapPressure > puff threshold

// Inputs and related LED feedback settings
#define CONF_INPUT_LED_DELAY 150          // Led blink time for input actions 
#define CONF_INPUT_LED_BLINK 1            // Led blink number  for input actions 

// Communication Mode Default settings
#define CONF_COM_MODE_LED_NUMBER  2
#define CONF_COM_MODE_LED_BLINK   1
#define CONF_COM_MODE_LED_BLINK_DELAY   1000


// Bluetooth connection and related LED feedback settings 
#define CONF_BT_SCAN_BLINK_DELAY 500
#define CONF_BT_SCAN_BLINK_NUMBER 1
#define CONF_BT_LED_NUMBER CONF_LED_MICRO // Which LED indicate Bluetooth connection status
#define CONF_BT_LED_COLOR LED_CLR_BLUE 
#define CONF_BT_LED_BRIGHTNESS CONF_LED_BRIGHTNESS

// Error codes
#define CONF_ERROR_NONE 0
#define CONF_ERROR_USB  1
#define CONF_ERROR_I2C  2

#define CONF_ERROR_LED_BLINK_DELAY 500
#define CONF_ERROR_LED_NUMBER CONF_LED_ALL // All LEDs
#define CONF_ERROR_LED_COLOR LED_CLR_RED   // red Color
#define CONF_ERROR_LED_BRIGHTNESS 255      // Full brightness

// Acceleration
// Cursor acceleration structure
const accStruct accProperty[]{
  { 0, 1.0, 0, 0 },
  { 1, 1.0, 0, 0 },
  { 2, 1.0, 0, 0 },
  { 3, 1.0, 0, 0 },
  { 4, 1.0, 0, 0 },
  { 5, 1.0, 0, 0 },
  { 6, 1.0, 0, 0 },
  { 7, 1.0, 0, 0 },
  { 8, 1.0, 0, 0 },
  { 9, 1.0, 0, 0 }
};

/* LIPSYNC INPUT AND OUTPUT MAPPING */

// Sip and Puff Action Mapping
//  {INPUT ACTION, MOUSE MODE ACTION, GAMEPAD MODE ACTION, MENU MODE ACTION, SAFE-MODE ACTION, minTime, maxTime}
const inputActionStruct sapActionProperty[]{
  { PRESS_SAP_MAIN_STATE_NONE,        CONF_ACTION_NOTHING,      CONF_ACTION_NOTHING,    CONF_ACTION_NOTHING, CONF_ACTION_NOTHING,                      0,     0 },
  
  // Puff actions
  { PRESS_SAP_MAIN_STATE_PUFF,        CONF_ACTION_LEFT_CLICK,   CONF_ACTION_B1_PRESS,   CONF_ACTION_SELECT_MENU_ITEM, CONF_ACTION_SELECT_MENU_ITEM,    0,  1000 },
  { PRESS_SAP_MAIN_STATE_PUFF,        CONF_ACTION_DRAG,         CONF_ACTION_B3_PRESS,   CONF_ACTION_NOTHING, CONF_ACTION_NOTHING,                   1000,  3000 },
  { PRESS_SAP_MAIN_STATE_PUFF,        CONF_ACTION_START_MENU,   CONF_ACTION_START_MENU, CONF_ACTION_STOP_MENU, CONF_ACTION_NOTHING,                 3000,  8000 },

  // Sip Actions
  { PRESS_SAP_MAIN_STATE_SIP,         CONF_ACTION_RIGHT_CLICK,  CONF_ACTION_B2_PRESS,   CONF_ACTION_NEXT_MENU_ITEM, CONF_ACTION_NEXT_MENU_ITEM,       0, 1000 },
  { PRESS_SAP_MAIN_STATE_SIP,         CONF_ACTION_SCROLL,       CONF_ACTION_B4_PRESS,   CONF_ACTION_NOTHING, CONF_ACTION_NOTHING,                  1000, 3000 },  
  { PRESS_SAP_MAIN_STATE_SIP,         CONF_ACTION_MIDDLE_CLICK, CONF_ACTION_B5_PRESS,   CONF_ACTION_NOTHING, CONF_ACTION_NOTHING,                  3000, 8000 }
};

// Buttons built in to hub: S1 = Next, S2 = Select
const inputActionStruct buttonActionProperty[]{
  { INPUT_MAIN_STATE_NONE,            CONF_ACTION_NOTHING,      CONF_ACTION_NOTHING,    CONF_ACTION_NOTHING, CONF_ACTION_NOTHING,                     0,     0 },

  { INPUT_MAIN_STATE_S2_PRESSED,      CONF_ACTION_LEFT_CLICK,   CONF_ACTION_B1_PRESS,   CONF_ACTION_SELECT_MENU_ITEM,  CONF_ACTION_SELECT_MENU_ITEM,     0,  1000 },
  { INPUT_MAIN_STATE_S2_PRESSED,      CONF_ACTION_DRAG,         CONF_ACTION_B3_PRESS,   CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,                  1000,  3000 },  
  { INPUT_MAIN_STATE_S2_PRESSED,      CONF_ACTION_START_MENU,   CONF_ACTION_START_MENU, CONF_ACTION_STOP_MENU,  CONF_ACTION_NOTHING,                3000,  8000 },

  { INPUT_MAIN_STATE_S1_PRESSED,      CONF_ACTION_RIGHT_CLICK,  CONF_ACTION_B2_PRESS,   CONF_ACTION_NEXT_MENU_ITEM,  CONF_ACTION_NEXT_MENU_ITEM,       0,  1000 },
  { INPUT_MAIN_STATE_S1_PRESSED,      CONF_ACTION_SCROLL,       CONF_ACTION_B4_PRESS,   CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,                  1000,  3000 }, 
  { INPUT_MAIN_STATE_S1_PRESSED,      CONF_ACTION_MIDDLE_CLICK, CONF_ACTION_B5_PRESS,   CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,                  3000,  8000 },

  { INPUT_MAIN_STATE_S12_PRESSED,     CONF_ACTION_START_MENU,   CONF_ACTION_START_MENU, CONF_ACTION_STOP_MENU,  CONF_ACTION_NOTHING,                   0,  1000 },
};

// External Assistive Switch Jacks
const inputActionStruct switchActionProperty[]{
  { INPUT_MAIN_STATE_NONE,            CONF_ACTION_NOTHING,      CONF_ACTION_NOTHING,        CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,           0,    0 },
  
  { INPUT_MAIN_STATE_S1_PRESSED,      CONF_ACTION_LEFT_CLICK,   CONF_ACTION_B1_PRESS,       CONF_ACTION_SELECT_MENU_ITEM,  CONF_ACTION_SELECT_MENU_ITEM,  0,  1000 },
  { INPUT_MAIN_STATE_S1_PRESSED,      CONF_ACTION_DRAG,         CONF_ACTION_B3_PRESS,       CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,        1000,  3000 },
  { INPUT_MAIN_STATE_S1_PRESSED,      CONF_ACTION_START_MENU,   CONF_ACTION_START_MENU,     CONF_ACTION_STOP_MENU,  CONF_ACTION_NOTHING,      3000,  8000 },
  
  { INPUT_MAIN_STATE_S2_PRESSED,      CONF_ACTION_MIDDLE_CLICK, CONF_ACTION_B5_PRESS,       CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,           0,  1000 },
  { INPUT_MAIN_STATE_S2_PRESSED,      CONF_ACTION_NOTHING,      CONF_ACTION_B6_PRESS,       CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,        1000,  3000 },
  { INPUT_MAIN_STATE_S2_PRESSED,      CONF_ACTION_CURSOR_CENTER, CONF_ACTION_CURSOR_CENTER, CONF_ACTION_NOTHING,  CONF_ACTION_NOTHING,        3000,  8000 },
  
  { INPUT_MAIN_STATE_S3_PRESSED,      CONF_ACTION_RIGHT_CLICK,  CONF_ACTION_B2_PRESS,       CONF_ACTION_NEXT_MENU_ITEM,  CONF_ACTION_NEXT_MENU_ITEM,    0,  1000 },
  { INPUT_MAIN_STATE_S3_PRESSED,      CONF_ACTION_SCROLL,        CONF_ACTION_B4_PRESS,       CONF_ACTION_NOTHING,   CONF_ACTION_NOTHING,       1000,  3000 },
  { INPUT_MAIN_STATE_S3_PRESSED,      CONF_ACTION_MIDDLE_CLICK,  CONF_ACTION_NOTHING,        CONF_ACTION_NOTHING,   CONF_ACTION_NOTHING,       3000,  8000 },
  
  { INPUT_MAIN_STATE_S13_PRESSED,      CONF_ACTION_START_MENU,  CONF_ACTION_START_MENU,     CONF_ACTION_STOP_MENU,  CONF_ACTION_NOTHING,          0, 3000 },
};

// External switch action remapping (roadmap item 11's #20 follow-up, firmware
// 4.5.0): the MOUSE- and GAMEPAD-mode action cells of the three switch jacks
// are user-mappable settings (a clinician's ask — switches mapped to functions
// DIFFERENT from sip & puff, and separately mappable per host). The menu- and
// safe-mode columns stay compiled (menu navigation is a fixed language, and
// remapping it could strand a user outside the menu), the S13 combination row
// stays compiled, and the time windows stay the routing language (item 15).
//
// Wire/storage form: one settings key per (jack, mode) — SA/SB/SC = jacks
// 1/2/3 mouse mode, GA/GB/GC = gamepad mode — each holding the three window
// actions packed as short*10000 + long*100 + max. Stored 0 = the compiled
// default triple (which is also what a settings file written by older
// firmware reads for the missing keys, so an update keeps the factory
// mapping). The SET wire value carries CONF_SWITCH_ACTION_PACK_BASE plus the
// packed triple so it always lands on the 6-digit command shape the serial
// format validator accepts (5-digit values are not in the whitelist).
#define CONF_SWITCH_ACTION_PACK_BASE 100000

// Row indices in switchActionProperty for the remappable cells: rows 1-3 are
// jack 1 (short/long/max), 4-6 jack 2, 7-9 jack 3. (Row 0 is NONE, row 10 is
// the S13 combination — both stay compiled.)
#define CONF_SWITCH_REMAP_FIRST_ROW 1
#define CONF_SWITCH_REMAP_ROWS_PER_JACK 3

// Actions a switch cell may be remapped to — the caregiver-safe subset of the
// output vocabulary: nothing, the clicks/drag/scroll, the gamepad buttons B1-B8,
// cursor centering, the speed steps, mode change (already gated by LK) and
// menu open. Deliberately excluded: calibration (15), menu navigation
// (20-22), software reset (23) and factory reset (24) — one accidental press
// of a remapped switch must never be able to erase settings or restart the
// device.
const int CONF_SWITCH_REMAP_ALLOWED[] = { CONF_ACTION_NOTHING,
                                          CONF_ACTION_LEFT_CLICK,
                                          CONF_ACTION_RIGHT_CLICK,
                                          CONF_ACTION_MIDDLE_CLICK,
                                          CONF_ACTION_DRAG,
                                          CONF_ACTION_SCROLL,
                                          CONF_ACTION_B1_PRESS,
                                          CONF_ACTION_B2_PRESS,
                                          CONF_ACTION_B3_PRESS,
                                          CONF_ACTION_B4_PRESS,
                                          CONF_ACTION_B5_PRESS,
                                          CONF_ACTION_B6_PRESS,
                                          CONF_ACTION_B7_PRESS,
                                          CONF_ACTION_B8_PRESS,
                                          CONF_ACTION_CURSOR_CENTER,
                                          CONF_ACTION_DEC_SPEED,
                                          CONF_ACTION_INC_SPEED,
                                          CONF_ACTION_CHANGE_MODE,
                                          CONF_ACTION_START_MENU };
#define CONF_SWITCH_REMAP_ALLOWED_NUM (sizeof(CONF_SWITCH_REMAP_ALLOWED) / sizeof(CONF_SWITCH_REMAP_ALLOWED[0]))

// GAMEPAD-column exclusions (4.5.7 review): drag and scroll are MOUSE-only
// hold modes — in gamepad mode the legacy performOutputAction branch
// clobbers the hold tracker, so a gamepad cell mapped to either one wedged
// EVERY input on the device until a reboot or a serial center flow (the
// release match keys on the hold tracker the clobber just erased), and the
// action itself pressed a mouse button on an interface that was never
// begun. The compiled gamepad columns contain only B1-B8, so this list
// only ever rejects a caregiver mapping (SET answers FAIL,3; the boot-load
// sanitizer falls back to the factory cell, same as any allowlist breach).
const int CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN[] = { CONF_ACTION_DRAG,
                                                   CONF_ACTION_SCROLL };
#define CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN_NUM (sizeof(CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN) / sizeof(CONF_SWITCH_REMAP_GAMEPAD_FORBIDDEN[0]))

// LED Action for all available output actions. This maps what happens with the lights when different actions are triggered.
// ledOutputActionNumber, ledNumber, ledStartColor, ledEndColor, ledEndAction
const ledActionStruct ledActionProperty[]{
  { CONF_ACTION_NOTHING,            CONF_LED_NONE,    LED_CLR_NONE,   LED_CLR_NONE, LED_ACTION_NONE },
  { CONF_ACTION_LEFT_CLICK,         CONF_LED_LEFT,    LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_RIGHT_CLICK,        CONF_LED_RIGHT,   LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_MIDDLE_CLICK,       CONF_LED_MIDDLE,  LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_DRAG,               CONF_LED_LEFT,    LED_CLR_RED,    LED_CLR_RED,  LED_ACTION_ON },
  { CONF_ACTION_SCROLL,             CONF_LED_RIGHT,   LED_CLR_RED,    LED_CLR_RED,  LED_ACTION_ON },
  { CONF_ACTION_B1_PRESS,           CONF_LED_LEFT,    LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B2_PRESS,           CONF_LED_RIGHT,   LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B3_PRESS,           CONF_LED_LEFT,    LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B4_PRESS,           CONF_LED_RIGHT,   LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B5_PRESS,           CONF_LED_MIDDLE,  LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B6_PRESS,           CONF_LED_MIDDLE,  LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B7_PRESS,           CONF_LED_NONE,    LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_B8_PRESS,           CONF_LED_NONE,    LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_CURSOR_CENTER,      CONF_LED_MICRO,   LED_CLR_PURPLE, LED_CLR_NONE, LED_ACTION_NONE },
  { CONF_ACTION_CURSOR_CALIBRATION, CONF_LED_MICRO,   LED_CLR_PURPLE, LED_CLR_NONE, LED_ACTION_ON },
  { CONF_ACTION_DEC_SPEED,          CONF_LED_LEFT,    LED_CLR_RED,    LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_INC_SPEED,          CONF_LED_RIGHT,   LED_CLR_RED,    LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_CHANGE_MODE,        CONF_LED_NONE,    LED_CLR_NONE,   LED_CLR_NONE, LED_ACTION_NONE },
  { CONF_ACTION_START_MENU,         CONF_LED_ALL,     LED_CLR_NONE,   LED_CLR_RED,  LED_ACTION_BLINK },
  { CONF_ACTION_STOP_MENU,          CONF_LED_ALL,     LED_CLR_NONE,   LED_CLR_NONE, LED_ACTION_BLINK },
  { CONF_ACTION_NEXT_MENU_ITEM,     CONF_LED_LEFT,    LED_CLR_RED,    LED_CLR_NONE, LED_ACTION_BLINK },
  { CONF_ACTION_SELECT_MENU_ITEM,   CONF_LED_RIGHT,   LED_CLR_RED,    LED_CLR_NONE, LED_ACTION_BLINK },
  { CONF_ACTION_RESET,              CONF_LED_MICRO,   LED_CLR_RED,    LED_CLR_RED,  LED_ACTION_NONE },
  { CONF_ACTION_FACTORY_RESET,      CONF_LED_MICRO,   LED_CLR_RED,    LED_CLR_RED,  LED_ACTION_NONE }
};
