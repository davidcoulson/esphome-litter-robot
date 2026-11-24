#include "visit_queue.h"

#include <ctime>

#include "esphome/core/log.h"

namespace esphome {
namespace visit_queue {

static const char *const TAG = "visit_queue";
static const uint32_t CLOCK_VALID = 1700000000;  // the system clock reads about 0 until a time source sets it

static uint32_t now_epoch() {
  const time_t now = ::time(nullptr);
  return now > (time_t) CLOCK_VALID ? (uint32_t) now : 0;
}

void VisitQueue::setup() {
  // Change the key if State's layout changes: an old blob must not be read as the new struct.
  this->pref_ = global_preferences->make_preference<State>(fnv1_hash("visit_queue_v1"));
  if (!this->pref_.load(&this->state_))
    this->state_ = State{};
  init(this->state_, random_uint32());
  after_boot(this->state_);
  this->save_();
  this->set_interval(1000, [this]() { this->tick_(); });
}

void VisitQueue::dump_config() {
  ESP_LOGCONFIG(TAG, "Visit queue: %u queued, next id %u, %u cats", this->state_.count,
                (unsigned) this->state_.next_id, (unsigned) this->cats_.size());
}

void VisitQueue::tick_() {
  this->uptime_s_++;
  if (expire(this->state_, this->uptime_s_, this->max_pending_s_) == 0)
    return;
  ESP_LOGW(TAG, "No verdict for a visit, sending it without one");
  this->save_();
  this->resend();
}

void VisitQueue::open(uint8_t slot, int8_t rssi, float weight) {
  if (slot > this->cats_.size())
    slot = 0;
  Record r{};
  if (visit_queue::open(this->state_, slot, rssi, weight, now_epoch(), this->uptime_s_, this->early_, &r))
    ESP_LOGW(TAG, "Queue full (Home Assistant has not acknowledged %u visits), dropped the oldest", CAPACITY);
  this->early_ = NONE;
  this->save_();
  this->emit_(r);
}

void VisitQueue::classify(bool feces) {
  Record *r = visit_queue::classify(this->state_, feces);
  if (r == nullptr) {
    // Feces is detected while the cat is still inside, before its weight is reported.
    this->early_ = feces ? FECES : URINE;
    return;
  }
  this->save_();
  this->emit_(*r);
}

void VisitQueue::visit_start() {
  // A verdict still waiting for a weight report belongs to a visit that never got one.
  this->early_ = NONE;
  if (close_pending(this->state_) == 0)
    return;
  this->save_();
  this->resend();
}

void VisitQueue::ack(uint32_t epoch, uint32_t id) {
  if (!visit_queue::ack(this->state_, epoch, id))
    return;
  ESP_LOGD(TAG, "Visit %u acknowledged, %u left", (unsigned) id, this->state_.count);
  this->save_();
}

void VisitQueue::resend() {
  // emit_ copies each record first, so an acknowledgement arriving meanwhile cannot disturb the loop.
  const State copy = this->state_;
  for (uint8_t i = 0; i < copy.count; i++)
    this->emit_(copy.recs[i]);
}

std::string VisitQueue::hello_cats() const {
  std::string out;
  for (size_t i = 0; i < this->cats_.size(); i++) {
    if (i != 0)
      out += ',';
    out += to_string(i + 1) + '=' + this->cats_[i];
  }
  return out;
}

void VisitQueue::save_() {
  this->pref_.save(&this->state_);
  // Write now: a visit must survive a reboot in the next minute.
  global_preferences->sync();
}

void VisitQueue::emit_(const Record &r) {
  static const char *const WASTE[] = {"pending", "feces", "urine"};
  Outgoing o;
  o.epoch = this->state_.epoch;
  o.id = r.id;
  o.slot = r.slot;
  const bool known = r.slot >= 1 && r.slot <= this->cats_.size();
  o.slot = known ? r.slot : 0;
  o.name = known ? this->cats_[r.slot - 1] : "Unknown";
  o.method = known ? "Tag" : "None";
  o.waste = WASTE[r.waste <= URINE ? r.waste : PENDING];
  o.rssi = r.rssi;
  o.weight = r.weight;
  o.time = r.time;
  o.age = age(r, now_epoch(), this->uptime_s_);
  o.final_ = r.final_;
  this->on_record_callback_.call(o);
}

}  // namespace visit_queue
}  // namespace esphome
