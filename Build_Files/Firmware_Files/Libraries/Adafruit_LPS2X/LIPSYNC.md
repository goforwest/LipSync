# Adafruit LPS2X — LipSync bounded fork

Vendored from the release-pinned **Adafruit LPS2X 2.0.6** library.

The upstream `reset()` and one-shot waits have no timeout. Adafruit BusIO
returns all-one bits when an I2C read fails, so a sensor that disappears or
wedges during reset is interpreted as “reset still active” forever. LipSync
uses the bounded Wire fork, but bounded transactions alone cannot terminate a
caller that repeats them without a deadline.

LipSync’s patch bounds both waits at 100 ms and makes `reset()` return `bool`;
the LPS22/LPS25 initializers propagate a timeout as `begin() == false`. Normal
sensors clear reset in milliseconds, so successful behavior is unchanged.
The firmware additionally bypasses upstream `getEvent()` for live ambient
pressure because that path ignores BusIO failures and decodes an uninitialized
buffer; see `LSPressure::readAmbientPressure()`.

All other source is version 2.0.6. Re-vendor and reapply this bounded patch
when changing the pinned dependency.
