# Adafruit LPS35HW — LipSync bounded fork

Vendored from the release-pinned **Adafruit LPS35HW 1.0.6** library.

Upstream waits forever for reset, one-shot measurement, and pressure-zero
bits to clear. A failed Adafruit BusIO read returns all ones, which keeps each
loop true even though every underlying Wire transaction is individually
bounded. LipSync bounds these waits at 100 ms; `reset()` returns `bool` and
`begin()` propagates a timeout as failure. Normal devices clear these bits in
milliseconds, so successful behavior is unchanged.

All other source is version 1.0.6. Re-vendor and reapply this bounded patch
when changing the pinned dependency.
