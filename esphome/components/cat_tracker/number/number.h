#pragma once

#include "esphome/components/cat_tracker/cat.h"
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"

namespace esphome::cat_tracker {

class CatTrackerCatWeightNumber : public number::Number, public Component {
 public:
  explicit CatTrackerCatWeightNumber(CatTrackerCat *cat) : cat_(cat) {}

  void setup() override;
  void dump_config() override;

 protected:
  void control(float value) override;
  CatTrackerCat *cat_{nullptr};
};

}  // namespace esphome::cat_tracker
