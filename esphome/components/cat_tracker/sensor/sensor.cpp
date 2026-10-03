#include "sensor.h"
#include "esphome/core/log.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.sensor";

void CatTrackerCountSensor::setup() {
  if (this->cat_ == nullptr)
    return;
  this->cat_->add_on_count_callback([this]() { this->publish_value_(); });
  this->publish_value_();
}

void CatTrackerCountSensor::publish_value_() {
  switch (this->type_) {
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_VISITS_COUNT:
      this->publish_state(this->cat_->get_count(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_VISITS));
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_URINE_COUNT:
      this->publish_state(this->cat_->get_count(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_URINE));
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_FECES_COUNT:
      this->publish_state(this->cat_->get_count(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_FECES));
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_URINE_WEIGHT:
      this->publish_state(this->cat_->get_waste_weight(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_URINE));
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_FECES_WEIGHT:
      this->publish_state(this->cat_->get_waste_weight(CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_FECES));
      break;
  }
}

void CatTrackerCountSensor::dump_config() { LOG_SENSOR("", "Cat Tracker Cat Value", this); }

}  // namespace esphome::cat_tracker
