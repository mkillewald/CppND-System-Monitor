#include "processor.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::vector;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  vector<string> values = LinuxParser::CpuUtilization();

  user_ = stol(values[1]);
  nice_ = stol(values[2]);
  system_ = stol(values[3]);
  idle_ = stol(values[4]);
  iowait_ = stol(values[5]);
  irq_ = stol(values[6]);
  softirq_ = stol(values[7]);
  steal_ = stol(values[8]);
  guest_ = stol(values[9]);
  guest_nice_ = stol(values[10]);

  float idle = idle_ + iowait_;
  float non_idle = user_ + nice_ + system_ + irq_ + softirq_ + steal_;
  float total = idle + non_idle;

  return non_idle / total;
}