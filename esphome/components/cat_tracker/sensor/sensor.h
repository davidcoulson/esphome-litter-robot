#pragma once

#include "esphome/components/cat_tracker/cat.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome::cat_tracker {

enum class CatTrackerSensorType : uint8_t {
  CAT_TRACKER_SENSOR_TYPE_VISITS_COUNT = 0,
  CAT_TRACKER_SENSOR_TYPE_URINE_COUNT = 1,
  CAT_TRACKER_SENSOR_TYPE_FECES_COUNT = 2,
  CAT_TRACKER_SENSOR_TYPE_URINE_WEIGHT = 3,
  CAT_TRACKER_SENSOR_TYPE_FECES_WEIGHT = 4,
  CAT_TRACKER_SENSOR_TYPE_WEIGHT = 5,
};

// Thin entity that exposes one of a cat's counters, waste weights, or weight.
class CatTrackerCountSensor : public sensor::Sensor, public Component {
 public:
  CatTrackerCountSensor(CatTrackerCat *cat, CatTrackerSensorType type) : cat_(cat), type_(type) {}

  void setup() override;
  void dump_config() override;

 protected:
  void publish_value_();
  CatTrackerCat *cat_{nullptr};
  CatTrackerSensorType type_;
};

}  // namespace esphome::cat_tracker
