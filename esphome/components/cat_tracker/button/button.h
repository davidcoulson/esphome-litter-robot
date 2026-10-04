#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/cat_tracker/cat.h"
#include "esphome/core/component.h"

namespace esphome::cat_tracker {

class CatTrackerResetButton : public button::Button, public Component, public Parented<CatTrackerCat> {
 public:
  void press_action() override;
  void dump_config() override;
};

}  // namespace esphome::cat_tracker
