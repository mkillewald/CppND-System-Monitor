#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

// Returns line from file at given path whose first token matches key_in.
// If key_in is not supplied, the first line of the file is returned.
string LinuxParser::GetLineFromFile(const string& path,
                                    const string& key = "") {
  string line;
  std::ifstream filestream(path);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (key.empty()) {
        return line;
      } else if (line.compare(0, key.length(), key) == 0) {
        return line;
      }
    }
  }
  return line;
}

// Tokenizes the input string and returns of vector of strings containing the
// tokens.
vector<string> LinuxParser::GetValuesFromLine(const string& line) {
  string value;
  vector<string> values;
  if (!line.empty()) {
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.emplace_back(value);
    }
  }
  return values;
}

// Returns a string value from an input string containing a key value pair
string LinuxParser::GetValueFromLine(const string& line) {
  string key, value;
  if (!line.empty()) {
    std::istringstream linestream(line);
    linestream >> key >> value;
  }
  return value;
}

// Read and return the operating system name
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// Read and return the system's kernel identifier (string)
string LinuxParser::Kernel() {
  string line = GetLineFromFile(kProcDirectory + kVersionFilename);
  vector<string> values = GetValuesFromLine(line);
  return values.at(KERNEL_INDEX);
}

// TODO: BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string key, value, line;
  vector<string> values;
  long total, available;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    for (int i = 0; i < 3; i++) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> key >> value;
      values.push_back(value);
    }
    if (values.size() > MEM_AVAIL_INDEX) {
      total = stol(values[MEM_TOTAL_INDEX]);
      available = stol(values[MEM_AVAIL_INDEX]);
      return (float)(total - available) / (float)total;
    }
  }
  return 0.0;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime;
  string line = GetLineFromFile(kProcDirectory + kUptimeFilename);
  if (!line.empty()) {
    std::istringstream linestream(line);
    linestream >> uptime;
    return std::stol(uptime);
  }
  return 0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long total = 0;
  vector<string> values = CpuUtilization();
  if (values.size() > CPU_STEAL_INDEX) {
    for (int i = CPU_USER_INDEX; i <= CPU_STEAL_INDEX; i++) {
      total += stol(values.at(i));
    }
  }
  return total;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  long active = 0;
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  vector<string> values = GetValuesFromLine(line);
  if (values.size() > PID_CSTIME_INDEX) {
    for (int i = PID_UTIME_INDEX; i <= PID_CSTIME_INDEX; i++) {
      active += stol(values.at(i));
    }
  }
  return active;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  long active = 0;
  vector<string> values = CpuUtilization();
  if (values.size() > CPU_STEAL_INDEX) {
    active = stol(values.at(CPU_USER_INDEX));
    active += stol(values.at(CPU_NICE_INDEX));
    active += stol(values.at(CPU_SYSTEM_INDEX));
    active += stol(values.at(CPU_IRQ_INDEX));
    active += stol(values.at(CPU_SOFTIRQ_INDEX));
    active += stol(values.at(CPU_STEAL_INDEX));
  }
  return active;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long idle = 0;
  vector<string> values = CpuUtilization();
  if (values.size() > CPU_IOWAIT_INDEX) {
    idle = stol(values.at(CPU_IDLE_INDEX));
    idle += stol(values.at(CPU_IOWAIT_INDEX));
  }
  return idle;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kCpu);
  return GetValuesFromLine(line);
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kProcesses);
  string value = GetValueFromLine(line);
  if (!value.empty()) {
    return stoi(value);
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kProcsRunning);
  string value = GetValueFromLine(line);
  if (!value.empty()) {
    return stoi(value);
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  return GetLineFromFile(kProcDirectory + to_string(pid) + kCmdlineFilename);
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line = GetLineFromFile(
      kProcDirectory + to_string(pid) + kStatusFilename, kVmSize);
  return GetValueFromLine(line);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatusFilename, kUid);
  return GetValueFromLine(line);
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;
  string uid = Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      int count = 0;
      size_t pos = 0;
      while (count < UID_INDEX) {
        pos = line.find(":", pos);
        if (pos == string::npos) {
          return string();
        }
        pos++;
        count++;
      }
      if (line.substr(pos, line.find(":", pos) - pos).compare(uid) == 0) {
        return line.substr(0, line.find(":"));
      }
    }
  }
  return string();
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  vector<string> values = GetValuesFromLine(line);
  if (values.size() > PID_STARTTIME_INDEX) {
    long start_time = stol(values.at(PID_STARTTIME_INDEX));
    return UpTime() - (start_time / sysconf(_SC_CLK_TCK));
  }

  return 0;
}
