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

Process::Process(unsigned int pid, string user, string command)
    : pid_(pid), user_(user), command_(command) {
  active_ = LinuxParser::ActiveJiffies(pid);
  uptime_ = LinuxParser::UpTime(pid);
  killed_ = false;
};

unsigned int Process::Pid() const { return pid_; }
string Process::User() const { return user_; }
string Process::Command() const { return command_; }
unsigned long Process::Active() const {
  return LinuxParser::ActiveJiffies(Pid());
}
unsigned long Process::UpTime() const { return LinuxParser::UpTime(Pid()); }
string Process::Ram() const { return LinuxParser::Ram(Pid()); }

string Process::State() const {
  string state = LinuxParser::State(Pid());
  if (state.empty()) {
    // process state was not read from file, so this is a good indication that
    // this proccess has been killed. Set killed_ to true and set state to "~"
    // to allow for sorting by state to remove killed processes;
    killed_ = true;
    return "~";
  }
  return state;
}

bool Process::isKilled() const { return killed_; }

float Process::CpuUtilization() const {
  unsigned long active_now = LinuxParser::ActiveJiffies(Pid());
  unsigned long uptime_now = UpTime();
  unsigned long uptime_d = uptime_now - uptime_;
  if (uptime_d > 0) {
    float active_d =
        (float)(active_now - active_) / (float)sysconf(_SC_CLK_TCK);
    active_ = active_now;
    uptime_ = uptime_now;
    cpu_util_ = active_d / (float)uptime_d;
  }
  return cpu_util_;
}

bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}

bool Process::operator==(unsigned int const& a) const { return Pid() == a; }
bool Process::operator==(Process const& a) const { return Pid() == a.Pid(); }