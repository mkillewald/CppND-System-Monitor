#include "format.h"

#include <string>

using std::string;
using std::to_string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  if (seconds < 0) {
    return "00:00:00";
  }

  long hours = seconds / 3600;
  seconds -= hours * 3600;
  long minutes = seconds / 60;
  seconds -= minutes * 60;

  string hours_str = hours < 10 ? "0" + to_string(hours) : to_string(hours);
  string minutes_str =
      minutes < 10 ? "0" + to_string(minutes) : to_string(minutes);
  string seconds_str =
      seconds < 10 ? "0" + to_string(seconds) : to_string(seconds);

  return hours_str + ":" + minutes_str + ":" + seconds_str;
}