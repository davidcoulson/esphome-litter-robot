// g++ -std=c++17 -I esphome/components/cat_tracker tests/cat_tracker/dwell_test.cpp && ./a.out
#include "dwell.h"
#include <cassert>
#include <cstdio>
using esphome::cat_tracker::VisitDwell;

int main() {
  const uint32_t gap = 60000;
  VisitDwell d;
  assert(std::isnan(d.minutes(0)));
  assert(!d.seen(1000, gap));
  assert(!d.seen(30000, gap));          // within the gap: same visit
  assert(!d.seen(80000, gap));          // 50 s later: still the same visit
  assert(d.minutes(121000) == 2.0f);
  assert(!d.expire(130000, gap));       // 50 s of silence: tolerated
  assert(d.expire(141001, gap));        // > 60 s: ended
  assert(d.ended_s == 79 && !d.active); // 80000 - 1000
  assert(!d.expire(200000, gap));       // already ended
  assert(!d.seen(300000, gap));         // new visit
  assert(d.seen(400000, gap));          // gap exceeded by a late sighting: old one ended, new one started
  assert(d.ended_s == 0 && d.active && d.start == 400000);  // single sighting = 0 s
  assert(d.finish() && !d.finish());
  // millis() wrap
  VisitDwell w;
  w.seen(0xFFFFFF00u, gap);
  assert(!w.seen(0x00001000u, gap));
  assert(w.ended_s == 0 && w.minutes(0x00001000u) > 0);
  puts("ok");
}
