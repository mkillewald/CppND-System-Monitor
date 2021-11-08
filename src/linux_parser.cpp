#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

/*
 * Returns a line from a file at given path whose first token matches key.
 * If key is not supplied, the first line of the file is returned.
 * An empty string is returned if key is supplied but not found, or if
 * the file was not opened.
 */
string LinuxParser::GetLineFromFile(const string& path,
                                    const string& key = "") {
  string line;
  std::ifstream filestream(path);
  if (filestream.is_open()) {
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
    values.push_back(value);
  }
  return values;
}

/*
 * Returns the token at given index from an white space deliminated input
 * string. Will return the first token if no index is supplied. If the input
 * line is empty, or an out of range index was supplied, then will return an
 * empty string.
 */
string LinuxParser::GetValueFromLine(const string& line, const int index = 0) {
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
 * Sometimes the filename aka "comm" field in /proc/pid/stat contains spaces.
 * This will replace those spaces with an underscore to allow for correct
 * tokenization of the line
 */
void LinuxParser::FixFilenameInParens(string& line) {
  size_t l_paren = line.find('(');
  if (l_paren == string::npos) {
    return;
  }
  size_t r_paren = line.find(')', l_paren + 1);
  std::replace(line.begin() + l_paren, line.begin() + r_paren, ' ', '_');
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
  return value;
}

string LinuxParser::Kernel() {
  string line = GetLineFromFile(kProcDirectory + kVersionFilename);
  return GetValueFromLine(line, Version::kKernel_);
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
    if (values.size() > Meminfo::kAvail_) {
      total = stol(values.at(Meminfo::kTotal_));
      available = stol(values.at(Meminfo::kAvail_));
      return (float)(total - available) / (float)total;
    }
  }
  return 0.0;
}

long LinuxParser::UpTime() {
  string uptime;
  string line = GetLineFromFile(kProcDirectory + kUptimeFilename);
  std::istringstream linestream(line);
  linestream >> uptime;
  if (uptime.empty()) {
    return 0;
  }
  return stol(uptime);
}

long LinuxParser::Jiffies() {
  long total = 0;
  vector<string> values = CpuUtilization();
  if (values.size() > CPUStates::kSteal_) {
    for (int i = CPUStates::kUser_; i <= CPUStates::kSteal_; i++) {
      total += stol(values.at(i));
    }
  }
  return total;
}

long LinuxParser::ActiveJiffies() {
  long active = 0;
  vector<string> values = CpuUtilization();
  if (values.size() > CPUStates::kSteal_) {
    active = stol(values.at(CPUStates::kUser_));
    active += stol(values.at(CPUStates::kNice_));
    active += stol(values.at(CPUStates::kSystem_));
    active += stol(values.at(CPUStates::kIRQ_));
    active += stol(values.at(CPUStates::kSoftIRQ_));
    active += stol(values.at(CPUStates::kSteal_));
  }
  return active;
}

long LinuxParser::IdleJiffies() {
  long idle = 0;
  vector<string> values = CpuUtilization();
  if (values.size() > CPUStates::kIOwait_) {
    idle = stol(values.at(CPUStates::kIdle_));
    idle += stol(values.at(CPUStates::kIOwait_));
  }
  return idle;
}

vector<string> LinuxParser::CpuUtilization() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kCpu);
  return GetValuesFromLine(line);
}

int LinuxParser::TotalProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kProcesses);
  string value = GetValueFromLine(line, 1);
  if (value.empty()) {
    return 0;
  }
  return stoi(value);
}

int LinuxParser::RunningProcesses() {
  string line = GetLineFromFile(kProcDirectory + kStatFilename, kProcsRunning);
  string value = GetValueFromLine(line, 1);
  if (value.empty()) {
    return 0;
  }
  return stoi(value);
}

string LinuxParser::Command(int pid) {
  return GetLineFromFile(kProcDirectory + to_string(pid) + kCmdlineFilename);
}

string LinuxParser::Ram(int pid) {
  string line = GetLineFromFile(
      kProcDirectory + to_string(pid) + kStatusFilename, kVmSize);
  return GetValueFromLine(line, 1);
}

string LinuxParser::Uid(int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatusFilename, kUid);
  return GetValueFromLine(line, 1);
}

string LinuxParser::User(int pid) {
  string line;
  string uid = Uid(pid);
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      string value;
      vector<string> values;
      while (values.size() < User::kUid_ + 1 &&
             std::getline(linestream, value, ':')) {
        values.push_back(value);
      }
      if (values.size() > User::kUid_ &&
          values.at(User::kUid_).compare(uid) == 0) {
        return values.at(User::kUserName_);
      }
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  FixFilenameInParens(line);
  string start_time_str = GetValueFromLine(line, PidStat::kStartTime_);
  if (start_time_str.empty()) {
    return 0;
  }
  long start_time = stol(start_time_str);
  return UpTime() - (start_time / sysconf(_SC_CLK_TCK));
}

long LinuxParser::ActiveJiffies(int pid) {
  long active = 0;
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  FixFilenameInParens(line);
  vector<string> values = GetValuesFromLine(line);

  if (values.size() > PidStat::kStime_) {
    for (int i = PidStat::kUtime_; i <= PidStat::kStime_; i++) {
      active += stol(values.at(i));
    }
  }
  return active;
}

string LinuxParser::State(int pid) {
  string line =
      GetLineFromFile(kProcDirectory + to_string(pid) + kStatFilename);
  FixFilenameInParens(line);
  return GetValueFromLine(line, PidStat::kState_);
}
