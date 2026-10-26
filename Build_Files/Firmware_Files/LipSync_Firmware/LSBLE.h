/* 
* File: LSBLE.h
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
#include <bluefruit.h>

#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)

BLEDis bledis;
BLEHidAdafruit blehid;
bool needsInitialization = true;

class LSBLEMouse {

  public:
    inline LSBLEMouse(void);
    inline void begin(const char* s = "LipSync");
    inline void end(void);
    inline void move(signed char x, signed char y);
    inline void moveAll(signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
    inline void scroll(signed char wheel = 0);
    inline void pan(signed char pan = 0);
    inline void click(uint8_t b = MOUSE_LEFT);
    inline void press(uint8_t b = MOUSE_LEFT);   // press LEFT by default
    inline void release(uint8_t b = MOUSE_LEFT); // release LEFT by default
    inline bool isPressed(uint8_t b = MOUSE_LEFT); // check LEFT by default
    inline bool isConnected(void);
  protected:
    uint8_t _buttons;
    void buttons(uint8_t b);
  private:
    void mouseReport(signed char b, signed char x, signed char y, signed char wheel = 0, signed char pan = 0);
};

// LSBLEKeyboard removed (4.5.7 review): never instantiated anywhere — a
// v4.0-era leftover whose keyboardReport() carried a delay(2) per report,
// the same copy-paste-invitation class that motivated 4.5.4's LSUSBKeyboard
// removal. (The btKeyReport struct went with it.)

void initializeBluefruit(const char* s) {
  // 4.5.9 bench round 3: every begin()/start() is CHECKED and the failure is
  // printed. A failed SoftDevice enable previously cascaded into the
  // GATT/DIS/HID/advertising SVC calls on a dead stack — an SVC call with
  // the SD down HardFaults, the Seeed core's HardFault handler is an
  // INSTANT reset, and the BLE boot looped forever (bench 2026-10-25: the
  // first real CM=2 boot on this firmware line — every earlier CM=2 boot was
  // a safe-mode CDC-only boot that never touched Bluefruit). On failure we
  // bail with needsInitialization still true: every BLE call site is
  // guarded (isConnected, btFeedbackLoop), no SVC call is made, and the
  // boot trace names the failure instead of faulting.
  if (!Bluefruit.begin()) {
    g_bleInitStage = 1;
    Serial.println("BOOT: BLE FAILED at SoftDevice enable");
    return;
  }
  Serial.println("BOOT: BLE stack up");
  Bluefruit.Periph.setConnInterval(9, 16);  // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(4);                  // Check bluefruit.h for supported values
  Bluefruit.setName(s);
  bledis.setManufacturer("MakersMakingChange");
  bledis.setModel("LipSync Mouse");
  if (!bledis.begin()) {
    g_bleInitStage = 2;
    Serial.println("BOOT: BLE FAILED at DIS begin");
    return;
  }
  if (!blehid.begin()) {
    g_bleInitStage = 3;
    Serial.println("BOOT: BLE FAILED at HID begin");
    return;
  }
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  // 4.5.12 bench fix: ONE appearance, exactly like the core's stock
  // blehid_mouse example. The 4.1.0-era sequence added BOTH keyboard and
  // mouse appearances, and the arithmetic sits exactly at the 31-byte
  // legacy advertising limit: flags(3) + txpower(3) + appearance(4)x2 +
  // service UUID(4) + name "LS_xxxxxxxx"(13) = 31 with ZERO margin — any
  // device whose name runs one byte longer makes addName/start fail. The
  // advertised appearance is cosmetic; the HID service still carries the
  // full keyboard+mouse report descriptors untouched (the wire HID
  // contract is unchanged).
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // 20 ms - in unit of 0.625 ms (Interval:  fast mode = 20 ms, slow mode = 152.5 ms)
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  if (!Bluefruit.Advertising.start(0)) {         // 0 = Don't stop advertising after n seconds
    g_bleInitStage = 4;
    Serial.println("BOOT: BLE FAILED at advertising start");
    return;
  }
  Serial.println("BOOT: BLE advertising");
  g_bleInitStage = 5;
  needsInitialization = false;  // FULL SUCCESS ONLY — every guarded call site keys on this
}


/*****************************
     MOUSE SECTION
 *****************************/

LSBLEMouse::LSBLEMouse(void)
{

}

void LSBLEMouse::begin(const char* s)
{
  _buttons = 0;
  if (needsInitialization) {
    initializeBluefruit(s);
    // needsInitialization is now cleared by initializeBluefruit itself, ONLY
    // on full success (4.5.9) — a failed begin leaves every guarded call site
    // safely reporting "not connected" instead of touching a dead stack.
  }
}


void LSBLEMouse::end(void)
{
}

void LSBLEMouse::mouseReport(int8_t b, int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
  // 4.5.10: the BLE stack comes up on its own task now (see the 16 KB
  // init task in LipSync_Firmware.ino) — an SVC call on a never-begun or
  // still-enabling SoftDevice HardFaults, which is a whole boot class
  // worse than a dropped report. Skip until needsInitialization clears;
  // the buttons state still mutates, so the first post-init movement
  // report carries it.
  if (needsInitialization) return;
  blehid.mouseReport(b, x, y, wheel, pan);
}

void LSBLEMouse::move(int8_t x, int8_t y)
{
  mouseReport(_buttons, x, y, 0, 0);
}
void LSBLEMouse::moveAll(int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
  mouseReport(_buttons, x, y, wheel, pan);
}

void LSBLEMouse::scroll(int8_t wheel)
{
  mouseReport(_buttons, 0, 0, wheel, 0);
}

void LSBLEMouse::pan(int8_t pan)
{
  mouseReport(_buttons, 0, 0, 0, pan);
}

void LSBLEMouse::click(uint8_t b)
{
  _buttons = b;
  mouseReport(_buttons, 0, 0, 0, 0);
  _buttons = 0;
  mouseReport(_buttons, 0, 0, 0, 0);
}

void LSBLEMouse::buttons(uint8_t b)
{
  if (b != _buttons)
  {
    _buttons = b;
    mouseReport(_buttons, 0, 0, 0, 0);
  }
}

void LSBLEMouse::press(uint8_t b)
{
  buttons(_buttons | b);
}

void LSBLEMouse::release(uint8_t b)
{
  buttons(_buttons & ~b);
}

bool LSBLEMouse::isPressed(uint8_t b)
{
  if ((b & _buttons) > 0)
    return true;
  return false;
}

bool LSBLEMouse::isConnected(void) {
  // 4.5.7 bench fix: Bluefruit.connected() on a SoftDevice that was never
  // begun (needsInitialization true) HANGS the firmware — the bench caught
  // it live: CM,1:2 on a USB session reached setLedDefault() BEFORE the
  // hand-off's btmouse.begin(), isConnected() never returned, no ack ever
  // printed, and the 30 s watchdog killed the device. Every caller that can
  // run pre-begin (setLedDefault's BLE branch, btFeedbackLoop) now gets an
  // honest "not connected" instead of a dead loop.
  if (needsInitialization) return false;
  return Bluefruit.connected();
}



// (KEYBOARD SECTION removed with LSBLEKeyboard — 4.5.7 review:
//  never instantiated, and its keyboardReport carried delay(2) per
//  report — the copy-paste-invitation class 4.5.4's LSUSBKeyboard
//  removal set the precedent for)
