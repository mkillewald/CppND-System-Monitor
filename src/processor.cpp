#include "processor.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::vector;

float Processor::Utilization() {
  unsigned long total_now = LinuxParser::Jiffies();
  unsigned long idle_now = LinuxParser::IdleJiffies();
  unsigned long total_d = total_now - total_;
  unsigned long idle_d = idle_now - idle_;
  if (total_d > 0) {
    total_ = total_now;
    idle_ = idle_now;
    return (float)(total_d - idle_d) / (float)total_d;
  }
  return 0.0;
}