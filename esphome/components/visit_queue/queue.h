#pragma once
// Visit queue logic for the Clowder package. No ESPHome includes, so it has host tests
// (tests/visit_queue). State is stored in flash as-is: keep it trivially copyable, and
// change the preference key in visit_queue.cpp if the layout changes.
#include <cstdint>

namespace esphome {
namespace visit_queue {

static const uint8_t CAPACITY = 16;
static const uint32_t UNKNOWN_UPTIME = 0xFFFFFFFF;
enum Waste : uint8_t { PENDING = 0, FECES = 1, URINE = 2 };
static const int8_t NONE = -1;  // no verdict arrived ahead of the weight report

struct Record {
  uint32_t id;
  uint32_t time;      // epoch seconds, 0 = clock not valid at the visit
  uint32_t uptime_s;  // robot uptime at the visit; UNKNOWN_UPTIME after a reboot
  float weight;       // lb
  int8_t rssi;
  uint8_t slot;       // 0 = unidentified
  uint8_t waste;
  bool final_;
};

struct State {
  uint32_t epoch;  // 0 = never initialised
  uint32_t next_id;
  uint32_t unidentified;
  uint8_t count;
  Record recs[CAPACITY];  // oldest first
};

// Give a blank State its epoch: 1..0x7FFFFFFF, because Home Assistant sends it back in a
// signed 32-bit action variable.
inline void init(State &s, uint32_t random) {
  if (s.epoch != 0)
    return;
  s.epoch = random & 0x7FFFFFFF;
  if (s.epoch == 0)
    s.epoch = 1;
  s.next_id = 1;
  s.unidentified = 0;
  s.count = 0;
}

inline void remove_at(State &s, uint8_t index) {
  for (uint8_t i = index; i + 1 < s.count; i++)
    s.recs[i] = s.recs[i + 1];
  s.count--;
}

// Call once after loading from flash. WasteSense forgot any visit in progress, so a pending
// record will never get its verdict: make it final as it is so it can be delivered.
inline void after_boot(State &s) {
  if (s.count > CAPACITY)
    s.count = 0;
  for (uint8_t i = 0; i < s.count; i++) {
    s.recs[i].uptime_s = UNKNOWN_UPTIME;
    s.recs[i].final_ = true;
  }
}

// Returns true if the oldest record had to be dropped to make room.
// early: a verdict that arrived before the weight report (FECES/URINE), or NONE.
inline bool open(State &s, uint8_t slot, int8_t rssi, float weight, uint32_t time, uint32_t uptime_s, int8_t early,
                 Record *out) {
  bool dropped = false;
  if (s.count >= CAPACITY) {
    remove_at(s, 0);
    dropped = true;
  }
  Record &r = s.recs[s.count++];
  r.id = s.next_id++;
  r.time = time;
  r.uptime_s = uptime_s;
  r.weight = weight;
  r.rssi = rssi;
  r.slot = slot;
  r.waste = early == NONE ? (uint8_t) PENDING : (uint8_t) early;
  r.final_ = early != NONE;
  if (slot == 0)
    s.unidentified++;
  if (out != nullptr)
    *out = r;
  return dropped;
}

// Sets the verdict on the newest record still waiting for one; nullptr if none is waiting.
// WasteSense closes the previous visit before the next one's weight report, so the newest
// pending record is always the visit the verdict belongs to.
inline Record *classify(State &s, bool feces) {
  for (int i = (int) s.count - 1; i >= 0; i--) {
    if (!s.recs[i].final_) {
      s.recs[i].waste = feces ? FECES : URINE;
      s.recs[i].final_ = true;
      return &s.recs[i];
    }
  }
  return nullptr;
}

// Home Assistant has counted the record: forget it. Only a final record of this epoch.
inline bool ack(State &s, uint32_t epoch, uint32_t id) {
  if (epoch != s.epoch)
    return false;
  for (uint8_t i = 0; i < s.count; i++) {
    if (s.recs[i].id == id) {
      if (!s.recs[i].final_)
        return false;
      remove_at(s, i);
      return true;
    }
  }
  return false;
}

// A new visit began: any record still pending missed its verdict (reboot mid-visit) and must
// not take the new visit's. Make it final as it is. Returns how many.
inline uint8_t close_pending(State &s) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < s.count; i++) {
    if (!s.recs[i].final_) {
      s.recs[i].final_ = true;
      n++;
    }
  }
  return n;
}

// A record pending longer than max_pending_s will not get a verdict: make it final. Returns how many.
inline uint8_t expire(State &s, uint32_t uptime_s, uint32_t max_pending_s) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < s.count; i++) {
    Record &r = s.recs[i];
    if (!r.final_ && r.uptime_s != UNKNOWN_UPTIME && uptime_s - r.uptime_s > max_pending_s) {
      r.final_ = true;
      n++;
    }
  }
  return n;
}

// Seconds since the visit: by wall clock when both readings are valid, else by uptime, else -1.
inline int32_t age(const Record &r, uint32_t now_epoch, uint32_t uptime_s) {
  if (r.uptime_s != UNKNOWN_UPTIME && uptime_s >= r.uptime_s && (r.time == 0 || now_epoch < r.time))
    return (int32_t) (uptime_s - r.uptime_s);
  if (r.time != 0 && now_epoch >= r.time)
    return (int32_t) (now_epoch - r.time);
  return -1;
}

}  // namespace visit_queue
}  // namespace esphome
