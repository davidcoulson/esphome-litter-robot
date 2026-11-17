#pragma once

#include <cmath>
#include <cstdint>

namespace esphome::cat_tracker {

// Tracks one continuous tag presence. A visit ends when the tag is not heard for longer than the gap.
// All times are millis(); unsigned subtraction survives the 49-day wrap.
struct VisitDwell {
  bool active{false};
  uint32_t start{0}, last{0}, ended_s{0};  // ended_s: duration of the visit that just ended, seconds

  // True if this sighting came after the gap, i.e. it ended the previous visit (and started a new one).
  bool seen(uint32_t now, uint32_t gap_ms) {
    bool ended = this->expire(now, gap_ms);
    if (!this->active) {
      this->active = true;
      this->start = now;
    }
    this->last = now;
    return ended;
  }
  bool expire(uint32_t now, uint32_t gap_ms) { return this->active && now - this->last > gap_ms && this->finish(); }
  bool finish() {
    if (!this->active)
      return false;
    this->active = false;
    this->ended_s = (this->last - this->start + 500) / 1000;
    return true;
  }
  float minutes(uint32_t now) const { return this->active ? (now - this->start) / 60000.0f : NAN; }
};

}  // namespace esphome::cat_tracker
