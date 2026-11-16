#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/cat_tracker/cat.h"
#include "esphome/core/component.h"

namespace esphome::cat_tracker {

enum class CatTrackerAlertType : uint8_t {
  CAT_TRACKER_ALERT_WEIGHT_CHANGED = 0,
  CAT_TRACKER_ALERT_VISITS_CHANGED = 1,
  CAT_TRACKER_ALERT_NO_VISIT = 2,
};

// On when a cat's recent history departs from its own baseline. Not medical advice: "changed", not "sick".
class CatTrackerAlert : public binary_sensor::BinarySensor, public Component {
 public:
  CatTrackerAlert(CatTrackerCat *cat, CatTrackerAlertType type) : cat_(cat), type_(type) {}
  void set_threshold(float threshold) { this->threshold_ = threshold; }

  void setup() override;
  void dump_config() override;

 protected:
  void update_();
  CatTrackerCat *cat_;
  CatTrackerAlertType type_;
  float threshold_{0};  // weight/visits: percent change; no_visit: hours
};

}  // namespace esphome::cat_tracker
