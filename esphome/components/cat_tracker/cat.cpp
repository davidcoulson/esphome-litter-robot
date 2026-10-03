#include "cat.h"
#include "esphome/core/log.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.cat";

struct CatTrackerCatRestoreState {
  float weight;
  uint32_t counts[3];
  float waste_weight[3];
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
  }
  this->weight_callback_.call(this->weight_);
  this->count_callback_.call();
}

void CatTrackerCat::set_weight(float weight) {
  this->weight_ = weight;
  this->weight_callback_.call(weight);
  this->save_state_();
}

void CatTrackerCat::record_visit() { this->record_count(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS); }

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
