#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid, string user, string command)
    : pid_(pid), user_(user), command_(command){};

// Return this process's ID
int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  long active_sec = LinuxParser::ActiveJiffies(Pid()) / sysconf(_SC_CLK_TCK);
  return 100.0 * (active_sec / Process::UpTime());
}

// Return the command that generated this process
string Process::Command() const { return command_; }

// Return this process's memory utilization
string Process::Ram() const {
  string ram_kb = LinuxParser::Ram(Pid());
  if (ram_kb.empty()) {
    return string();
  }
  std::ostringstream o_stream;
  o_stream.precision(5);
  o_stream << stof(ram_kb) / 1000.0;
  return o_stream.str();
}

// Return the user (name) that generated this process
string Process::User() const { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() const { return LinuxParser::UpTime(Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}