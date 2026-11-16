#ifndef LSI2CSCAN_H
#define LSI2CSCAN_H

#include <stdint.h>
#include <stdio.h>  // snprintf (the warning-line composer; host suite included)

// Pure decision layer for the boot I2C scan (checkI2C) and the hardware
// safe-mode wire line (toggleSafeMode), split out so the host suite can pin
// the semantics — the same pattern as LSHostSwitch.h / LSMemorySeedGate.h.
// Deliberately compiles without LSConfig.h or any Arduino header (stdio only),
// so the constants here are the single source the firmware and the suite both
// read; the .ino includes this file directly.
//
// THE FIELD MYSTERY THIS ENCODES (bench + app log 2026-11-15/16, the "two
// error beeps vs one" observation): the hub PCB carries NO external I2C
// pull-ups (the schematic's PIN_SDA/PIN_SCL nets connect only the XIAO module
// and the two cable connectors), so from power-up until setup()'s Wire.begin()
// configures the nRF52's internal pull-ups — hundreds of ms of bootloader plus
// early setup — both lines float while the bus's four slaves are powered and
// listening. A slave that powers up or is left stranded by the previous
// session's NVIC_SystemReset() (which can land mid-transaction: the reset
// dwell historically left the 20 ms screen poll and the 20/50 ms
// joystick/pressure polls transacting) into a non-idle protocol state answers
// its address with a CLEAN NACK — and a clean NACK never reaches the vendored
// Wire's timeout recovery (only a timeout does), so the stuck state survives
// every warm reset. The old single-shot scan then condemned the whole session
// to safe mode on the first pass, and because a safe boot generates zero I2C
// traffic, it healed nothing: only the scan probes' own clock pulses re-align
// the slaves, one device class per boot — which is exactly why the user saw
// "two beeps" (all four devices NACKed), then "one beep" (sensors recovered,
// display still NACKing), and only a further restart reached a healthy boot.
// The firmware now (a) brings the bus up with a proactive bus-clear at the top
// of setup() before ANY transaction, (b) retries the scan a bounded number of
// passes with a settle + bus-clear between passes before declaring a device
// missing, (c) quiesces every I2C-driving poll and bus-clears before the
// deliberate reset so the successor boot inherits an idle bus, and (d) names
// the missing-hardware set on the wire in the heartbeat's own sns= encoding.

// Scan passes before a NACK becomes a verdict. 3 total passes: the first is
// the old single-shot behavior (so a healthy boot costs nothing extra — it
// exits after pass 1), and two more cover the observed healing cadence (the
// sensors recovered one boot apart in the field episode; within one boot the
// probes' clock pulses plus the inter-pass bus-clear do that healing work).
#define CONF_I2C_SCAN_PASSES 3
// The settle delay between passes (ms): long enough for a slave released by
// the bus-clear to exit its recovery, short enough that the worst-case
// all-NACK boot grows by ~(PASSES-1) x SETTLE plus probe time (~50 ms total).
#define CONF_I2C_SCAN_SETTLE_MS 20

// The sns= nibble, in the heartbeat line's digit order. THE DIGIT ORDER IS
// LOAD-BEARING: the HB line prints sns=<joystick><mouthpiece><ambient><display>
// (LipSync_Firmware.ino debugLoop), so a mask rendered most-significant-digit
// first must read identically — joystick is the HIGH bit, display the LOW bit.
// The 4.6.0 bench's "sns=0001" (interface cable out: joystick + both pressures
// absent, display present) is 0x1; the field episode's all-missing boot is
// 0x0 and its display-only-missing boot is 0xE ("1110": joystick, mouthpiece
// and ambient present, display absent).
#define LSI2C_SNS_DISPLAY   0x1
#define LSI2C_SNS_AMBIENT   0x2
#define LSI2C_SNS_MOUTHPIECE 0x4
#define LSI2C_SNS_JOYSTICK  0x8
#define LSI2C_SNS_ALL       0xF

static inline uint8_t lsi2cSnsMask(int joystickConnected, int mouthpieceConnected,
                                   int ambientConnected, int displayConnected) {
  return (uint8_t)((joystickConnected ? LSI2C_SNS_JOYSTICK : 0)
                 | (mouthpieceConnected ? LSI2C_SNS_MOUTHPIECE : 0)
                 | (ambientConnected ? LSI2C_SNS_AMBIENT : 0)
                 | (displayConnected ? LSI2C_SNS_DISPLAY : 0));
}

// The scan loop's continuation decision, pinned by the host suite:
//   - missingMask == 0 → stop: every device answered; a healthy boot never
//     pays for a second pass (and never emits a heal note — the silence
//     contract).
//   - passesDone >= CONF_I2C_SCAN_PASSES → stop: the budget is the guarantee
//     that a truly absent device delays boot by a bounded ~(PASSES-1) x
//     (SETTLE + probe) and no more.
//   - otherwise one more pass: the inter-pass settle + bus-clear plus the
//     previous pass's own address clocks are the realignment mechanism.
// passesDone counts COMPLETED passes (0 = none yet: the first pass is
// unconditional and does not consult this helper).
static inline int lsi2cScanWantsAnotherPass(int passesDone, uint8_t missingMask) {
  return (missingMask != 0) && (passesDone < CONF_I2C_SCAN_PASSES);
}

// The hardware safe-mode line's worst-case length + NUL: the fixed prefix
// "WARNING: SAFE MODE ENABLED - Hardware" (38) + " (sns=xxxx)" (11) + NUL.
#define LSI2C_SAFE_MODE_HARDWARE_WARNING_BUF 50

// Compose the hardware safe-mode warning WITH the missing-hardware set named
// in the heartbeat's own encoding (4.9.8, fix E). The bare line forced every
// field reader to re-derive which device class was absent — the 2026-11-15
// "two beeps" mystery was a full sns=0000 hiding behind a generic string.
// Returns the composed length (excluding NUL), or a negative value when the
// caller's buffer is too small — the caller forwards the line UNCHANGED to
// the TX queue rather than sending a truncated one.
static inline int lsi2cSafeModeHardwareWarning(char* buf, int bufSize,
                                               int joystickConnected,
                                               int mouthpieceConnected,
                                               int ambientConnected,
                                               int displayConnected) {
  const uint8_t sns = lsi2cSnsMask(joystickConnected, mouthpieceConnected,
                                   ambientConnected, displayConnected);
  const int written = snprintf(buf, (size_t)bufSize,
                               "WARNING: SAFE MODE ENABLED - Hardware (sns=%d%d%d%d)",
                               (sns & LSI2C_SNS_JOYSTICK) ? 1 : 0,
                               (sns & LSI2C_SNS_MOUTHPIECE) ? 1 : 0,
                               (sns & LSI2C_SNS_AMBIENT) ? 1 : 0,
                               (sns & LSI2C_SNS_DISPLAY) ? 1 : 0);
  if (written < 0 || written >= bufSize) return -1;  // encode/truncation refusal
  return written;
}

#endif  // LSI2CSCAN_H
