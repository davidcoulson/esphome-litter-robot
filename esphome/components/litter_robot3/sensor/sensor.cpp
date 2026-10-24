#include "sensor.h"

#include "esphome/core/log.h"

namespace esphome::litter_robot3 {

static const char *const TAG = "litter_robot3.sensor";

void LitterRobot3CleanCycleCountSensor::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field == LR3_FIELD_CLEAN_CYCLE_COUNT && this->publish_dedup_.next(value))
      this->publish_state(value);
  });
}

void LitterRobot3CleanCycleCountSensor::dump_config() { LOG_SENSOR("", "Litter Robot 3 Clean Cycles", this); }

void LitterRobot3CatSensor::setup() {
  this->parent_->add_status_callback([this](LitterRobot3Field field, uint32_t value) {
    if (field == LR3_FIELD_CAT_SENSOR && this->publish_dedup_.next(value))
      this->publish_state(value);
  });
}

void LitterRobot3CatSensor::dump_config() { LOG_SENSOR("", "Litter Robot 3 Cat Sensor", this); }

}  // namespace esphome::litter_robot3
