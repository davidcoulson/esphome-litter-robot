#include "sensor.h"
#include "esphome/core/log.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.sensor";

void CatTrackerCountSensor::setup() {
  if (this->cat_ == nullptr)
    return;
  this->cat_->add_on_count_callback([this]() { this->publish_value_(); });
  if (this->type_ == CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_WEIGHT) {
    this->cat_->add_on_weight_callback([this](float) { this->publish_value_(); });
  }
  this->cat_->add_on_history_callback([this]() { this->publish_value_(); });
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
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_WEIGHT:
      this->publish_state(this->cat_->get_weight());
      break;
    // Last 7 full days (yesterday back); today is partial. Weight averages over visits, including today.
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_VISITS_PER_DAY_7D:
      this->publish_state(this->cat_->visits_per_day(1, 7));
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_AVG_WEIGHT_7D:
      this->publish_state(this->cat_->avg_weight(0, 7));
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_HOURS_SINCE_VISIT:
      this->publish_state(this->cat_->hours_since_visit());
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_LAST_VISIT_DURATION:
      this->publish_state(this->cat_->last_duration());
      break;
    case CatTrackerSensorType::CAT_TRACKER_SENSOR_TYPE_AVG_VISIT_DURATION_7D:
      this->publish_state(this->cat_->avg_duration(0, 7));
      break;
  }
}

void CatTrackerCountSensor::dump_config() { LOG_SENSOR("", "Cat Tracker Cat Value", this); }

}  // namespace esphome::cat_tracker
