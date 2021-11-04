#include "processor.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::vector;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  vector<string> values = LinuxParser::CpuUtilization();

  if (values.size() > CPU_STEAL_INDEX) {
    long user = stol(values.at(CPU_USER_INDEX));
    long nice = stol(values.at(CPU_NICE_INDEX));
    long system = stol(values.at(CPU_SYSTEM_INDEX));
    long idle = stol(values.at(CPU_IDLE_INDEX));
    long iowait = stol(values.at(CPU_IOWAIT_INDEX));
    long irq = stol(values.at(CPU_IRQ_INDEX));
    long softirq = stol(values.at(CPU_SOFTIRQ_INDEX));
    long steal = stol(values.at(CPU_STEAL_INDEX));

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
  return 0.0;
}