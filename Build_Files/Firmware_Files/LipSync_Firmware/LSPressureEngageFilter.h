#ifndef _LSPRESSUREENGAGEFILTER_H
#define _LSPRESSUREENGAGEFILTER_H

#include <stdint.h>

// Qualifies a new sip/puff engagement without delaying an already-active
// hold. A single pressure sample can be an I2C/sensor transient; requiring
// the same nonzero direction on consecutive polls prevents that one sample
// from becoming STARTED -> RELEASED -> click. The state values are supplied
// by LSPressure (0 = none, nonzero = sip/puff), keeping this helper pure and
// directly host-testable.
class LSPressureEngageFilter {
  public:
    explicit LSPressureEngageFilter(uint8_t requiredSamples = 2)
      : _requiredSamples(requiredSamples == 0 ? 1 : requiredSamples) {}

    void reset() {
      _candidateState = 0;
      _candidateSamples = 0;
    }

    int qualify(int detectedState, bool holdAlreadyActive) {
      if (holdAlreadyActive) {
        // Hysteresis/release behavior belongs to the live hold and must not
        // acquire a second engagement delay.
        reset();
        return detectedState;
      }

      if (detectedState == 0) {
        reset();
        return 0;
      }

      if (_candidateState != detectedState) {
        _candidateState = detectedState;
        _candidateSamples = 1;
      } else if (_candidateSamples < 255) {
        _candidateSamples++;
      }

      if (_candidateSamples >= _requiredSamples) {
        const int qualifiedState = _candidateState;
        reset();
        return qualifiedState;
      }
      return 0;
    }

  private:
    uint8_t _requiredSamples;
    int _candidateState = 0;
    uint8_t _candidateSamples = 0;
};

#endif
