#pragma once

#include "esphome/components/cat_tracker/cat_tracker.h"
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome::cat_tracker {

class CatTrackerCatWeightNumber : public number::Number, public Component, public Parented<CatTrackerComponent> {
 public:
  void setup() override;
  void dump_config() override;

  void publish_weight(float weight);
  void reset();
  uint32_t get_visit_count() const { return this->visit_count_; }
  template<typename F> void add_on_visit_callback(F &&callback) {
    this->on_visit_callback_.add(std::forward<F>(callback));
  }

 protected:
  void control(float value) override;
  void save_state_();
  uint32_t visit_count_{0};
  CallbackManager<void(uint32_t)> on_visit_callback_;
  ESPPreferenceObject pref_{};
};

}  // namespace esphome::cat_tracker
