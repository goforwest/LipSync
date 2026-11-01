#ifndef _LSPRESSUREOFFSETFILTER_H
#define _LSPRESSUREOFFSETFILTER_H

#include <math.h>
#include <stdint.h>

// Collects a pressure-sensor offset without letting a user's sip or puff
// become the new neutral. Once a baseline exists, every candidate must stay
// inside a small neutral band around it and the samples must be stable. The
// initial boot measurement has no reference against which activity can be
// distinguished, but it still requires a complete, consecutive stable run;
// partial or changing readings are never averaged into a baseline.
class LSPressureOffsetFilter {
  public:
    enum Result : uint8_t {
      INACTIVE = 0,
      COLLECTING,
      COMPLETE,
      ACTIVITY_REJECTED,
      ATTEMPTS_EXHAUSTED,
      RETRYING_INITIAL
    };

    LSPressureOffsetFilter(uint8_t requiredSamples,
                           uint8_t maxAttempts,
                           float stabilityTolerance)
      : _requiredSamples(requiredSamples == 0 ? 1 : requiredSamples),
        _maxAttempts(maxAttempts < _requiredSamples ? _requiredSamples : maxAttempts),
        _stabilityTolerance(stabilityTolerance > 0.0f ? stabilityTolerance : 0.01f) {}

    void begin(float currentOffset, bool baselineValid, float neutralLimit) {
      _currentOffset = currentOffset;
      _baselineValid = baselineValid;
      _neutralLimit = neutralLimit > 0.0f ? neutralLimit : 0.01f;
      _attempts = 0;
      resetSamples();
      _running = true;
    }

    void cancel() {
      _running = false;
      _attempts = 0;
      resetSamples();
    }

    bool isRunning() const { return _running; }
    float resultOffset() const { return _resultOffset; }

    Result addSample(bool valid, float rawDifference) {
      if (!_running) return INACTIVE;
      _attempts++;

      if (!valid || !isfinite(rawDifference)) {
        // A baseline is only the average of one uninterrupted run of real
        // readings. Never bridge an I2C failure with samples from before it.
        resetSamples();
        return finishAttemptWindowIfNeeded();
      }

      if (_baselineValid && fabsf(rawDifference - _currentOffset) > _neutralLimit) {
        // With an established reference, this is user pressure (or a shift
        // too large to distinguish safely from user pressure). Keeping the
        // old offset is safer than absorbing it and reversing the apparent
        // direction when the mouthpiece is released.
        _running = false;
        resetSamples();
        return ACTIVITY_REJECTED;
      }

      if (_sampleCount > 0) {
        const float candidateMean = _sampleSum / _sampleCount;
        if (fabsf(rawDifference - candidateMean) > _stabilityTolerance) {
          if (_baselineValid) {
            _running = false;
            resetSamples();
            return ACTIVITY_REJECTED;
          }
          // No boot baseline exists yet. Start a fresh candidate run from
          // this reading rather than blending two changing pressure levels.
          _sampleSum = rawDifference;
          _sampleCount = 1;
          return finishAttemptWindowIfNeeded();
        }
      }

      _sampleSum += rawDifference;
      _sampleCount++;
      if (_sampleCount >= _requiredSamples) {
        _resultOffset = _sampleSum / _sampleCount;
        _running = false;
        return COMPLETE;
      }

      return finishAttemptWindowIfNeeded();
    }

  private:
    uint8_t _requiredSamples;
    uint8_t _maxAttempts;
    float _stabilityTolerance;
    float _currentOffset = 0.0f;
    float _neutralLimit = 0.0f;
    float _sampleSum = 0.0f;
    float _resultOffset = 0.0f;
    uint8_t _sampleCount = 0;
    uint8_t _attempts = 0;
    bool _baselineValid = false;
    bool _running = false;

    void resetSamples() {
      _sampleSum = 0.0f;
      _sampleCount = 0;
    }

    Result finishAttemptWindowIfNeeded() {
      if (_attempts < _maxAttempts) return COLLECTING;

      if (_baselineValid) {
        _running = false;
        resetSamples();
        return ATTEMPTS_EXHAUSTED;
      }

      // At first boot there is no safe old baseline to retain. Stay inert
      // and open another bounded collection window until a full stable run
      // arrives; the main loop remains non-blocking throughout.
      _attempts = 0;
      resetSamples();
      return RETRYING_INITIAL;
    }
};

#endif
