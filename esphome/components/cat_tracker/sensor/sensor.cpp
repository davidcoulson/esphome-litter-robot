#include "sensor.h"
#include "esphome/core/log.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.sensor";

void CatVisitCountSensor::setup() {
  if (this->cat_number_ == nullptr)
    return;
  this->cat_number_->add_on_visit_callback([this](uint32_t count) { this->publish_state(count); });
  this->publish_state(this->cat_number_->get_visit_count());
}

void CatVisitCountSensor::dump_config() { LOG_SENSOR("", "Cat Tracker Cat Visit Count", this); }

}  // namespace esphome::cat_tracker
