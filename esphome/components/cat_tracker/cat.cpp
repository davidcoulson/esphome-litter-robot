#include "cat.h"
#include "esphome/core/log.h"

#include <cinttypes>
#include <cmath>
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
  this->history_pref_ = global_preferences->make_preference<History>(
      fnv1_hash_extend(fnv1_hash("cat_tracker_hist_"), this->id_name_));
  this->history_pref_.load(&this->history_);
  // Time passes with no visit: refresh the derived history values (new day, hours since visit).
  this->set_interval(5 * 60 * 1000, [this]() { this->history_callback_.call(); });
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

void CatTrackerCat::record_visit() {
  uint32_t day = this->today_();
  if (day != 0) {
    uint8_t i = day % HISTORY_DAYS;
    if (this->history_.day[i] != day) {
      this->history_.day[i] = day;
      this->history_.visits[i] = 0;
      this->history_.weight_sum[i] = 0;
    }
    if (this->history_.visits[i] < UINT8_MAX && !std::isnan(this->weight_)) {
      this->history_.visits[i]++;
      this->history_.weight_sum[i] += this->weight_;
    }
    this->history_.last_visit = ::time(nullptr);
    this->save_history_();
    this->history_callback_.call();
  }
  this->record_count(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS);
}

uint32_t CatTrackerCat::today_() const {
  time_t now = ::time(nullptr);
  if (now < 1600000000)  // SNTP has not set the clock yet
    return 0;
  // newlib's struct tm has no tm_gmtoff: the UTC fields read back as local time give now - offset.
  struct tm utc;
  gmtime_r(&now, &utc);
  utc.tm_isdst = -1;
  return static_cast<uint32_t>((now + (now - mktime(&utc))) / 86400);
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

float CatTrackerCat::visits_per_day(uint8_t offset, uint8_t days) const {
  return this->today_() == 0 ? NAN : static_cast<float>(this->visits_in(offset, days)) / days;
}

float CatTrackerCat::avg_weight(uint8_t offset, uint8_t days) const {
  uint32_t today = this->today_(), n = 0;
  float sum = 0;
  for (uint8_t k = offset; today != 0 && k < offset + days && k < HISTORY_DAYS; k++) {
    uint32_t d = today - k;
    if (this->history_.day[d % HISTORY_DAYS] == d) {
      n += this->history_.visits[d % HISTORY_DAYS];
      sum += this->history_.weight_sum[d % HISTORY_DAYS];
    }
  }
  return n == 0 ? NAN : sum / n;
}

float CatTrackerCat::hours_since_visit() const {
  if (this->history_.last_visit == 0 || this->today_() == 0)
    return NAN;
  return (::time(nullptr) - this->history_.last_visit) / 3600.0f;
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
  this->save_history_();
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
