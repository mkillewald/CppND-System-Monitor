#include "processor.h"

#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::vector;

Processor::Processor() {
  id_ = -1;
  total_ = 0;
  idle_ = 0;
  cpu_util_ = 0.0;
}
Processor::Processor(int id) : id_(id) {
  total_ = 0;
  idle_ = 0;
  cpu_util_ = 0.0;
}
int Processor::Id() const { return id_; }
unsigned long Processor::Jiffies() const { return total_; }
unsigned long Processor::IdleJiffies() const { return idle_; }
float Processor::Utilization() const { return cpu_util_; }

void Processor::SetJiffies(unsigned long total) { total_ = total; }
void Processor::SetIdleJiffies(unsigned long idle) { idle_ = idle; }
void Processor::SetCpuUtilization(float cpu_util) { cpu_util_ = cpu_util; }

void Processor::Update() {
  unsigned long total_now = LinuxParser::Jiffies(Id());
  unsigned long idle_now = LinuxParser::IdleJiffies(Id());
  unsigned long total_d = total_now - Jiffies();
  unsigned long idle_d = idle_now - IdleJiffies();
  if (total_d > 0) {
    SetJiffies(total_now);
    SetIdleJiffies(idle_now);
    SetCpuUtilization((float)(total_d - idle_d) / (float)total_d);
  }
}