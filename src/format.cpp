#include "format.h"

#include <string>

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  long hours = seconds / 3600;
  long minutes = seconds % 3600 / 60;
  long secs = seconds % 60;

  string hours_str =
      hours > 9 ? std::to_string(hours) : "0" + std::to_string(hours);
  string minutes_str = minutes > 9 ? ":" + std::to_string(minutes)
                                   : ":0" + std::to_string(minutes);
  string secs_str =
      secs > 9 ? ":" + std::to_string(secs) : ":0" + std::to_string(secs);

  return hours_str + minutes_str + secs_str;
}
