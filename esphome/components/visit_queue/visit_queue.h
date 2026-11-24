#pragma once
#include <string>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "queue.h"

namespace esphome {
namespace visit_queue {

// One record as it goes to Home Assistant.
struct Outgoing {
  uint32_t epoch;
  uint32_t id;
  uint8_t slot;
  std::string name;
  std::string method;
  std::string waste;
  int rssi;
  float weight;
  uint32_t time;
  int32_t age;
  bool final_;
};

class VisitQueue : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void add_cat(const std::string &name) { this->cats_.push_back(name); }
  void set_max_pending_s(uint32_t seconds) { this->max_pending_s_ = seconds; }

  // A weight report was accepted as a visit. slot 0 = no cat identified.
  void open(uint8_t slot, int8_t rssi, float weight);
  // WasteSense verdict. If the weight report has not arrived yet it is kept for the next open().
  void classify(bool feces);
  // A new visit began. Call after WasteSense has closed the previous visit.
  void visit_start();
  void ack(uint32_t epoch, uint32_t id);
  void resend();

  uint32_t unidentified() const { return this->state_.unidentified; }
  uint8_t queued() const { return this->state_.count; }
  std::string hello_cats() const;

  template<typename F> void add_on_record_callback(F &&callback) {
    this->on_record_callback_.add(std::forward<F>(callback));
  }

 protected:
  void save_();
  void emit_(const Record &r);
  void tick_();

  State state_{};
  ESPPreferenceObject pref_;
  std::vector<std::string> cats_;
  uint32_t max_pending_s_{2700};
  uint32_t uptime_s_{0};
  int8_t early_{NONE};
  CallbackManager<void(const Outgoing &)> on_record_callback_;
};

}  // namespace visit_queue
}  // namespace esphome
