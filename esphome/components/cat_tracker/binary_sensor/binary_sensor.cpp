#include "binary_sensor.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.binary_sensor";

void CatTrackerAlert::setup() {
  this->cat_->add_on_history_callback([this]() { this->update_(); });
  if (this->type_ == CatTrackerAlertType::CAT_TRACKER_ALERT_IN_BOX_TOO_LONG)
    this->set_interval(10000, [this]() { this->update_(); });
  this->update_();
}

void CatTrackerAlert::update_() {
  bool on = false;
  switch (this->type_) {
    case CatTrackerAlertType::CAT_TRACKER_ALERT_WEIGHT_CHANGED: {
      // Last 3 days against the 10 days before; needs 3 visits in each so one odd reading cannot trip it.
      float recent = this->cat_->avg_weight(0, 3), base = this->cat_->avg_weight(3, 10);
      on = this->cat_->visits_in(0, 3) >= 3 && this->cat_->visits_in(3, 10) >= 3 && base > 0 &&
           std::abs(recent - base) / base * 100.0f > this->threshold_;
      break;
    }
    case CatTrackerAlertType::CAT_TRACKER_ALERT_VISITS_CHANGED: {
      // Last 7 full days against the 7 before; needs a week of earlier data (at least 7 visits).
      float recent = this->cat_->visits_per_day(1, 7), base = this->cat_->visits_per_day(8, 7);
      on = this->cat_->visits_in(8, 7) >= 7 && std::abs(recent - base) / base * 100.0f > this->threshold_;
      break;
    }
    case CatTrackerAlertType::CAT_TRACKER_ALERT_NO_VISIT: {
      float hours = this->cat_->hours_since_visit();
      on = !std::isnan(hours) && hours > this->threshold_;
      break;
    }
    case CatTrackerAlertType::CAT_TRACKER_ALERT_IN_BOX_TOO_LONG: {
      // A tag has been heard continuously (see the cat's min_rssi and gap) for longer than the threshold.
      float minutes = this->cat_->minutes_in_box();
      on = !std::isnan(minutes) && minutes > this->threshold_;
      break;
    }
  }
  if (!this->has_state() || on != this->state)
    this->publish_state(on);
}

void CatTrackerAlert::dump_config() {
  LOG_BINARY_SENSOR("", "Cat Tracker Alert", this);
  ESP_LOGCONFIG(TAG, "  Threshold: %.1f", this->threshold_);
}

}  // namespace esphome::cat_tracker
