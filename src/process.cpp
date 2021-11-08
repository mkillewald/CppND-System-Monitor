#include "process.h"

#include <unistd.h>

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid, string& command) : pid_(pid), command_(command) {
  user_ = LinuxParser::User(pid);
  active_ = LinuxParser::ActiveJiffies(pid);
  uptime_ = LinuxParser::UpTime(pid);
  cpu_util_ = 0.0;
};

int Process::Pid() const { return pid_; }
string Process::User() const { return user_; }
string Process::Command() const { return command_; }
long Process::UpTime() const { return LinuxParser::UpTime(Pid()); }
string Process::State() const { return LinuxParser::State(Pid()); }

float Process::CpuUtilization() const {
  long active_now = LinuxParser::ActiveJiffies(Pid());
  long uptime_now = UpTime();
  long active_d = active_now - active_;
  long uptime_d = uptime_now - uptime_;
  if (uptime_d > 0) {
    active_ = active_now;
    uptime_ = uptime_now;
    cpu_util_ =
        ((float)active_d / (float)sysconf(_SC_CLK_TCK)) / (float)uptime_d;
  }
  return cpu_util_;
}

string Process::Ram() const {
  string ram_kb = LinuxParser::Ram(Pid());
  if (ram_kb.empty()) {
    // good indication this proccess has been killed
    return "0";
  }
  return to_string(std::stof(ram_kb) / 1000.0);
}

bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}

bool Process::operator==(int const& a) const { return Pid() == a; }
// bool Process::operator==(Process const& a) const { return Pid() == a.Pid(); }