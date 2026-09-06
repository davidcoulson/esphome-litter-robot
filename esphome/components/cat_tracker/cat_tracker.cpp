#include "cat_tracker.h"
#include "esphome/components/cat_tracker/number/number.h"
#include "esphome/core/log.h"

#include <cmath>
#include <limits>

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker";

void CatTrackerComponent::setup() {
#if CAT_TRACKER_MAX_CATS > 0
  if (this->weight_sensor_ != nullptr) {
    this->weight_sensor_->add_on_state_callback([this](float weight) { this->handle_weight_(weight); });
  }
#endif
}

void CatTrackerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Cat Tracker:");
  ESP_LOGCONFIG(TAG, "  Weight tolerance: %.1f lbs", this->tolerance_);
#if CAT_TRACKER_MAX_CATS > 0
  ESP_LOGCONFIG(TAG, "  Tracked cats: %u", CAT_TRACKER_MAX_CATS);
#endif
}

#if CAT_TRACKER_MAX_CATS > 0
void CatTrackerComponent::register_cat(CatTrackerCatWeightNumber *cat) {
  if (this->cats_.size() >= CAT_TRACKER_MAX_CATS) {
    ESP_LOGW(TAG, "Cannot track more than %u cats", CAT_TRACKER_MAX_CATS);
    return;
  }
  this->cats_.push_back(cat);
}

void CatTrackerComponent::handle_weight_(float weight) {
  if (std::isnan(weight) || weight <= 0.0f) {
    return;
  }
  CatTrackerCatWeightNumber *best = nullptr;
  float best_distance = std::numeric_limits<float>::max();
  for (auto *cat : this->cats_) {
    if (std::isnan(cat->state) || cat->state <= 0.0f) {
      continue;
    }
    float distance = std::abs(weight - cat->state);
    if (distance <= this->tolerance_ && distance < best_distance) {
      best = cat;
      best_distance = distance;
    }
  }
  if (best != nullptr) {
    ESP_LOGD(TAG, "Matched weight %.1f lbs to cat with weight %.1f lbs (distance %.1f lbs)", weight, best->state,
             best_distance);
    best->publish_weight(weight);
  }
}
#endif

void CatTrackerComponent::reset() {
#if CAT_TRACKER_MAX_CATS > 0
  for (auto *cat : this->cats_) {
    cat->reset();
  }
#endif
}

}  // namespace esphome::cat_tracker
