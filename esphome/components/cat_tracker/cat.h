#pragma once

#include "civil.h"
#include "dwell.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

#include <array>
#include <ctime>

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

  // weighed: weight_ was measured on this visit (not a cached or seeded value)
  void record_visit(bool weighed);
  void record_count(CatTrackerCountType type);
  uint32_t get_count(CatTrackerCountType type) const { return this->counts_[static_cast<size_t>(type)]; }
  void set_waste_weight(CatTrackerCountType type, float weight);
  float get_waste_weight(CatTrackerCountType type) const { return this->waste_weight_[static_cast<size_t>(type)]; }
  void reset();

  // Daily history (today + 15 days, flash-backed). Visits before the clock is set are held and counted once it is.
  // Averages cover the `days` full days ending `offset` days ago (offset 0 includes today).
  float visits_per_day(uint8_t offset, uint8_t days) const;
  float avg_weight(uint8_t offset, uint8_t days) const;  // NaN without visits
  uint32_t visits_in(uint8_t offset, uint8_t days) const;
  uint32_t weights_in(uint8_t offset, uint8_t days) const;
  float hours_since_visit() const;  // since the last visit, or since monitoring began if none yet; NaN without a clock
  // True once history spans at least `days_back` days before today, so a baseline window is fully covered.
  bool history_covers(uint8_t days_back) const;
  // Visit duration from BLE tag sightings. Sightings below min_rssi are ignored; a visit ends when the tag is
  // silent for longer than the gap (or on end_visit()). Durations are history-backed, the live minutes are not.
  void set_min_rssi(int8_t rssi) { this->min_rssi_ = rssi; }
  void set_gap_ms(uint32_t gap_ms) { this->gap_ms_ = gap_ms; }
  void tag_seen(int rssi);
  void end_visit();
  float minutes_in_box() const { return this->dwell_.minutes(millis()); }  // NaN when no tag is present
  float last_duration() const { return this->dur_.last == 0 ? NAN : this->dur_.last; }  // seconds
  float avg_duration(uint8_t offset, uint8_t days) const;                                 // seconds, NaN without visits
  template<typename F> void add_on_history_callback(F &&callback) {
    this->history_callback_.add(std::forward<F>(callback));
  }

  template<typename F> void add_on_weight_callback(F &&callback) {
    this->weight_callback_.add(std::forward<F>(callback));
  }
  template<typename F> void add_on_count_callback(F &&callback) {
    this->count_callback_.add(std::forward<F>(callback));
  }

 protected:
  void save_state_();
  void save_history_();
  void flush_pending_();
  void on_visit_ended_();
  uint32_t today_() const;  // local day number, 0 without a valid clock
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

  static constexpr uint8_t HISTORY_DAYS = 16;  // today + 14 completed days, one spare
  struct History {
    uint32_t day[HISTORY_DAYS];  // day number a slot holds (slot = day % 16); stale slots are ignored
    uint8_t visits[HISTORY_DAYS];
    uint8_t weight_n[HISTORY_DAYS];  // visits that carried a weight, so a visit never depends on one
    float weight_sum[HISTORY_DAYS];
    uint32_t last_visit;  // epoch seconds, 0 = never
    uint32_t started;     // epoch seconds monitoring began (first time the clock was valid), 0 = not yet
  } history_{};
  ESPPreferenceObject history_pref_{};
  // Visits seen before the clock was set; counted into today's slot once it is.
  uint8_t pending_visits_{0}, pending_weights_{0};
  float pending_weight_sum_{0};
  // Duration history lives in its own preference so the history above stays loadable.
  static constexpr uint8_t DURATION_DAYS = 14;
  struct Durations {
    uint32_t day[DURATION_DAYS];
    uint32_t sum_s[DURATION_DAYS];
    uint8_t n[DURATION_DAYS];
    uint32_t last;  // seconds, 0 = never
  } dur_{};
  ESPPreferenceObject dur_pref_{};
  VisitDwell dwell_{};
  int8_t min_rssi_{-85};
  uint32_t gap_ms_{60000};
  CallbackManager<void()> history_callback_;
};

}  // namespace esphome::cat_tracker
