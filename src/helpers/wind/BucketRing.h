#pragma once

#include <stdint.h>

// A ring of N fixed-length time buckets, each holding one value, used to compute a mean and a max over
// everything currently held. This is the sustained/gust averaging logic behind ArgentWindSpeed, pulled
// out so it can be unit tested directly (see test_bucket_ring.cpp) without exposing it through
// ArgentWindSpeed.h — production code has exactly one instance, private to ArgentWindSpeed.cpp.
template <uint16_t N>
class BucketRing {
  uint16_t _values[N] = {0};
  uint16_t _next = 0;     // slot the next bucket is written to
  uint16_t _filled = 0;   // buckets of data held, up to N

public:
  // Record `value` for the last `buckets` buckets. Normally buckets is 1; it is larger when the caller
  // was stalled and every missed bucket gets the same value.
  void record(uint16_t value, uint32_t buckets = 1) {
    for (uint32_t b = 0; b < buckets && b < N; b++) {
      _values[_next] = value;
      _next = (_next + 1) % N;
      if (_filled < N) _filled++;
    }
  }

  // Mean of everything held.
  uint16_t mean() const {
    if (_filled == 0) return 0;
    uint32_t sum = 0;
    for (uint16_t i = 0; i < _filled; i++) sum += _values[i];
    return sum / _filled;
  }

  // Highest single bucket value held.
  uint16_t max() const {
    uint16_t most = 0;
    for (uint16_t i = 0; i < _filled; i++) {
      if (_values[i] > most) most = _values[i];
    }
    return most;
  }
};
