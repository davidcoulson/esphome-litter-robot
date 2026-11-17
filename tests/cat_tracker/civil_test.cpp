// g++ -std=c++17 -I esphome/components/cat_tracker tests/cat_tracker/civil_test.cpp && ./a.out
#include "civil.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
using esphome::cat_tracker::civil_day;

static uint32_t local_day(time_t t) {
  struct tm lt;
  localtime_r(&t, &lt);
  return civil_day(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
}

int main() {
  assert(civil_day(1970, 1, 1) == 0);
  assert(civil_day(2000, 3, 1) == 11017);
  assert(civil_day(2026, 3, 8) == civil_day(2026, 3, 7) + 1);
  assert(civil_day(2028, 3, 1) == civil_day(2028, 2, 29) + 1);  // leap day
  setenv("TZ", "America/New_York", 1);
  tzset();
  // 2026-03-07 23:30 EST = 2026-03-08 04:30 UTC; the old offset trick put this on March 8.
  struct tm t{};
  t.tm_year = 126; t.tm_mon = 2; t.tm_mday = 7; t.tm_hour = 23; t.tm_min = 30; t.tm_isdst = -1;
  assert(local_day(mktime(&t)) == civil_day(2026, 3, 7));
  // 2026-11-01 00:30 EDT (before the fall-back); the old trick put this on October 31.
  struct tm u{};
  u.tm_year = 126; u.tm_mon = 10; u.tm_mday = 1; u.tm_hour = 0; u.tm_min = 30; u.tm_isdst = 1;
  assert(local_day(mktime(&u)) == civil_day(2026, 11, 1));
  // consecutive local midnights are consecutive days across both transitions
  for (int m : {2, 10}) {
    struct tm a{};
    a.tm_year = 126; a.tm_mon = m; a.tm_mday = 1; a.tm_hour = 12; a.tm_isdst = -1;
    time_t base = mktime(&a);
    for (int i = 1; i < 40; i++)
      assert(local_day(base + i * 86400) == local_day(base) + i);
  }
  puts("ok");
}
