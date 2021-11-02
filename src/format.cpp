#include "format.h"

#include <string>

using std::string;
using std::to_string;

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR 3600

template <typename T>
string Format::ZeroedString(T value) {
  return value < 10 ? "0" + to_string(value) : to_string(value);
}

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  if (seconds < 0) {
    return "00:00:00";
  }

  long hours = seconds / SECONDS_PER_HOUR;
  seconds %= SECONDS_PER_HOUR;
  long minutes = seconds / SECONDS_PER_MINUTE;
  seconds %= SECONDS_PER_MINUTE;

  return ZeroedString(hours) + ":" + ZeroedString(minutes) + ":" +
         ZeroedString(seconds);
}