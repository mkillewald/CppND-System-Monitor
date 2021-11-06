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

Process::Process(int pid) : pid_(pid){};

// Return this process's ID
int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  long active_sec = LinuxParser::ActiveJiffies(Pid()) / sysconf(_SC_CLK_TCK);
  long process_uptime = Process::UpTime();
  if (process_uptime > 0) {
    return (float)active_sec / (float)process_uptime;
  }
  return 0.0;
}

// Return the command that generated this process
string Process::Command() const { return LinuxParser::Command(Pid()); }

// Return this process's memory utilization
string Process::Ram() const {
  string ram_kb = LinuxParser::Ram(Pid());
  if (ram_kb.empty()) {
    return "0";
  }
  return to_string(stof(ram_kb) / 1000.0);
}

// Return the user (name) that generated this process
string Process::User() const { return LinuxParser::User(Pid()); }

// Return the age of this process (in seconds)
long int Process::UpTime() const { return LinuxParser::UpTime(Pid()); }

// Return the state of this process
string Process::State() const { return LinuxParser::State(Pid()); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}