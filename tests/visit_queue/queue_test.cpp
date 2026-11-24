// g++ -std=c++17 -I esphome/components/visit_queue tests/visit_queue/queue_test.cpp -o /tmp/vq && /tmp/vq
#include "queue.h"
#include <cassert>
#include <cstdio>
#include <type_traits>
using namespace esphome::visit_queue;

static_assert(std::is_trivially_copyable<State>::value, "State is stored in flash as-is");

static State fresh() {
  State s{};
  init(s, 0x12345678u);
  return s;
}

int main() {
  // init: a random epoch that fits a signed 32-bit action variable; a second init changes nothing
  State s{};
  init(s, 0xFFFFFFFFu);
  assert(s.epoch == 0x7FFFFFFFu && s.next_id == 1 && s.count == 0);
  init(s, 5);
  assert(s.epoch == 0x7FFFFFFFu);
  State z{};
  init(z, 0x80000000u);  // masks to 0, which means "never initialised"
  assert(z.epoch == 1);

  // open: ids count up; slot 0 bumps the unidentified counter
  s = fresh();
  Record out{};
  assert(!open(s, 4, -52, 11.46f, 1000, 10, NONE, &out));
  assert(out.id == 1 && out.slot == 4 && out.rssi == -52 && out.time == 1000 && out.uptime_s == 10);
  assert(out.waste == PENDING && !out.final_);
  open(s, 0, -90, 9.0f, 1100, 20, NONE, &out);
  open(s, 2, -60, 10.0f, 1200, 30, NONE, &out);
  assert(out.id == 3 && s.count == 3 && s.next_id == 4 && s.unidentified == 1);

  // classify: the newest pending record; nothing waiting -> nullptr (Review Focus 1)
  Record *r = classify(s, true);
  assert(r != nullptr && r->id == 3 && r->waste == FECES && r->final_);
  r = classify(s, false);
  assert(r != nullptr && r->id == 2 && r->waste == URINE && r->final_);
  r = classify(s, false);
  assert(r != nullptr && r->id == 1);
  assert(classify(s, true) == nullptr);

  // a verdict that came before the weight report is applied when the record opens
  s = fresh();
  open(s, 4, -52, 11.46f, 1000, 10, FECES, &out);
  assert(out.waste == FECES && out.final_ && s.recs[0].final_);
  assert(classify(s, false) == nullptr);

  // ack: only a final record of this epoch (Review Focus 3)
  s = fresh();
  open(s, 4, -52, 11.46f, 1000, 10, NONE, &out);
  open(s, 5, -52, 11.0f, 1001, 11, NONE, &out);
  assert(!ack(s, s.epoch, 1));      // still pending
  assert(!ack(s, s.epoch, 99));     // unknown id
  s.recs[0].final_ = true;
  assert(!ack(s, s.epoch + 1, 1));  // wrong epoch
  assert(ack(s, s.epoch, 1));
  assert(s.count == 1 && s.recs[0].id == 2);
  assert(!ack(s, s.epoch, 1));      // already gone

  // overflow: the oldest is dropped, ids keep counting (Review Focus 4)
  s = fresh();
  for (int i = 0; i < CAPACITY; i++)
    assert(!open(s, 0, -80, 9.0f, 0, i, NONE, &out));
  assert(open(s, 3, -50, 9.0f, 0, 99, NONE, &out));
  assert(s.count == CAPACITY && s.recs[0].id == 2 && s.recs[CAPACITY - 1].id == 17 && s.next_id == 18);
  assert(s.unidentified == 16);

  // after_boot: no verdict will come for a pending record, so it becomes final as it is (Review Focus 2)
  s = fresh();
  open(s, 4, -52, 11.46f, 1000, 10, NONE, &out);
  after_boot(s);
  assert(s.recs[0].final_ && s.recs[0].waste == PENDING && s.recs[0].uptime_s == UNKNOWN_UPTIME);
  s.count = 200;  // corrupt flash content must not index past the array
  after_boot(s);
  assert(s.count == 0);

  // expire: pending longer than the limit becomes final
  s = fresh();
  open(s, 1, -52, 11.0f, 0, 1000, NONE, &out);
  open(s, 2, -52, 11.0f, 0, 3000, NONE, &out);
  assert(expire(s, 4000, 2700) == 1);
  assert(s.recs[0].final_ && s.recs[0].waste == PENDING && !s.recs[1].final_);
  assert(expire(s, 4000, 2700) == 0);

  // close_pending: a new visit began, so a record still pending will never get its own verdict
  // (the robot rebooted mid-visit); it must not take the next visit's
  s = fresh();
  open(s, 1, -52, 11.0f, 0, 10, NONE, &out);
  s.recs[0].final_ = true;
  open(s, 2, -52, 11.0f, 0, 20, NONE, &out);
  assert(close_pending(s) == 1);
  assert(s.recs[1].final_ && s.recs[1].waste == PENDING);
  assert(close_pending(s) == 0 && classify(s, true) == nullptr);

  // age (Review Focus 5)
  Record a{};
  a.time = 1000; a.uptime_s = 500;
  assert(age(a, 1060, 530) == 60);       // both clocks valid: wall clock wins
  assert(age(a, 0, 530) == 30);          // clock lost: uptime
  assert(age(a, 900, 530) == 30);        // clock went backwards: uptime
  a.time = 0;
  assert(age(a, 1060, 530) == 30);
  a.uptime_s = UNKNOWN_UPTIME;
  assert(age(a, 1060, 530) == -1);
  a.time = 1000;
  assert(age(a, 0, 530) == -1);
  puts("ok");
}
