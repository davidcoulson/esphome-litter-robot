#include "button.h"
#include "esphome/core/log.h"

namespace esphome::cat_tracker {

static const char *const TAG = "cat_tracker.button";

void CatTrackerResetButton::dump_config() { LOG_BUTTON("", "Cat Tracker Reset", this); }

}  // namespace esphome::cat_tracker
