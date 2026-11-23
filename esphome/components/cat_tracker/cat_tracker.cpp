#include "cat_tracker.h"
#include "esphome/core/log.h"

#include <cmath>
#include <limits>

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker";

void CatTrackerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Cat Tracker:");
  ESP_LOGCONFIG(TAG, "  Weight tolerance: %.1f lbs", this->weight_tolerance_);
#if CAT_TRACKER_MAX_CATS > 0
  ESP_LOGCONFIG(TAG, "  Tracked cats: %u", CAT_TRACKER_MAX_CATS);
#endif
}

#if CAT_TRACKER_MAX_CATS > 0
void CatTrackerComponent::register_cat(CatTrackerCat *cat) {
  if (this->cats_.size() >= CAT_TRACKER_MAX_CATS) {
    ESP_LOGW(TAG, "Cannot track more than %u cats", CAT_TRACKER_MAX_CATS);
    return;
  }
  this->cats_.push_back(cat);
}
#endif

CatTrackerCat *CatTrackerComponent::match_weight(float weight) {
#if CAT_TRACKER_MAX_CATS > 0
  if (std::isnan(weight) || weight <= 0.0f) {
    return nullptr;
  }
  CatTrackerCat *best = nullptr;
  float best_distance = std::numeric_limits<float>::max();
  bool any_calibrated = false;
  for (auto *cat : this->cats_) {
    float cat_weight = cat->get_weight();
    if (std::isnan(cat_weight) || cat_weight <= 0.0f) {
      continue;
    }
    any_calibrated = true;
    float distance = std::abs(weight - cat_weight);
    if (distance <= this->weight_tolerance_ && distance < best_distance) {
      best = cat;
      best_distance = distance;
    }
  }
  if (best == nullptr) {
    if (any_calibrated) {
      ESP_LOGD(TAG, "No cat matched weight %.1f lbs", weight);
    } else {
      ESP_LOGW(TAG, "No cat is calibrated; set a cat weight before matching");
    }
    return nullptr;
  }
  ESP_LOGD(TAG, "Matched weight %.1f lbs to cat with weight %.1f lbs (distance %.1f lbs)", weight, best->get_weight(),
           best_distance);
  best->set_weight(weight);
  return best;
#else
  ESP_LOGW(TAG, "Cannot process weight: no cats are configured");
  return nullptr;
#endif
}

void CatTrackerComponent::register_visit(CatTrackerCat *cat, bool weighed) {
#if CAT_TRACKER_MAX_CATS > 0
  if (cat == nullptr) {
    ESP_LOGW(TAG, "Cannot register visit: no cat specified");
    return;
  }
  cat->record_visit(weighed);
  this->on_visit_callback_.call();
#else
  ESP_LOGW(TAG, "Cannot register visit: no cats are configured");
#endif
}

void CatTrackerComponent::record_waste(CatTrackerCat *cat, CatTrackerCountType type) {
#if CAT_TRACKER_MAX_CATS > 0
  if (type == CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS) {
    ESP_LOGW(TAG, "Cannot record waste with the visits count type");
    return;
  }
  if (cat == nullptr) {
    ESP_LOGW(TAG, "Cannot record waste: no cat specified");
    return;
  }
  cat->record_count(type);
#else
  ESP_LOGW(TAG, "Cannot record waste: no cats are configured");
#endif
}

void CatTrackerComponent::record_waste_weight(CatTrackerCat *cat, CatTrackerCountType type, float weight) {
#if CAT_TRACKER_MAX_CATS > 0
  if (type == CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS) {
    ESP_LOGW(TAG, "Cannot record waste weight with the visits count type");
    return;
  }
  if (cat == nullptr) {
    ESP_LOGW(TAG, "Cannot record waste weight: no cat specified");
    return;
  }
  cat->set_waste_weight(type, weight);
#else
  ESP_LOGW(TAG, "Cannot record waste weight: no cats are configured");
#endif
}

void CatTrackerComponent::end_visits(CatTrackerCat *cat) {
#if CAT_TRACKER_MAX_CATS > 0
  if (cat != nullptr) {
    cat->end_visit();
    return;
  }
  for (auto *c : this->cats_)
    c->end_visit();
#endif
}

void CatTrackerComponent::reset() {
#if CAT_TRACKER_MAX_CATS > 0
  for (auto *cat : this->cats_) {
    cat->reset();
  }
  global_preferences->sync();
#endif
}

}  // namespace esphome::cat_tracker
