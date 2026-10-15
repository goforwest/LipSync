# TLV493D-A1B6 — LipSync patched fork

Vendored fork of Infineon's deprecated
`TLV493D-A1B6-3DMagnetic-Sensor` library (v1.0.4, the master snapshot the
project kept as `TLV493D-A1B6.zip`; upstream now points at the XENSIV
TLx493D successor — see roadmap item 2 for the eventual migration). Pinned
into the build via the `--libraries` flag exactly like the vendored `Wire`
(see `.github/workflows/build_firmware.yml` and
`LipSync_Connect/dev/firmware/build-firmware.sh`), so CI, local builds and
releases all compile this copy — never whatever TLV493D library happens to
be installed in a user's Arduino15.

## Why this fork exists

The stock library blocks without a bound on the hot path: the joystick
poll (`LSJoystick::update()`, every 20 ms) runs the sensor in
POWERDOWNMODE, so each `updateData()` performs a mode-switch write, a
**busy-wait of the configured measurement period**, the read, and a
mode-back write. Two problems:

1. The wait ran **even when the wake-up write failed** — against a dead or
   wedged sensor, every poll paid the full delay for a measurement that was
   never started, on top of the I2C timeouts (pre-4.2.0 firmware: this is
   the mechanism that let an on-demand `js.update()` from the `JV` endpoint
   silence the device long enough for the 30 s watchdog to fire).
2. The wait length comes from the access-mode table; a mode configuration
   (POWERDOWNMODE's 1000 ms period) can freeze a caller for a second per
   call.

## What changed

- `updateData()`: a failed wake-up write returns `TLV493D_BUS_ERROR`
  immediately — no wait, no read, no register bookkeeping on stale data.
- `updateData()`: the measurement wait is clamped to
  `TLV493D_MAX_MEASUREMENT_DELAY_MS` (50 ms — ~5x the master-controlled
  measurement's ~10 ms, so healthy polling is unaffected; combined with the
  vendored Wire's 50 ms per-transaction bound, the whole call is now
  worst-case bounded instead of unbounded).
- Nothing else. The register model, API and parity handling are byte-for-byte
  upstream; the sketch still includes `<Tlv493d.h>` and calls the same
  methods, so a future migration to the XENSIV library (roadmap item 2) is
  a port of `LSJoystick`, not of this fork.

## Upstream/deprecation status

Upstream archived the repository and marks this library deprecated in
`library.properties` ("Please use our XENSIV™ 3D Magnetic Sensor TLx493D
Arduino Library instead"). The LipSync keeps this fork until item 2's
migration is undertaken deliberately — with hardware validation, since any
change to the joystick read path affects the cursor feel of an assistive
device.
