# Wire (nRF52) — LipSync patched fork

Vendored fork of the `Wire` library bundled with `Seeeduino:nrf52@1.1.13`
(the core the LipSync firmware pins). Re-validate against the core's
`libraries/Wire/` if the core version is ever bumped — see
`.github/workflows/build_firmware.yml` for the pinned version.

## Why this fork exists

The stock driver waits on TWIM event registers with no bound
(`while (!EVENTS_LASTRX && !EVENTS_ERROR);`). If the I2C bus wedges — a
sensor browned-out mid-transaction holding SDA low, a damaged interface
cable (see upstream issue #73) — the event never fires and the transaction
never returns: the firmware's main loop hangs until the hardware watchdog
resets the device (~30 s of dead USB), or forever on watchdog-less boots
in older firmware.

## What changed

- Every TWIM event wait is bounded by `WIRE_TRANSACTION_TIMEOUT_MS`
  (50 ms — ~25x a full 100 kHz transaction, so healthy buses are
  unaffected).
- On timeout, `wireRecoverBus()` runs: a bounded STOP, then the peripheral
  is disconnected and disabled, the pins are driven as GPIO for the NXP
  I2C bus-clear (I2C-bus spec 3.1.16: up to 9 SCL clocks to release a
  slave stuck mid-byte), and the TWIM is re-attached and re-enabled.
- The failing call then reports an error (`requestFrom` returns 0 bytes,
  `endTransmission` returns 4), which the firmware's sensor-health layer
  turns into "sensor unhealthy" — a slow re-probe cycle instead of a hang.
- `TwoWire::busHealthy()` reports false for a 1 s window after any recovery,
  letting firmware loops (joystick ticks, screen draws, the USB-retry page)
  back off all I2C while the bus is held — a held bus then costs one bounded
  recovery per window instead of a constant timeout tax.

`Wire.h` differs from the core's copy only by the `busHealthy()` declaration
and its comment; everything else lives in `Wire_nRF52.cpp`, marked with
`LipSync patch` comment blocks.

## How the build uses it

- CI (`build_firmware.yml`) and the interface repo's local twin
  (`dev/firmware/build-firmware.sh`) pass this folder to
  `arduino-cli compile --libraries`, which takes precedence over the
  core's bundled Wire.
- Arduino IDE builders: copy this `Wire` folder into your sketchbook
  libraries directory (`~/Documents/Arduino/libraries/` on macOS,
  `~/Arduino/libraries/` on Windows/Linux) so it shadows the core's copy.
- Without this library the firmware still functions; the always-armed
  hardware watchdog (firmware 4.2.0+) recovers a wedged bus in ~30 s.
