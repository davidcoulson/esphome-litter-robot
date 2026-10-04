#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

#include <array>

namespace esphome::cat_tracker {

enum class CatTrackerCountType : uint8_t {
  CAT_TRACKER_COUNT_TYPE_VISITS = 0,
  CAT_TRACKER_COUNT_TYPE_URINE = 1,
  CAT_TRACKER_COUNT_TYPE_FECES = 2,
};

class CatTrackerCat : public Component {
 public:
  explicit CatTrackerCat(const char *id_name) : id_name_(id_name) {}

  void setup() override;
  void dump_config() override;

  // Entity name the legacy (2026.9.2) NVS key was derived from, e.g. "Cat 1 Weight".
  // Empty/null disables the one-time legacy state import.
  void set_legacy_object_id(const char *legacy_object_id) { this->legacy_object_id_ = legacy_object_id; }

  void set_weight(float weight);
  float get_weight() const { return this->weight_; }

  void record_visit();
  void record_count(CatTrackerCountType type);
  uint32_t get_count(CatTrackerCountType type) const { return this->counts_[static_cast<size_t>(type)]; }
  void set_waste_weight(CatTrackerCountType type, float weight);
  float get_waste_weight(CatTrackerCountType type) const { return this->waste_weight_[static_cast<size_t>(type)]; }
  void reset();

  template<typename F> void add_on_weight_callback(F &&callback) {
    this->weight_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_count_callback(F &&callback) {
    this->count_callback_.add(std::forward<F>(callback));
  }

 protected:
  void save_state_();
  // Imports weight/visits from the 2026.9.2 per-entity NVS key (one-time).
  void migrate_legacy_state_();

  const char *id_name_;
  const char *legacy_object_id_{nullptr};
  float weight_{NAN};
  std::array<uint32_t, 3> counts_{};
  std::array<float, 3> waste_weight_{};
  CallbackManager<void(float)> weight_callback_;
  CallbackManager<void()> count_callback_;
  ESPPreferenceObject pref_{};
};

}  // namespace esphome::cat_tracker
