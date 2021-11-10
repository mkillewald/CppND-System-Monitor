#include "system.h"

#include <unistd.h>

#include <algorithm>
#include <cstddef>
#include <functional>
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

vector<Process>& System::Processes() { return processes_; }
Processor& System::Cpu() { return cpu_; }
string System::Kernel() const { return LinuxParser::Kernel(); }
string System::OperatingSystem() const {
  return LinuxParser::OperatingSystem();
}
unsigned long System::RunningProcesses() const {
  return LinuxParser::RunningProcesses();
}
unsigned long System::TotalProcesses() const {
  return LinuxParser::TotalProcesses();
}
unsigned long System::UpTime() const { return LinuxParser::UpTime(); }
float System::MemoryUtilization() const {
  return LinuxParser::MemoryUtilization();
}
System::Sort_t System::Sort() const { return sort_; }
void System::SetSort(Sort_t s) { sort_ = s; }
bool System::Descending() const { return descending_; }
void System::SetDescending(bool d) { descending_ = d; }

void System::UpdateProcesses() {
  AddProcesses();

  for (auto& process : processes_) {
    process.Update();
  }

  RemoveProcesses();
  SortProcesses();
}

void System::AddProcesses() {
  for (unsigned int pid : LinuxParser::Pids()) {
    if (std::find(processes_.begin(), processes_.end(), pid) ==
        processes_.end()) {
      string command = LinuxParser::Command(pid);
      string user = LinuxParser::User(pid);
      if (command.empty()) {
        command = LinuxParser::Filename(pid);
      }
      processes_.emplace_back(Process(pid, user, command));
    }
  }
}

void System::RemoveProcesses() {
  // getting crashes after running for >1hour~ related to this function and
  // accessing out of range memory. Each time the pid of the Process it was
  // sorting was very large or sometimes negative. Added checks for
  // pid > TotalProcesses() to help resolve.
  std::sort(processes_.begin(), processes_.end(),
            [total = TotalProcesses()](Process& a, Process& b) {
              if (a.Pid() > total || b.Pid() > total) {
                return false;
              } else {
                return a.State() < b.State();
              }
            });

  // Verify if process has been killed, and pop it off
  while (processes_.back().isKilled()) {
    processes_.pop_back();
  }
}

void System::SortProcesses() {
  std::function<bool(Process & a, Process & b)> sort_function;
  switch (Sort()) {
    case kPid_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? a.Pid() > b.Pid() : a.Pid() < b.Pid();
      };
      break;
    }
    case kUser_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? a.User() > b.User() : a.User() < b.User();
      };
      break;
    }
    case kState_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? a.State() > b.State() : a.State() < b.State();
      };
      break;
    }
    default:
    case kCpu_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? b < a : a < b;
      };
      break;
    }
    case kRam_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? stof(a.Ram()) > stof(b.Ram())
                 : stof(a.Ram()) < stof(b.Ram());
      };
      break;
    }
    case kUpTime_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? a.UpTime() > b.UpTime() : a.UpTime() < b.UpTime();
      };
      break;
    }
    case kCommand_: {
      sort_function = [d = Descending()](Process& a, Process& b) {
        return d ? a.Command() > b.Command() : a.Command() < b.Command();
      };
      break;
    }
  }
  std::sort(processes_.begin(), processes_.end(), sort_function);
}
