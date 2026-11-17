#pragma once

#include <cstdint>

namespace esphome::cat_tracker {

// Days since 1970-01-01 for a civil (calendar) date: month 1-12, day 1-31. Taking the local calendar date
// from localtime_r() avoids guessing the UTC offset, which is wrong near DST changes.
constexpr uint32_t civil_day(int y, int m, int d) {
  y -= m <= 2;
  const int era = y / 400;  // years >= 1970 only
  const int yoe = y - era * 400;
  const int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return static_cast<uint32_t>(era * 146097 + doe - 719468);
}

}  // namespace esphome::cat_tracker
