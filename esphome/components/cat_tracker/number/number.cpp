#include "number.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.number";

struct TrackedCatRestoreState {
  float last_weight;
  uint32_t visit_count;
};

void CatTrackerCatWeightNumber::setup() {
  this->parent_->register_cat(this);
  this->pref_ = this->make_entity_preference<TrackedCatRestoreState>();
  TrackedCatRestoreState state{};
  if (this->pref_.load(&state)) {
    this->visit_count_ = state.visit_count;
    this->publish_state(state.last_weight);
    this->on_visit_callback_.call(this->visit_count_);
  }
}

void CatTrackerCatWeightNumber::control(float value) {
  this->publish_state(value);
  this->save_state_();
}

void CatTrackerCatWeightNumber::publish_weight(float weight) {
  this->publish_state(weight);
  this->visit_count_++;
  this->on_visit_callback_.call(this->visit_count_);
  this->save_state_();
}

void CatTrackerCatWeightNumber::reset() {
  this->visit_count_ = 0;
  this->on_visit_callback_.call(0);
  this->publish_state(NAN);

  TrackedCatRestoreState state{};
  state.last_weight = NAN;
  state.visit_count = 0;
  this->pref_.save(&state);
  global_preferences->sync();
}

void CatTrackerCatWeightNumber::save_state_() {
  TrackedCatRestoreState state{};
  state.last_weight = this->state;
  state.visit_count = this->visit_count_;
  this->pref_.save(&state);
}

void CatTrackerCatWeightNumber::dump_config() { LOG_NUMBER("", "Cat Tracker Cat Weight", this); }

}  // namespace esphome::cat_tracker
