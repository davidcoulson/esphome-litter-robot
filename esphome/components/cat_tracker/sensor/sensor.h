#pragma once

#include "esphome/components/cat_tracker/cat_tracker.h"
#include "esphome/components/cat_tracker/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome::cat_tracker {

class CatVisitCountSensor : public sensor::Sensor, public Component, public Parented<CatTrackerComponent> {
 public:
  void setup() override;
  void dump_config() override;
  void set_cat_number(CatTrackerCatWeightNumber *cat) { this->cat_number_ = cat; }

 protected:
  CatTrackerCatWeightNumber *cat_number_{nullptr};
};

}  // namespace esphome::cat_tracker
