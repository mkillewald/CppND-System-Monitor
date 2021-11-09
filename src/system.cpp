#include "system.h"

#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() { return cpu_; }
std::string System::Kernel() const { return LinuxParser::Kernel(); }
int System::RunningProcesses() const { return LinuxParser::RunningProcesses(); }
int System::TotalProcesses() const { return LinuxParser::TotalProcesses(); }
long int System::UpTime() const { return LinuxParser::UpTime(); }

float System::MemoryUtilization() const {
  return LinuxParser::MemoryUtilization();
}

std::string System::OperatingSystem() const {
  return LinuxParser::OperatingSystem();
}

int System::GetSort() const { return sort_; }
void System::SetSort(int s) { sort_ = s; }

/*
 * Add new processes to processes_ vector
 */
void System::AddProcesses() {
  for (int& pid : LinuxParser::Pids()) {
    if (std::find(processes_.begin(), processes_.end(), pid) ==
        processes_.end()) {
      string command = LinuxParser::Command(pid);
      if (!command.empty()) {
        processes_.emplace_back(Process(pid, command));
      }
    }
  }
}

/*
 * Remove killed processes from the processes_ vector
 */
void System::RemoveProcesses() {
  // Sort by State(). Killed processes will have an empty string because it
  // could not be read from file.
  std::sort(processes_.begin(), processes_.end(),
            [](Process& a, Process& b) { return a.State() > b.State(); });

  // Verify state of process is an empty string, and pop it off
  while (processes_.size() > 0 && processes_.back().State().empty()) {
    processes_.pop_back();
  }
}

void System::SortProcesses() {
  std::function<bool(Process & a, Process & b)> sort_function;
  switch (sort_) {
    default:
    case Sort::kMaxCpu_: {
      sort_function = [](Process& a, Process& b) { return b < a; };
      break;
    }
    case Sort::kMinCpu_: {
      sort_function = [](Process& a, Process& b) { return a < b; };
      break;
    }
    case Sort::kMaxRam_: {
      sort_function = [](Process& a, Process& b) {
        return stof(a.Ram()) > stof(b.Ram());
      };
      break;
    }
    case Sort::kMinRam_: {
      sort_function = [](Process& a, Process& b) {
        return stof(a.Ram()) < stof(b.Ram());
      };
      break;
    }
    case Sort::kMaxPid_: {
      sort_function = [](Process& a, Process& b) { return a.Pid() > b.Pid(); };
      break;
    }
    case Sort::kMinPid_: {
      sort_function = [](Process& a, Process& b) { return a.Pid() < b.Pid(); };
      break;
    }
    case Sort::kMaxState_: {
      sort_function = [](Process& a, Process& b) {
        return a.State() > b.State();
      };
      break;
    }
    case Sort::kMinState_: {
      sort_function = [](Process& a, Process& b) {
        return a.State() < b.State();
      };
      break;
    }
  }
  std::sort(processes_.begin(), processes_.end(), sort_function);
}

vector<Process>& System::Processes() {
  RemoveProcesses();
  AddProcesses();
  SortProcesses();
  return processes_;
}
