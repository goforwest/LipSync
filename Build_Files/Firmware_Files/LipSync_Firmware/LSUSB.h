/* 
* File: LSUSB.h
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

#include "Adafruit_TinyUSB.h"
#pragma once

#define ATTRIBUTE_PACKED  __attribute__((packed, aligned(1)))

#define RID_KEYBOARD 1
#define RID_MOUSE 2
  
#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)
#define MOUSE_DESCRIPTOR "LipSync Mouse" // TODO 2025-Feb-21 Unused due to Tiny USB library hang

// Consecutive not-ready report ticks before latching an error episode
// (3 ticks = 60 ms at the 20 ms poll): a busy HID endpoint — a report in
// flight, the host about to poll it — frees within a tick or two; only a
// real outage persists. See LSUSBMouse::mouseReport.
#define CONF_MOUSE_EP_BUSY_SKIP_LIMIT 3

#define GAMEPAD_DESCRIPTOR "LipSync Gamepad" // TODO 2025-Feb-21 Unused due to Tiny USB library hang

extern unsigned int g_usbAttempt;  // global variable to keep track of USB connection attempts
extern void usbCheckConnection(void);


// https://github.com/hathach/tinyusb/blob/master/examples/device/hid_generic_inout/src/usb_descriptors.c

//uint8_t const _ascii2keycode[128][2] = {HID_ASCII_TO_KEYCODE};

uint8_t const mouse_desc_hid_report[] =
{
    TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(RID_KEYBOARD) ),
    TUD_HID_REPORT_DESC_MOUSE( HID_REPORT_ID(RID_MOUSE) )
};


class LSUSBMouse {
  private:
    void mouseReport(signed char b, signed char x, signed char y, signed char wheel = 0,signed char pan = 0); 
  public:
    inline LSUSBMouse(void);
    inline void begin(void);
    inline void end(void);
    inline void wakeup(void);
    inline void move(signed char x, signed char y);
    inline void moveAll(signed char x, signed char y, signed char wheel = 0,signed char pan = 0); 
    inline void scroll(signed char wheel = 0);
    inline void pan(signed char pan = 0); 
    inline void click(uint8_t b = MOUSE_LEFT);
    inline void press(uint8_t b = MOUSE_LEFT);     // press LEFT by default
    inline void release(uint8_t b = MOUSE_LEFT);   // release LEFT by default
    inline bool isPressed(uint8_t b = MOUSE_LEFT); // check LEFT by default
    inline bool isReady(void);
    inline bool isConnected(void);
    bool usbRetrying = false;
    bool showTestPage = false;
    bool timedOut = false;
    // HID telemetry (bench 2026-10-16: cursor dead on 4.5.0 while the ready
    // page showed, sensor read fine and serial worked — the one fact that
    // splits "reports never attempted" from "reports attempted and refused"
    // from "reports accepted by TinyUSB but ignored by the host"). Counted
    // in mouseReport(), reported through BH,0:0.
    uint32_t reportAttempts = 0;
    uint32_t reportSuccesses = 0;
    uint32_t nonzeroReports = 0;      // Reports that carried actual movement (x or y != 0)
    int16_t maxAbsReport = 0;         // Largest |x|/y| ever sent — 0 forever means the report PIPELINE sends zeros
    uint8_t consecutiveSkips = 0;     // Not-ready ticks in a row — persistence separates a transient busy endpoint from a real outage
    unsigned long _lastReportAttemptMs = 0;
  protected:
    uint8_t _buttons;
    bool _begun = false;   // HID interface registered on the USB configuration — must happen exactly once (see begin())
    void buttons(uint8_t b);
    Adafruit_USBD_HID usb_hid;
};

// LSUSBKeyboard removed (4.5.4 dead-code cleanup): the class was never
// instantiated anywhere in the sketch — a v4.0-era leftover whose
// keyboardReport() still carried an unbounded while(!isReady()) delay(1)
// spin, the exact bug class the 4.4.0+ HID rework eradicated from every
// live path. Unreachable code cannot hang, but it also must not sit in
// the file as a copy-paste invitation. RID_KEYBOARD and the keyboard
// report descriptor in mouse_desc_hid_report[] stay: they define the
// composite device's report IDs on the wire and the descriptor must
// remain byte-identical for hosts.

typedef struct ATTRIBUTE_PACKED {
    uint8_t buttons;
    uint8_t	xAxis;
    uint8_t	yAxis;
} HID_GamepadReport_Data_t;

// HID report descriptor for XAC Compatible gamepad with 8 buttons and 2 axis joystick 
uint8_t const gamepad_desc_hid_report[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Gamepad)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x08,        //   Usage Maximum (0x08)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};


class LSUSBGamepad {
  public:
    inline LSUSBGamepad(void);
    inline void begin(void);
    inline void end(void);
    inline void wakeup(void);
    inline void send(void);
    inline bool GamepadReport(void* data, size_t length) {
        return this->usb_hid.sendReport(0, data, (uint8_t)length);
    };
    inline void write(void);
    inline void write(void *report);
    inline void press(uint8_t b);
    inline void release(uint8_t b);
    inline void releaseAll(void);
    inline void buttons(uint8_t b);
    inline void xAxis(uint8_t a);
    inline void yAxis(uint8_t a);
    inline void move(uint8_t x,uint8_t y);
    inline bool isReady(void);
    inline bool isConnected(void);
    bool usbRetrying = false;
    bool timedOut = false;
    // HID telemetry — the gamepad twin of LSUSBMouse's counters (bench
    // 2026-10-18, gamepad-mode validation): in gamepad mode BH,0:0 used to
    // read the MOUSE's interface unconditionally, so hidReady read 0 and
    // attempts/successes read 0 forever while the gamepad happily reported —
    // every health check and stuck-boot forensics lied about a working
    // device, and would say nothing at all about a broken one (the exact
    // "attempts climbing, successes 0" signature that decided the 4.4.0
    // post-flash wedges was invisible in gamepad mode). Counted in send()/
    // write() at the same points the mouse counts in mouseReport(): attempts
    // before the readiness gate (refused reports are the signal), successes
    // after TinyUSB accepts the report, and the movement stats from the
    // axis deltas actually carried. Reported through BH,0:0.
    uint32_t reportAttempts = 0;
    uint32_t reportSuccesses = 0;
    uint32_t nonzeroReports = 0;      // Reports that carried actual axis movement (x or y != center 128)
    int16_t maxAbsReport = 0;         // Largest |x|/|y| delta ever sent — 0 forever means the report PIPELINE sends centered reports
  protected:
    HID_GamepadReport_Data_t _report;
    uint32_t startMillis;
    bool _begun = false;   // HID interface registered on the USB configuration — must happen exactly once (see begin())
    Adafruit_USBD_HID usb_hid;
    inline void countReportMovement(void);
};




/*****************************
 *   MOUSE SECTION
 *****************************/ 

LSUSBMouse::LSUSBMouse(void)
{

}

void LSUSBMouse::begin(void)
{
  _buttons = 0;
  // Register the HID interface on the USB configuration exactly once.
  // Adafruit_USBD_HID::begin() calls TinyUSBDevice.addInterface() with no
  // already-begun guard (unlike the CDC class), so re-calling it from the
  // connect-retry path appended a duplicate HID interface to the descriptor
  // — and the next enumeration then presented a corrupted configuration to
  // the host, which can leave the composite device half-configured (serial
  // alive, HID never ready). Every retry must be a mount check only.
  if (!_begun) {
    this->usb_hid.setPollInterval(1);
    this->usb_hid.setReportDescriptor(mouse_desc_hid_report, sizeof(mouse_desc_hid_report));
    //this->usb_hid.setStringDescriptor(MOUSE_DESCRIPTOR); // TODO this causes TinyUSB to crash 2025-Jan-20
    this->usb_hid.begin();
    _begun = true;
  }
  if (USB_DEBUG) { Serial.println("USBDEBUG: Initializing USB HID Mouse");  }

  unsigned long timerHidTimeoutBegin = millis();
  unsigned long usbTimeoutMillis;

  g_usbAttempt++;
    
  if (usbRetrying) {
    usbTimeoutMillis = 200;                     // when reattemping to connect the USB, uses a smaller value to not freeze up the code for long but will keep retrying 
  } else {
    usbTimeoutMillis = CONF_USB_HID_TIMEOUT;    // the first time the usb tried to connect, use the default timeout
  }
  
  while( !USBDevice.mounted() ) { //  Wait for USB device to mount
    delay(1);

    if ((millis() - timerHidTimeoutBegin) > usbTimeoutMillis) { 
      usbRetrying = true;
      break;
    } 
  }

  if (USBDevice.mounted()) { // If USB device mounts, send blank report
    usbRetrying = false;
    g_usbAttempt = 0;
    move(0,0);
  }
  
}


bool LSUSBMouse::isConnected(void) {
  return this->usb_hid.ready() && !USBDevice.suspended();
}


void LSUSBMouse::end(void)
{
}

void LSUSBMouse::wakeup(void)
{
	if ( USBDevice.suspended() )  {
      USBDevice.remoteWakeup();
    }
}

void LSUSBMouse::mouseReport(int8_t b, int8_t x, int8_t y, int8_t wheel, int8_t pan) 
{
	// Never block on HID readiness. The old while(!isReady()) delay(1) spin
  // sat on the joystick-poll path and the connect-retry path, so any window
  // where the HID interface was not ready while the serial session was
  // alive — exactly the post-flash / post-bootloader re-enumeration state,
  // or a host that half-configured the composite device after a retry
  // re-added the HID interface — froze the whole firmware for up to
  // CONF_USB_HID_TIMEOUT (5 s) with the recursion re-arming the retry timer
  // from inside the spin. A live serial session died right there: the first
  // one or two host exchanges overlapping the freeze timed out waiting for
  // a response (the observed post-flash SS/SL restore drops, firmware-update
  // improvements item 2). Skipping the report costs one lost cursor update
  // while the connect machinery recovers the interface.
	wakeup();
  _lastReportAttemptMs = millis();
  reportAttempts++;
  if (!isReady()) {
    // A not-ready endpoint is not always an outage: a report still in flight
    // (the host a millisecond from polling the interrupt endpoint) reads
    // busy too, and a serial burst can stretch that window by a tick — the
    // old 4.1.0 spin waited it out invisibly, but the 4.4.0 skip latched
    // every transient busy tick into an error episode and drew the No-USB
    // page mid-health-check while the cursor kept working (bench 2026-10-16).
    // Latch only on persistence: a busy endpoint frees within a tick or two,
    // a real outage never does. Skipped busy ticks still cost nothing — the
    // pending report already carries the movement.
    if (++consecutiveSkips < CONF_MOUSE_EP_BUSY_SKIP_LIMIT) return;
    if (!timedOut) {
      // Episode start: latch the timeout state (consumed by errorCheck and
      // usbCheckConnection) and hand recovery to the connect machinery once —
      // the latch keeps the handoff (and any nested report from begin()) from
      // recursing.
      timedOut = true;
      showTestPage = true;
      usbCheckConnection();
    }
    return;
  }
  consecutiveSkips = 0;
  if (usb_hid.mouseReport(RID_MOUSE,b,x,y,wheel,pan)) {
    reportSuccesses++;
    if (x != 0 || y != 0) {
      nonzeroReports++;
      int16_t absX = (int16_t)abs(x);
      int16_t absY = (int16_t)abs(y);
      if (absX > maxAbsReport) maxAbsReport = absX;
      if (absY > maxAbsReport) maxAbsReport = absY;
    }
  }
  timedOut = false;
}

void LSUSBMouse::move(int8_t x, int8_t y) 
{
    mouseReport(_buttons, x, y, 0, 0);
}
void LSUSBMouse::moveAll(int8_t x, int8_t y, int8_t wheel, int8_t pan) 
{
    mouseReport(_buttons, x, y, wheel, pan);
}

void LSUSBMouse::scroll(int8_t wheel) 
{
    mouseReport(_buttons, 0, 0, wheel, 0);
}

void LSUSBMouse::pan(int8_t pan) 
{
    mouseReport(_buttons, 0, 0, 0, pan);
}

void LSUSBMouse::click(uint8_t b)
{
  _buttons = b;
  mouseReport(_buttons, 0, 0, 0, 0);
  _buttons = 0;
  mouseReport(_buttons, 0, 0, 0, 0);
}

void LSUSBMouse::buttons(uint8_t b)
{
	if (b != _buttons)
	{
	  _buttons = b;
	  mouseReport(_buttons, 0, 0, 0, 0);
	}
}

void LSUSBMouse::press(uint8_t b) 
{
	buttons(_buttons | b);
}

void LSUSBMouse::release(uint8_t b)
{
	buttons(_buttons & ~b);
}

bool LSUSBMouse::isPressed(uint8_t b)
{
	if ((b & _buttons) > 0) 
	  return true;
	return false;
}

bool LSUSBMouse::isReady(void)
{
	if (usb_hid.ready()) 
	  return true;
	return false;
}

/*****************************
 *   GAMEPAD SECTION
 *****************************/ 
LSUSBGamepad::LSUSBGamepad(void)
{
  this->usb_hid.setPollInterval(1);
  this->usb_hid.setReportDescriptor(gamepad_desc_hid_report, sizeof(gamepad_desc_hid_report));
  //this->usb_hid.setStringDescriptor(GAMEPAD_DESCRIPTOR); // TODO this causes TinyUSB to crash 2025-Jan-20
}

void LSUSBGamepad::begin(void)
{
  // Register the HID interface exactly once — see LSUSBMouse::begin(): a
  // retry must never re-add the interface to the USB configuration.
  if (!_begun) {
    this->usb_hid.begin();
    _begun = true;
  }

  unsigned long timerHidTimeoutBegin = millis();
  unsigned long usbTimeoutMillis;

  g_usbAttempt++;
    
  if (usbRetrying) {
    usbTimeoutMillis = 200;                     // when reattemping to connect the USB, uses a smaller value to not freeze up the code for long but will keep retrying 
  } else {
    usbTimeoutMillis = CONF_USB_HID_TIMEOUT;    // the first time the usb tried to connect, use the default timeout
  }
  
  while( !USBDevice.mounted() ) {
    delay(1);

    if ((millis() - timerHidTimeoutBegin) > usbTimeoutMillis){
      usbRetrying = true;
      break;
    } 
  }

  if (USBDevice.mounted()) {
    usbRetrying = false;
    g_usbAttempt = 0;
  }
  
  // Release all the buttons and center joystick
  end();
  startMillis = millis();
}

void LSUSBGamepad::send(void)
{
  if (startMillis != millis()) {
  wakeup();

  // Never block on HID readiness (see LSUSBMouse::mouseReport()): the old
  // 5-second spin refired on every send while the interface was down,
  // freezing the whole firmware against its serial session. Flag the
  // retry path instead and skip this report.
  reportAttempts++;  // Counted before the gate: a refused report is the diagnostic signal, not a non-event
  if (!isReady()) {
    usbRetrying = true;
    // Refused while the host had already configured the composite — the
    // configured-but-HID-dead signature (bench 2026-10-16). A simple latch
    // is enough here: the retry/re-enumeration machinery is driven by
    // usbRetrying through usbCheckConnection, exactly as before; the latch
    // only makes the refusal visible in BH. Cleared by any accepted report.
    if (USBDevice.mounted()) timedOut = true;
    return;
  }
  if (GamepadReport(&_report, sizeof(_report))) {
    reportSuccesses++;
    countReportMovement();
    timedOut = false;
  }
    startMillis = millis();
  }
}

void LSUSBGamepad::end(void)
{
  _report.buttons = 0;
  _report.xAxis = 128;
  _report.yAxis = 128;
  GamepadReport(&_report, sizeof(_report));
}

void LSUSBGamepad::wakeup(void)
{
	if ( USBDevice.suspended() )  {
      USBDevice.remoteWakeup();
    }
}

void LSUSBGamepad::write(void)
{
  wakeup();

  // Never block on HID readiness — see send().
  reportAttempts++;
  if (!isReady()) {
    usbRetrying = true;
    if (USBDevice.mounted()) timedOut = true;
    return;
  }
  if (GamepadReport(&_report, sizeof(_report))) {
    reportSuccesses++;
    countReportMovement();
    timedOut = false;
  }
}

void LSUSBGamepad::write(void *report)
{
  wakeup();

  // Never block on HID readiness — see send().
  reportAttempts++;
  if (!isReady()) {
    usbRetrying = true;
    if (USBDevice.mounted()) timedOut = true;
    return;
  }
  memcpy(&_report, report, sizeof(_report));
  if (GamepadReport(&_report, sizeof(_report))) {
    reportSuccesses++;
    countReportMovement();
    timedOut = false;
  }
}

// Movement accounting for an accepted report (the mouse counts this inside
// mouseReport(); the gamepad reports from three gated paths, so the shared
// helper reads the just-sent _report). Axes are 128-centered: the delta from
// center is the movement actually carried.
void LSUSBGamepad::countReportMovement(void) {
  int16_t absX = (int16_t)abs((int)_report.xAxis - 128);
  int16_t absY = (int16_t)abs((int)_report.yAxis - 128);
  if (absX > 0 || absY > 0) nonzeroReports++;
  if (absX > maxAbsReport) maxAbsReport = absX;
  if (absY > maxAbsReport) maxAbsReport = absY;
}

void LSUSBGamepad::press(uint8_t b)
{
  b &= 0x7; // Limit value between 0..7
  _report.buttons |= (uint8_t)1 << b;
}


void LSUSBGamepad::release(uint8_t b)
{
  b &= 0x7; // Limit value between 0..7
  _report.buttons &= ~((uint8_t)1 << b);
}


void LSUSBGamepad::releaseAll(void)
{
  _report.buttons = 0;
}

void LSUSBGamepad::buttons(uint8_t b)
{
  _report.buttons = b;
}


void LSUSBGamepad::xAxis(uint8_t a)
{
  _report.xAxis = 128 + a;
}


void LSUSBGamepad::yAxis(uint8_t a)
{
  _report.yAxis = 128 + a;
}

void LSUSBGamepad::move(uint8_t x,uint8_t y)
{
  _report.xAxis = 128 + x;
  _report.yAxis = 128 + y;
}

bool LSUSBGamepad::isReady(void)
{
	if (usb_hid.ready()) 
	  return true;
	return false;
}

bool LSUSBGamepad::isConnected(void) {
  	if (usb_hid.ready()) { 
	    return true;
    } else {
      return false;
    }	
}
