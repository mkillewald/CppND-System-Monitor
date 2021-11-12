#include "linux_parser.h"

#include <unistd.h>

#include <experimental/filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

namespace fs = std::experimental::filesystem;

/*
 * Returns a line from a file at given path whose first token matches key.
 * If key is not supplied, the first line of the file is returned.
 * An empty string is returned if key is supplied but not found, or if
 * the file was not opened.
 */
string LinuxParser::GetLineFromFile(const string& path,
                                    const string& key = "") {
  std::ifstream filestream(path);
  if (filestream.is_open()) {
    string line;
    if (key.empty()) {
      std::getline(filestream, line);
      return line;
    }
    while (std::getline(filestream, line)) {
      if (line.compare(0, key.length(), key) == 0) {
        return line;
      }
    }
  }
  return string();
}

/*
 * Tokenizes the input string and returns of vector of strings containing the
 * tokens.
 */
vector<string> LinuxParser::GetValuesFromLine(const string& line) {
  string value;
  vector<string> values;
  std::istringstream linestream(line);
  while (linestream >> value) {
    values.emplace_back(value);
  }
  return values;
}

/*
 * Returns the token at given index from an white space deliminated input
 * string. Will return the first token if no index is supplied. If the input
 * line is empty, or an out of range index was supplied, then will return an
 * empty string.
 */
string LinuxParser::GetValueFromLine(const string& line, const int index) {
  string value;
  std::istringstream linestream(line);
  int count = 0;
  while (linestream >> value) {
    if (index == count) {
      return value;
    }
    count++;
  }

  return string();
}

/*
 * Sometimes the filename aka "comm" field in /proc/pid/stat contains spaces
 * within the parenthesis. This will replace those spaces with an underscore to
 * allow for correct tokenization of the line
 */
void LinuxParser::FixTokenInParens(string& line) {
  size_t l_paren = line.find('(');
  if (l_paren == string::npos) {
    return;
  }
  size_t r_paren = line.find(')', l_paren + 1);
  std::replace(line.begin() + l_paren, line.begin() + r_paren, ' ', '_');
}

string LinuxParser::Kernel() {
  string line = GetLineFromFile(kProcDirectory + kVersionFilename);
  return GetValueFromLine(line, Version::kKernel_);
}

string LinuxParser::OperatingSystem() {
  string key, value, line;
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
  return string();
}

vector<unsigned int> LinuxParser::Pids() {
  vector<unsigned int> pids;
  const fs::path directory{kProcDirectory};
  for (auto& file : fs::directory_iterator(directory)) {
    // Is this a directory?
    if (fs::is_directory(file.status())) {
      // Is every character of the name a digit?
      string filename(file.path().filename());
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        unsigned int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string key, value, line;
  long total, available;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    for (int i = 0; i < 3; i++) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (i == 0) {
        total = stol(value);
      } else if (i == 2) {
        available = stol(value);
      }
    }
    if (total > 0) {
      return (float)(total - available) / (float)total;
    }
  }
  return 0.0;
}

unsigned long LinuxParser::UpTime() {
  string uptime;
  string line = GetLineFromFile(kProcDirectory + kUptimeFilename);
  std::istringstream linestream(line);
  linestream >> uptime;
  if (uptime.empty()) {
    return 0;
  }
  return stoul(uptime);
}

unsigned long LinuxParser::Jiffies(int index) {
  unsigned long total = 0;
  vector<string> values = CpuUtilization(index);
  if (values.size() > CPUStates::kSteal_) {
    for (int i = CPUStates::kUser_; i <= CPUStates::kSteal_; i++) {
      total += stoul(values.at(i));
    }
  }
  return total;
}

unsigned long LinuxParser::ActiveJiffies(int index) {
  unsigned long active = 0;
  vector<string> values = CpuUtilization(index);
  if (values.size() > CPUStates::kSteal_) {
    active = stoul(values.at(CPUStates::kUser_));
    active += stoul(values.at(CPUStates::kNice_));
    active += stoul(values.at(CPUStates::kSystem_));
    active += stoul(values.at(CPUStates::kIRQ_));
    active += stoul(values.at(CPUStates::kSoftIRQ_));
    active += stoul(values.at(CPUStates::kSteal_));
  }
  return active;
}

unsigned long LinuxParser::IdleJiffies(int index) {
  unsigned long idle = 0;
  vector<string> values = CpuUtilization(index);
  if (values.size() > CPUStates::kIOwait_) {
    idle = stoul(values.at(CPUStates::kIdle_));
    idle += stoul(values.at(CPUStates::kIOwait_));
  }
  return idle;
}

unsigned long LinuxParser::TotalProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kProcesses);
  string value = GetValueFromLine(line, 1);
  if (value.empty()) {
    return 0;
  }
  return stoul(value);
}

unsigned long LinuxParser::RunningProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kProcsRunning);
  string value = GetValueFromLine(line, 1);
  if (value.empty()) {
    return 0;
  }
  return stoul(value);
}

vector<string> LinuxParser::CpuUtilization(int index) {
  string cpu_key{kCpu};
  if (index > -1) {
    cpu_key += to_string(index);
  }
  string line = GetLineFromFile(kProcDirectory + kStatFilename, cpu_key);
  return GetValuesFromLine(line);
}

string LinuxParser::Command(unsigned int pid) {
  return GetLineFromFile(kProcDirectory + to_string(pid) + kCmdlineFilename);
}

string LinuxParser::Filename(unsigned int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  size_t l_paren = line.find('(');
  size_t r_paren = line.find(')', l_paren + 1);
  if (l_paren > 0 && r_paren > l_paren) {
    return line.substr(l_paren, r_paren - l_paren + 1);
  }
  return string();
}

int LinuxParser::GetTotalCpus() {
  int total{-1};
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    string line;
    while (std::getline(filestream, line)) {
      if (GetValueFromLine(line).substr(0, 3).compare(kCpu) == 0) {
        total++;
      }
    }
  }
  return total;
}

string LinuxParser::Ram(unsigned int pid) {
  // Using VmRSS here instead of VmSize because VmSize includes virtual memory
  // used by the process, and VmRSS gives exact physical memory being used.
  //
  // see https://man7.org/linux/man-pages/man5/proc.5.html for more info.
  string line = GetLineFromFile(
      kProcDirectory + to_string(pid) + kStatusFilename, kVmRSS);
  if (line.empty()) {
    return "0.00000";
  }
  return to_string(std::stof(GetValueFromLine(line, 1)) / 1000.0);
}

string LinuxParser::Uid(unsigned int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatusFilename, kUid);
  return GetValueFromLine(line, 1);
}

string LinuxParser::User(unsigned int pid) {
  string line;
  string uid = Uid(pid);
  if (uid.empty()) {
    return string();
  }
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      string value;
      vector<string> values;
      while (values.size() < User::kUid_ + 1 &&
             std::getline(linestream, value, ':')) {
        values.emplace_back(value);
      }
      if (values.size() > User::kUid_ &&
          values.at(User::kUid_).compare(uid) == 0) {
        return values.at(User::kUserName_);
      }
    }
  }
  return string();
}

unsigned long LinuxParser::UpTime(unsigned int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  FixTokenInParens(line);
  string start_time_str = GetValueFromLine(line, PidStat::kStartTime_);
  if (start_time_str.empty()) {
    return 0;
  }
  unsigned long start_time = stoul(start_time_str);
  return UpTime() - (start_time / sysconf(_SC_CLK_TCK));
}

unsigned long LinuxParser::ActiveJiffies(unsigned int pid) {
  unsigned long active = 0;
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  FixTokenInParens(line);
  vector<string> values = GetValuesFromLine(line);

  if (values.size() > PidStat::kStime_) {
    for (int i = PidStat::kUtime_; i <= PidStat::kStime_; i++) {
      active += stoul(values.at(i));
    }
  }
  return active;
}

string LinuxParser::State(unsigned int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  FixTokenInParens(line);
  return GetValueFromLine(line, PidStat::kState_);
}
