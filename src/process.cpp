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
int Process::Pid() { return pid_; }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() { return 0; }

// Return the command that generated this process
string Process::Command() { return command_; }

// Return this process's memory utilization
string Process::Ram() {
  string ram_mb;
  string ram_kb = LinuxParser::Ram(Pid());
  if (!ram_kb.empty()) {
    ram_mb = to_string(stol(ram_kb) / 1000);
  }
  return ram_mb;
}

// Return the user (name) that generated this process
string Process::User() { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a [[maybe_unused]]) const {
  return true;
}