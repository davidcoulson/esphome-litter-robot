#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/cat_tracker/cat_tracker.h"
#include "esphome/core/component.h"

namespace esphome::cat_tracker {

class CatTrackerResetButton : public button::Button, public Component, public Parented<CatTrackerComponent> {
 public:
  void press_action() override { this->parent_->reset(); }
  void dump_config() override;
};

}  // namespace esphome::cat_tracker
