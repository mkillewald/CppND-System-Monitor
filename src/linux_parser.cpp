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
                                    const string& key_in = "") {
  string line;
  std::ifstream filestream(path);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      if (key_in.empty()) {
        return line;
      } else if (line.compare(0, key_in.length(), key_in) == 0) {
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
  vector<string> values =
      LinuxParser::GetValuesFromLine(kProcDirectory + kVersionFilename);
  return values.at(KERNEL_INDEX);
}

// BONUS: Update this to use std::filesystem
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
    if (values.size() > MEM_AVAIL_INDEX &&
        !values.at(MEM_TOTAL_INDEX).empty() &&
        !values.at(MEM_AVAIL_INDEX).empty()) {
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

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, "cpu");
  return GetValuesFromLine(line);
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, "processes");
  string value = GetValueFromLine(line);
  if (!value.empty()) {
    return stoi(value);
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line =
      GetLineFromFile(kProcDirectory + kStatFilename, "procs_running");
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

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid [[maybe_unused]]) { return string(); }

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid [[maybe_unused]]) { return 0; }
