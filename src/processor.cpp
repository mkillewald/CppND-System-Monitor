#include "processor.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::vector;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  vector<string> values = LinuxParser::CpuUtilization();

  long user = stol(values[1]);
  long nice = stol(values[2]);
  long system = stol(values[3]);
  long idle = stol(values[4]);
  long iowait = stol(values[5]);
  long irq = stol(values[6]);
  long softirq = stol(values[7]);
  long steal = stol(values[8]);

  // Calculation based on question and answer by Vangelis Tasoulas at:
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux

  idle += iowait;
  long non_idle = user + nice + system + irq + softirq + steal;
  long total = idle + non_idle;
  long total_d = total - prev_total_;
  long idle_d = idle - prev_idle_;
  prev_total_ = total;
  prev_idle_ = idle;

  return (float)(total_d - idle_d) / (float)total_d;
}