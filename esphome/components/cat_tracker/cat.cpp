#include "cat.h"
#include "esphome/core/log.h"

#include <cinttypes>
#include <cmath>
#include <algorithm>
#include <cstring>

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.cat";

struct CatTrackerCatRestoreState {
  float weight;
  uint32_t counts[3];
  float waste_weight[3];
};

// State layout used by the 2026.9.2 cat tracker (stored per weight-number entity).
struct LegacyTrackedCatRestoreState {
  float last_weight;
  uint32_t visit_count;
};

void CatTrackerCat::setup() {
  this->pref_ = global_preferences->make_preference<CatTrackerCatRestoreState>(
      fnv1_hash_extend(fnv1_hash("cat_tracker_cat_"), this->id_name_));
  CatTrackerCatRestoreState state{};
  if (this->pref_.load(&state)) {
    this->weight_ = state.weight;
    for (size_t i = 0; i < this->counts_.size(); i++) {
      this->counts_[i] = state.counts[i];
      this->waste_weight_[i] = state.waste_weight[i];
    }
  } else {
    this->migrate_legacy_state_();
  }
  // History v2 has 16 slots and weight counts; import the 14-slot v1 layout once if it is all there is.
  this->history_pref_ = global_preferences->make_preference<History>(
      fnv1_hash_extend(fnv1_hash("cat_tracker_hist2_"), this->id_name_));
  if (!this->history_pref_.load(&this->history_)) {
    struct HistoryV1 {
      uint32_t day[14];
      uint8_t visits[14];
      float weight_sum[14];
      uint32_t last_visit;
    } old{};
    auto v1 = global_preferences->make_preference<HistoryV1>(
        fnv1_hash_extend(fnv1_hash("cat_tracker_hist_"), this->id_name_));
    if (v1.load(&old)) {
      uint32_t first = 0;
      for (size_t i = 0; i < 14; i++) {
        uint32_t d = old.day[i];
        if (d == 0)
          continue;
        uint8_t j = d % HISTORY_DAYS;
        this->history_.day[j] = d;
        this->history_.visits[j] = old.visits[i];
        this->history_.weight_n[j] = old.visits[i];  // v1 only counted visits that had a weight
        this->history_.weight_sum[j] = old.weight_sum[i];
        if (first == 0 || d < first)
          first = d;
      }
      this->history_.last_visit = old.last_visit;
      // Coverage starts at the oldest day v1 recorded (a day boundary is close enough here).
      this->history_.started = first == 0 ? 0 : first * 86400u;
      this->save_history_();
    }
  }
  this->dur_pref_ = global_preferences->make_preference<Durations>(
      fnv1_hash_extend(fnv1_hash("cat_tracker_dur_"), this->id_name_));
  this->dur_pref_.load(&this->dur_);
  // Time passes with no visit: refresh the derived history values (new day, hours since visit).
  this->set_interval(5 * 60 * 1000, [this]() { this->history_callback_.call(); });
  this->set_interval(5000, [this]() {
    this->flush_pending_();
    if (this->dwell_.expire(millis(), this->gap_ms_))
      this->on_visit_ended_();
  });
  this->weight_callback_.call(this->weight_);
  this->count_callback_.call();
  this->history_callback_.call();
}

void CatTrackerCat::migrate_legacy_state_() {
  if (this->legacy_object_id_ == nullptr || this->legacy_object_id_[0] == '\0')
    return;
  auto legacy = global_preferences->make_preference<LegacyTrackedCatRestoreState>(
      fnv1_hash_object_id(this->legacy_object_id_, strlen(this->legacy_object_id_)));
  LegacyTrackedCatRestoreState old_state{};
  if (!legacy.load(&old_state))
    return;
  ESP_LOGI(TAG, "Imported legacy state for cat '%s': weight %.2f, visits %" PRIu32, this->id_name_,
           old_state.last_weight, old_state.visit_count);
  this->weight_ = old_state.last_weight;
  this->counts_[static_cast<size_t>(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS)] = old_state.visit_count;
  this->save_state_();
  global_preferences->sync();
}

void CatTrackerCat::set_weight(float weight) {
  this->weight_ = weight;
  this->weight_callback_.call(weight);
  this->save_state_();
}

void CatTrackerCat::record_visit(bool weighed) {
  if (this->today_() == 0) {
    // No clock yet: hold the visit so it still counts once SNTP/HA sets the time.
    if (this->pending_visits_ < UINT8_MAX) {
      this->pending_visits_++;
      if (weighed && !std::isnan(this->weight_) && this->pending_weights_ < UINT8_MAX) {
        this->pending_weights_++;
        this->pending_weight_sum_ += this->weight_;
      }
    }
  } else {
    this->pending_visits_++;
    if (weighed && !std::isnan(this->weight_)) {
      this->pending_weights_++;
      this->pending_weight_sum_ += this->weight_;
    }
    this->flush_pending_();
  }
  this->record_count(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS);
}

void CatTrackerCat::flush_pending_() {
  uint32_t day = this->today_();
  if (day == 0)
    return;
  bool dirty = false;
  if (this->history_.started == 0) {
    this->history_.started = ::time(nullptr);
    dirty = true;
  }
  if (this->pending_visits_ > 0) {
    uint8_t i = day % HISTORY_DAYS;
    if (this->history_.day[i] != day) {
      this->history_.day[i] = day;
      this->history_.visits[i] = 0;
      this->history_.weight_n[i] = 0;
      this->history_.weight_sum[i] = 0;
    }
    this->history_.visits[i] = std::min<uint32_t>(UINT8_MAX, this->history_.visits[i] + this->pending_visits_);
    this->history_.weight_n[i] = std::min<uint32_t>(UINT8_MAX, this->history_.weight_n[i] + this->pending_weights_);
    this->history_.weight_sum[i] += this->pending_weight_sum_;
    this->history_.last_visit = ::time(nullptr);
    this->pending_visits_ = this->pending_weights_ = 0;
    this->pending_weight_sum_ = 0;
    dirty = true;
  }
  if (dirty) {
    this->save_history_();
    this->history_callback_.call();
  }
}

uint32_t CatTrackerCat::today_() const {
  time_t now = ::time(nullptr);
  if (now < 1600000000)  // SNTP has not set the clock yet
    return 0;
  struct tm lt;
  localtime_r(&now, &lt);
  return civil_day(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
}

uint32_t CatTrackerCat::visits_in(uint8_t offset, uint8_t days) const {
  uint32_t today = this->today_(), total = 0;
  if (today == 0)
    return 0;
  for (uint8_t k = offset; k < offset + days && k < HISTORY_DAYS; k++) {
    uint32_t d = today - k;
    if (this->history_.day[d % HISTORY_DAYS] == d)
      total += this->history_.visits[d % HISTORY_DAYS];
  }
  return total;
}

uint32_t CatTrackerCat::weights_in(uint8_t offset, uint8_t days) const {
  uint32_t today = this->today_(), total = 0;
  for (uint8_t k = offset; today != 0 && k < offset + days && k < HISTORY_DAYS; k++) {
    uint32_t d = today - k;
    if (this->history_.day[d % HISTORY_DAYS] == d)
      total += this->history_.weight_n[d % HISTORY_DAYS];
  }
  return total;
}

float CatTrackerCat::visits_per_day(uint8_t offset, uint8_t days) const {
  return !this->history_covers(offset + days - 1) ? NAN : static_cast<float>(this->visits_in(offset, days)) / days;
}

float CatTrackerCat::avg_weight(uint8_t offset, uint8_t days) const {
  uint32_t today = this->today_(), n = 0;
  float sum = 0;
  for (uint8_t k = offset; today != 0 && k < offset + days && k < HISTORY_DAYS; k++) {
    uint32_t d = today - k;
    if (this->history_.day[d % HISTORY_DAYS] == d) {
      n += this->history_.weight_n[d % HISTORY_DAYS];
      sum += this->history_.weight_sum[d % HISTORY_DAYS];
    }
  }
  return n == 0 ? NAN : sum / n;
}

float CatTrackerCat::hours_since_visit() const {
  uint32_t ref = this->history_.last_visit != 0 ? this->history_.last_visit : this->history_.started;
  if (ref == 0 || this->today_() == 0)
    return NAN;
  return (::time(nullptr) - ref) / 3600.0f;
}

bool CatTrackerCat::history_covers(uint8_t days_back) const {
  uint32_t today = this->today_();
  if (today == 0 || this->history_.started == 0)
    return false;
  // started is an epoch time; compare by local day number via the same calendar conversion.
  time_t s = this->history_.started;
  struct tm lt;
  localtime_r(&s, &lt);
  return today - civil_day(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday) >= days_back;
}

void CatTrackerCat::tag_seen(int rssi) {
  if (rssi < this->min_rssi_)
    return;
  if (this->dwell_.seen(millis(), this->gap_ms_))
    this->on_visit_ended_();
}

void CatTrackerCat::end_visit() {
  if (this->dwell_.finish())
    this->on_visit_ended_();
}

void CatTrackerCat::on_visit_ended_() {
  uint32_t secs = this->dwell_.ended_s, day = this->today_();
  if (secs > 0) {
    this->dur_.last = secs;
    if (day != 0) {
      uint8_t i = day % DURATION_DAYS;
      if (this->dur_.day[i] != day) {
        this->dur_.day[i] = day;
        this->dur_.sum_s[i] = 0;
        this->dur_.n[i] = 0;
      }
      if (this->dur_.n[i] < UINT8_MAX) {
        this->dur_.n[i]++;
        this->dur_.sum_s[i] += secs;
      }
    }
    this->dur_pref_.save(&this->dur_);
  }
  this->history_callback_.call();
}

float CatTrackerCat::avg_duration(uint8_t offset, uint8_t days) const {
  uint32_t today = this->today_(), n = 0, sum = 0;
  for (uint8_t k = offset; today != 0 && k < offset + days && k < DURATION_DAYS; k++) {
    uint32_t d = today - k;
    if (this->dur_.day[d % DURATION_DAYS] == d) {
      n += this->dur_.n[d % DURATION_DAYS];
      sum += this->dur_.sum_s[d % DURATION_DAYS];
    }
  }
  return n == 0 ? NAN : static_cast<float>(sum) / n;
}

void CatTrackerCat::save_history_() { this->history_pref_.save(&this->history_); }

void CatTrackerCat::record_count(CatTrackerCountType type) {
  this->counts_[static_cast<size_t>(type)]++;
  this->count_callback_.call();
  this->save_state_();
}

void CatTrackerCat::set_waste_weight(CatTrackerCountType type, float weight) {
  this->waste_weight_[static_cast<size_t>(type)] = weight;
  this->count_callback_.call();
  this->save_state_();
}

void CatTrackerCat::reset() {
  this->counts_.fill(0);
  this->waste_weight_.fill(0.0f);
  this->weight_ = NAN;
  this->weight_callback_.call(NAN);
  this->count_callback_.call();
  this->save_state_();
  this->history_ = {};
  this->pending_visits_ = this->pending_weights_ = 0;
  this->pending_weight_sum_ = 0;
  this->save_history_();
  this->dur_ = {};
  this->dur_pref_.save(&this->dur_);
  this->dwell_ = {};
  this->history_callback_.call();
}

void CatTrackerCat::save_state_() {
  CatTrackerCatRestoreState state{};
  state.weight = this->weight_;
  for (size_t i = 0; i < this->counts_.size(); i++) {
    state.counts[i] = this->counts_[i];
    state.waste_weight[i] = this->waste_weight_[i];
  }
  this->pref_.save(&state);
}

void CatTrackerCat::dump_config() {
  ESP_LOGCONFIG(TAG, "Cat Tracker Cat '%s'", this->id_name_ != nullptr ? this->id_name_ : "(unnamed)");
}

}  // namespace esphome::cat_tracker
