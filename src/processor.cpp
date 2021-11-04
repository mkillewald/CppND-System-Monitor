#include "processor.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::vector;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  long total = LinuxParser::Jiffies();
  long idle = LinuxParser::IdleJiffies();
  long total_d = total - prev_total_;
  long idle_d = idle - prev_idle_;
  prev_total_ = total;
  prev_idle_ = idle;
  return (float)(total_d - idle_d) / (float)total_d;
}