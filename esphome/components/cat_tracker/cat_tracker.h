#pragma once

#include "esphome/components/cat_tracker/cat.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#ifndef CAT_TRACKER_MAX_CATS
static constexpr uint8_t CAT_TRACKER_MAX_CATS = 0;
#endif

namespace esphome::cat_tracker {

class CatTrackerComponent : public Component {
 public:
  explicit CatTrackerComponent(float weight_tolerance = 0.6f) : weight_tolerance_(weight_tolerance) {}

  void dump_config() override;

#if CAT_TRACKER_MAX_CATS > 0
  void register_cat(CatTrackerCat *cat);
#endif
  void reset();
  CatTrackerCat *match_weight(float weight);
  void register_visit(CatTrackerCat *cat);
  void record_waste(CatTrackerCat *cat, CatTrackerCountType type);
  // Stores the most recent waste weight (ounces) for the given waste type.
  void record_waste_weight(CatTrackerCat *cat, CatTrackerCountType type, float weight);

  template<typename F> void add_on_visit_callback(F &&callback) {
    this->on_visit_callback_.add(std::forward<F>(callback));
  }

 protected:
  float weight_tolerance_;
  LazyCallbackManager<void()> on_visit_callback_;
#if CAT_TRACKER_MAX_CATS > 0
  StaticVector<CatTrackerCat *, CAT_TRACKER_MAX_CATS> cats_;
#endif
};

}  // namespace esphome::cat_tracker
