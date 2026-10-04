#include "button.h"
#include "esphome/core/log.h"
#include "esphome/core/preferences.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.button";

void CatTrackerResetButton::press_action() {
  this->parent_->reset();
  global_preferences->sync();
}

void CatTrackerResetButton::dump_config() { LOG_BUTTON("", "Cat Tracker Reset", this); }

}  // namespace esphome::cat_tracker
