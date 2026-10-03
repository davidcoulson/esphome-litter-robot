#include "number.h"
#include "esphome/core/log.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.number";

void CatTrackerCatWeightNumber::setup() {
  if (this->cat_ == nullptr)
    return;
  this->cat_->add_on_weight_callback([this](float weight) { this->publish_state(weight); });
  this->publish_state(this->cat_->get_weight());
}

void CatTrackerCatWeightNumber::control(float value) {
  if (this->cat_ != nullptr)
    this->cat_->set_weight(value);
}

void CatTrackerCatWeightNumber::dump_config() { LOG_NUMBER("", "Cat Tracker Cat Weight", this); }

}  // namespace esphome::cat_tracker
