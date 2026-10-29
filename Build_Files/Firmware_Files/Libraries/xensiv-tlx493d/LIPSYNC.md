# XENSIV™ 3D Magnetic Sensor TLx493D — vendored for LipSync (upstream issue #37 migration)

**STATUS: VALIDATED AND MERGED (bench 2026-10-18).** The full protocol in
LipSync_Connect `docs/xensiv-migration-bench-protocol.md` passed on hardware:
JV raw parity vs the TLV493D-fork baseline (corner magnitudes ~25–30 mT on
both, same 0.098 scale, no axis/sign changes), a fresh four-corner
calibration completing cleanly in proper quadrature, the Z-direction latch
holding `dirX=-1, dirY=1` across five restarts, cursor feel signed off by
the bench user (SS=7/AV=3), and an unseated-cable flaky-bus episode that
matched the documented degraded-state pattern exactly (graceful gating, no
reset loop, full recovery on re-seat). The battery trade is CLOSED AS MOOT
(follow-up 2026-10-18): **the LipSync has no internal battery** — the BOM
is a Xiao nRF52840 powered via the hub USB cable, with no cell or charger
anywhere in the parts list. Every powered state is externally fed: host USB
(computer / power bank / powered splitter) or, for mobile use, the phone
over USB-OTG — the one case where the sensor's continuous mA-scale draw
(built-in measurement vs the old fork's µA-scale power-down between reads)
lands on something the user notices, and precisely the case the User Guide
already steers to an external power source. There is no device-battery-life
metric for the continuous current to affect; powerdown-mode work would
churn the freshly-validated read path for no measurable product benefit.
The retired `TLV493D-A1B6` fork was removed in the merge commit; its
bounding work lives on in the vendored Wire, and this library's read path
needs none (see the audit below).

Vendored copy of Infineon's `arduino-xensiv-3d-magnetic-sensor-tlx493d`
(library.properties version 1.1.2, MIT license; repository
<https://github.com/Infineon/arduino-xensiv-3d-magnetic-sensor-tlx493d>,
cloned `--recursive` at master @ 27 commits, 2026-10-18 — the core C library
is committed in-repo under `src/corelib/`, so no submodule content is
missing). Pinned into the build via the `--libraries` flag exactly like the
vendored `Wire` (see `dev/firmware/build-firmware.sh` in LipSync_Connect
and the firmware CI workflow), so CI, local builds and releases all compile
this copy — never whatever TLx493D library happens to be installed in a
user's Arduino15.

## Why this migration (and why it was deferred)

Upstream #37: Infineon deprecated the `TLV493D-A1B6` library this firmware
shipped with. 4.4.0 vendored a *bounded fork* of it (see
`../TLV493D-A1B6/README.md`) instead of migrating, because the joystick read
path is the live cursor path of an assistive device — any change there
touches what the user's mouth-puff-and-stick feels, and it was deliberately
left for a bench-validated pass. This is that pass; it was bench-validated
and merged in firmware 4.5.1.

## Boundedness audit (the fork's guarantees, verified against the successor)

The old fork existed because the stock TLV493D library blocked without a
bound on the 20 ms poll path: a wake-up write, an unbounded busy-wait of the
measurement period, the read, and a mode-back write — per poll. The vendored
fork bounded that (no wait on a failed wake write; the wait clamped at
50 ms).

The XENSIV A1B6 (gen-1) path is **bounded by construction — and strictly
cheaper than the fork**:

- `TLx493D_A1B6_setDefaultConfig()` puts the sensor in
  **MASTER-CONTROLLED continuous measurement**: the sensor measures on its
  own, so a poll is *just* `TLx493D_A1B6_readRegisters()` → one
  `tlx493d_transfer` → one I2C read of 10 registers. No wake write, no
  measurement wait, no sleep write, no `delay()` anywhere in the gen-1 read
  path (the gen-2/3 `readRegistersAndCheck` retry loop — 10 bounded
  re-reads — is NOT wired for A1B6; `TLx493D_A1B6_readRegistersAndCheck`
  calls the plain single read).
- Every transaction still flows through the sketch's `Wire` object — the
  vendored Wire fork that bounds every TWIM event wait at 50 ms with wedge
  recovery and `busHealthy()` hold-off. Whole-call worst case: one bounded
  I2C read.
- `getMagneticField(&x, &y, &z)` scales the 12-bit raw values with
  `GEN_1_MAG_FIELD_MULT` = **0.098** — the exact scale of the old library's
  `getX()/getY()/getZ()` (`TLV493D_B_MULT` = 0.098), so raw values are
  numerically comparable across the migration (the bench JV parity check
  compares them directly). Same concat model, same mT units.

Master-controlled measurement draws the sensor continuously (~mA-scale, vs
µA-scale power-down between reads under the old fork's POWERDOWNMODE). The
hardware has no internal battery, so this was documented and accepted during
the completed bench validation (see the status section above).

## What LipSync uses from this library

Only the A1B6 (TLV493D-A1B6) I2C surface: `TLx493D_A1B6` =
`TLx493D<Kit2GoBoardSupport, TwoWireWrapper, TLx493D_A1B6_e>`, constructed
with the sketch's `Wire` and `TLx493D_IIC_ADDR_A0_e` (= 0xBC write address =
0x5E 7-bit — `I2CADDR_TLV493D`), `begin(false, false, false, false)` — no
Kit2Go power pins (the sensor is hardwired on the LipSync PCB) and
`executeInit=false` (Wire is already configured by `initWire()` with the
bounded fork; a library re-`begin()` must never reconfigure the bus under
the sketch). Reads via `getMagneticField()`/`getMagneticFieldAndTemperature()`.

## Provenance note

This copy is upstream plus **one bounded, correctness-only port** (4.5.7
review): `TwoWireWrapper::transfer()` now propagates `endTransmission()`'s
result — Arduino `write()` only buffers locally, and the discarded return
made a NACKed register write (the master-controlled-mode configuration after
a sensor brownout) report success while the sensor never entered measurement
mode, a silently dead cursor with no fault signal. If the bounding guarantees
above ever regress (an upstream change reintroducing an unbounded wait or a
per-read write-wait-read cycle), port the old fork's bounding into this copy
BEFORE updating — the guarantees are load-bearing for the 20 ms poll loop and
the watchdog math, not cosmetic. (The powerdown-mode variant once considered
for battery reasons is closed as moot: see the status paragraph — the LipSync
hardware has no internal battery to trade against.)

## Documented limitation (4.5.7 review): gen-1 data validity is unchecked

`TLx493D_A1B6_readRegistersAndCheck()` is an upstream passthrough to the plain
single read — unlike gen-2/3, no FRM (frame-counter) consistency check, no
all-0xFF rejection, no T/PD status-bit inspection exists on the A1B6 path, so
the 10-byte burst is accepted unconditionally. Time-boundedness covers WHEN a
read returns, not WHAT it returns: a burst can straddle the sensor's own
frame update (a torn x/y/z across adjacent frames — absorbed by the
firmware's 0.1 mT skip gate and deadzones at rest), and a frozen or garbage
register page after a sensor brownout is delivered as fully valid data. The
firmware covers the harmful half at its own layer: LSJoystick checks
`getMagneticField()`'s bool and holds the last reading on failure, plus a
consecutive-failure re-init. Anything more belongs in a future upstream
contribution (wiring the gen-2/3 validity machinery for gen 1), not in this
vendoring.

---

*The remainder of this file is upstream's README.*
