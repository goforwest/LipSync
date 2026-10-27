# Bluefruit52Lib — LipSync instrumented fork

Vendored fork of the `Bluefruit52Lib` bundled with `Seeeduino:nrf52@1.1.13`
(the core the LipSync firmware pins; see
`.github/workflows/build_firmware.yml` for the pinned version). Re-validate
against the core's `libraries/Bluefruit52Lib/` if the core version is ever
bumped.

## Why this fork exists

The bench hunt (rounds 8-17, Oct 26-27 2026) proved the BLE-boot deaths are
SOFTDEVICE ASSERTIONS: `sd_softdevice_enable`'s fault handler resets the
chip the moment the application-registered handler returns, and the stock
handler is compiled EMPTY at `CFG_DEBUG=0` — so every death read identically
(rst=0x4 SREQ, no HardFault marker, no witness marker, no fault page) and the
fault was provable only by elimination. The SoftDevice hands its OWN assert
location (file, line, PC) to that handler; the stock discards it.

## What changed

Exactly ONE change from upstream: `nrf_error_cb` in
`src/bluefruit.cpp` (see the `LIPSYNC PATCH` comment there). It now:

- prints the fault (id, pc, info) and — for `NRF_FAULT_ID_SD_ASSERT` — the
  SD's own `assert at <file> : <line>` to the CDC port;
- records the asserting PC's 8 KB flash page into `GPREGRET` as
  `0xC0 | page` (the same encoding the app's HardFault capture uses; the
  values cannot collide with the bootloader's DFU magics), which the next
  boot reports as `fp=0x<page-base>` on the BOOT/HB lines —
  `fp` below 0x26000 names the SoftDevice's own assertion page;
- resets (the same SREQ the stock path produced), so the boot-cycle shape
  is unchanged — but every successor boot now names the killer.

Everything else is byte-identical to the core's copy. When the core is
bumped, re-vendor and re-apply only this patch.
