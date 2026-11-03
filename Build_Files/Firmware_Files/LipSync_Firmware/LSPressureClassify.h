#ifndef LSPRESSURE_CLASSIFY_H
#define LSPRESSURE_CLASSIFY_H

// Pure main-state classification for the sip/puff evaluator (upstream #58),
// split out of LSPressure so the host suite can pin the hard-band semantics
// (the same pattern as LSPressureEngageFilter.h / LSPressureOffsetFilter.h).
// No Arduino types; numeric states mirror PRESS_SAP_MAIN_STATE_*.
//
// CONTRACT (consumed by LSPressure::updateState):
//  - A STARTED hold keeps its main state for its whole life while the
//    pressure stays beyond the REGULAR release boundary (the 4.5.7
//    hysteresis fraction). This is what makes the hard bands safe:
//      * hard-sticky DOWN — a hold that started in the hard band stays a
//        hard event even as it decays into the regular band (decaying must
//        not convert it into a click on release);
//      * no mid-hold UPGRADE — a regular hold pushed into the hard band
//        stays a regular event (entering hard mid-hold would otherwise
//        split one gesture into click-then-hard-action).
//    A hard event therefore means "started the gesture hard". Entering the
//    hard band fresh requires the engage filter's two consecutive fresh
//    HARD samples, which is this layer's caller's business.
//  - The hard thresholds must exceed the regular ones (validated at the API
//    layer with CONF_PRESS_HARD_MARGIN_HPA). A caller that violates it gets
//    the hard band treated as empty — hard states simply never classify —
//    which is fail-safe (the feature goes inert, never misfiring).
//  - Behavioral note vs the pre-#58 code: a STARTED hold whose pressure
//    swings fully to the OPPOSITE side in one poll classifies NONE here
//    (release) where the old inline chain could classify the opposite
//    direction one poll earlier; the emitted RELEASED event and the engage
//    filter's two-sample requirement make the wire behavior identical.

#define LSPRESSURE_CLASSIFY_NONE 0
#define LSPRESSURE_CLASSIFY_SIP 1
#define LSPRESSURE_CLASSIFY_PUFF 2
#define LSPRESSURE_CLASSIFY_HARD_SIP 3
#define LSPRESSURE_CLASSIFY_HARD_PUFF 4

static const float LSPRESSURE_CLASSIFY_RELEASE_FRACTION = 0.7f;  // CONF_PRESS_RELEASE_FRACTION's value, host-side

static inline int lipsyncClassifySapMainState(
    float pressureValue,
    float sipThreshold, float puffThreshold,
    float hardSipThreshold, float hardPuffThreshold,
    int prevStateMain, int prevStateSecondary, int startedState) {

  const bool holdActive = (prevStateSecondary == startedState);

  // Hold continuation first (see the contract): class is held for life.
  if (holdActive) {
    if ((prevStateMain == LSPRESSURE_CLASSIFY_PUFF || prevStateMain == LSPRESSURE_CLASSIFY_HARD_PUFF)
        && pressureValue > puffThreshold * LSPRESSURE_CLASSIFY_RELEASE_FRACTION) {
      return prevStateMain;
    }
    if ((prevStateMain == LSPRESSURE_CLASSIFY_SIP || prevStateMain == LSPRESSURE_CLASSIFY_HARD_SIP)
        && pressureValue < -sipThreshold * LSPRESSURE_CLASSIFY_RELEASE_FRACTION) {
      return prevStateMain;
    }
    return LSPRESSURE_CLASSIFY_NONE;  // fell past the full release boundary
  }

  // Fresh entry — the hard bands win by magnitude, and only exist while the
  // hard threshold actually sits above the regular one.
  if (hardPuffThreshold > puffThreshold && pressureValue > hardPuffThreshold) {
    return LSPRESSURE_CLASSIFY_HARD_PUFF;
  }
  if (hardSipThreshold > sipThreshold && pressureValue < -hardSipThreshold) {
    return LSPRESSURE_CLASSIFY_HARD_SIP;
  }

  // Fresh regular-band detection.
  if (pressureValue > puffThreshold) return LSPRESSURE_CLASSIFY_PUFF;
  if (pressureValue < -sipThreshold) return LSPRESSURE_CLASSIFY_SIP;
  return LSPRESSURE_CLASSIFY_NONE;
}

#endif  // LSPRESSURE_CLASSIFY_H
