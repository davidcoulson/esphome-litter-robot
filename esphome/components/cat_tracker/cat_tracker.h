#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#ifndef CAT_TRACKER_MAX_CATS
static constexpr uint8_t CAT_TRACKER_MAX_CATS = 0;
#endif

namespace esphome::cat_tracker {

class CatTrackerCatWeightNumber;

class CatTrackerComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;

  void set_weight_sensor(sensor::Sensor *weight_sensor) { this->weight_sensor_ = weight_sensor; }
  void set_tolerance(float tolerance) { this->tolerance_ = tolerance; }

#if CAT_TRACKER_MAX_CATS > 0
  void register_cat(CatTrackerCatWeightNumber *cat);
#endif
  void reset();

 protected:
  void handle_weight_(float weight);

  sensor::Sensor *weight_sensor_{nullptr};
  float tolerance_{0.6f};
#if CAT_TRACKER_MAX_CATS > 0
  StaticVector<CatTrackerCatWeightNumber *, CAT_TRACKER_MAX_CATS> cats_;
#endif
};

}  // namespace esphome::cat_tracker
