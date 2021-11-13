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
  cpu_util_ = 0;
  killed_ = false;
};

unsigned int Process::Pid() const { return pid_; }
string Process::User() const { return user_; }
string Process::Command(unsigned int len) const {
  if (len > 0 && len <= 6) {
    return command_.substr(0, len);
  } else if (len > 0 && command_.size() > len) {
    return command_.substr(0, len - 6) + " (...)";
  }
  return command_;
}
unsigned long Process::Active() const { return active_; }
unsigned long Process::UpTime() const { return uptime_; }
float Process::CpuUtilization() const { return cpu_util_; }
string Process::Ram() const { return ram_; }
string Process::State() const { return state_; }
bool Process::isKilled() const { return killed_; }

void Process::SetActive(unsigned long active) { active_ = active; }
void Process::SetUpTime(unsigned long uptime) { uptime_ = uptime; }
void Process::SetCpuUtilization(float cpu_util) { cpu_util_ = cpu_util; }
void Process::SetRam(string ram) { ram_ = ram; }
void Process::SetState(string state) { state_ = state; }
void Process::SetKilled(bool k) { killed_ = k; }

void Process::UpdateCpuUtilization() {
  unsigned long active_now = LinuxParser::ActiveJiffies(Pid());
  unsigned long uptime_now = LinuxParser::UpTime(Pid());
  unsigned long uptime_d = uptime_now - uptime_;
  if (uptime_d > 0) {
    float active_d =
        (float)(active_now - active_) / (float)sysconf(_SC_CLK_TCK);
    SetActive(active_now);
    SetUpTime(uptime_now);
    SetCpuUtilization(active_d / (float)uptime_d);
  }
}

void Process::UpdateRam() { SetRam(LinuxParser::Ram(Pid())); }

void Process::UpdateState() {
  string state = LinuxParser::State(Pid());
  if (state.empty()) {
    // process state was not read from file, so this is a good indication that
    // this proccess has been killed. Set killed_ to true and set state to "~"
    // to allow for sorting by state to remove killed processes
    SetKilled(true);
    state = "~";
  }
  SetState(state);
}

void Process::Update() {
  UpdateCpuUtilization();
  UpdateRam();
  UpdateState();
}

bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}

bool Process::operator==(unsigned int const& a) const { return Pid() == a; }
bool Process::operator==(Process const& a) const { return Pid() == a.Pid(); }