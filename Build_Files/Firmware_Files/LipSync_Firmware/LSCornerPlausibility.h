#ifndef LSCORNER_PLAUSIBILITY_H
#define LSCORNER_PLAUSIBILITY_H

// Pure corner-plausibility decision for the joystick calibration (5.0.0
// audit round 4), split out so the host suite can pin the semantics — the
// same pattern as LSPressureClassify.h. No Arduino types.
//
// WHY CENTER-RELATIVE: the corner slots store RAW sensor coordinates and
// the radius math is center-relative (LSJoystick::setMinimumRadius works
// from corner-minus-center distances), but BOTH plausibility gates used to
// compare the RAW axis values against CONF_JOY_CALIB_CORNER_MIN. On a
// neutral-origin unit raw == deflection and the gate measured the right
// quantity; on the documented off-origin-neutral field class (a
// compensated neutral tens of mT from the sensor origin, the class the
// 4.9.1 first-valid-wins seeding exists for) the raw check is dominated by
// the neutral offset:
//   - a corner captured with almost no movement (raw ~ the neutral) passed
//     the gate on magnitude of the NEUTRAL alone — the "moved too little,
//     use the default" protection upstream added in 4.1.0 silently never
//     applied to this unit class;
//   - a real far-quadrant corner whose deflection carried an axis past the
//     neutral's opposite sign could read |raw| below the min and get
//     replaced by the ±13 default with a SUCCESS ack.
// Measuring the per-axis DEFLECTION (corner minus center) makes the gate
// mean the same thing on every unit. A sensorless caller with no center
// passes {0,0}, which degrades to the old raw-axis shape honestly (there is
// no better reference without a sensor).
//
// Boundary semantics match the pre-round-4 gates exactly at a {0,0} center:
// capture rejects when ANY axis is strictly below the min; the readback
// twin accepts only when BOTH axes are at or above it.

// CONF_JOY_CALIB_CORNER_MIN's value, host-side — static_assert-pinned
// against LSConfig.h in the sketch (the LSPressureClassify pattern: a
// config change must fail the build, not silently retune the gate).
static constexpr float LSCORNER_PLAUSIBILITY_MIN_M_T = 3.0f;

#include <math.h>  // isfinite, fabsf

// 1 = the corner is a plausible captured/stored calibration point for this
// center (both axes deflected at least the min), 0 = degenerate or
// non-finite. corners/centers are raw sensor coordinates in mT.
static inline int lsCornerDeflectionPlausible(float cornerX, float cornerY,
                                              float centerX, float centerY) {
  if (!isfinite(cornerX) || !isfinite(cornerY)) return 0;
  if (!isfinite(centerX) || !isfinite(centerY)) return 0;
  const float deflectionX = cornerX - centerX;
  const float deflectionY = cornerY - centerY;
  return (fabsf(deflectionX) >= LSCORNER_PLAUSIBILITY_MIN_M_T) &&
         (fabsf(deflectionY) >= LSCORNER_PLAUSIBILITY_MIN_M_T);
}

#endif  // LSCORNER_PLAUSIBILITY_H
