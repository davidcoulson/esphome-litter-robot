#pragma once

#include "esphome/core/automation.h"
#include "cat_tracker.h"

namespace esphome::cat_tracker {

template<typename... Ts>
class CatTrackerRecordVisitAction final : public Action<Ts...>, public Parented<CatTrackerComponent> {
 public:
  TEMPLATABLE_VALUE(float, weight)
  TEMPLATABLE_VALUE(float, waste_weight)

  void set_cat(CatTrackerCat *cat) { this->cat_ = cat; }
  void set_waste_type(CatTrackerCountType waste_type) {
    this->waste_type_ = waste_type;
    this->has_waste_ = true;
  }

  void play(const Ts &...x) override {
    CatTrackerCat *cat = this->cat_;
    if (cat == nullptr) {
      cat = this->parent_->match_weight(this->weight_.value(x...));
    }
    if (cat == nullptr)
      return;
    this->parent_->register_visit(cat);
    if (this->has_waste_) {
      this->parent_->record_waste(cat, this->waste_type_);
    }
    if (this->waste_weight_.has_value()) {
      this->parent_->record_waste_weight(cat, this->waste_type_, this->waste_weight_.value(x...));
    }
  }

 protected:
  CatTrackerCat *cat_{nullptr};
  CatTrackerCountType waste_type_{CatTrackerCountType::CAT_TRACKER_COUNT_TYPE_FECES};
  bool has_waste_{false};
};

template<typename... Ts> class CatTrackerTagSeenAction final : public Action<Ts...> {
 public:
  TEMPLATABLE_VALUE(int, rssi)
  void set_cat(CatTrackerCat *cat) { this->cat_ = cat; }
  void play(const Ts &...x) override { this->cat_->tag_seen(this->rssi_.value(x...)); }

 protected:
  CatTrackerCat *cat_{nullptr};
};

template<typename... Ts>
class CatTrackerVisitEndedAction final : public Action<Ts...>, public Parented<CatTrackerComponent> {
 public:
  void set_cat(CatTrackerCat *cat) { this->cat_ = cat; }
  void play(const Ts &...x) override { this->parent_->end_visits(this->cat_); }

 protected:
  CatTrackerCat *cat_{nullptr};
};

}  // namespace esphome::cat_tracker
