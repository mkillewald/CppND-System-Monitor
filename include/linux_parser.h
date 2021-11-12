#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <string>
#include <vector>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// Keys
const std::string kCpu{"cpu"};
const std::string kProcesses{"processes"};
const std::string kProcsRunning{"procs_running"};
const std::string kVmRSS{"VmRSS:"};
const std::string kUid{"Uid:"};

// /etc/passwd
enum User { kUserName_ = 0, kPasswd_, kUid_, kGid_, kGecos_, kHome_, kShell_ };

// /proc/version
enum Version {
  kOSType_ = 0,
  kVersionKey_,
  kKernel_,
  kCompiledBy_,
  kGCCVersion_,
  kType_,
  kDate_
};

// /proc/meminfo
enum Meminfo { kTotal_ = 0, kFree_, kAvail_, kBuffers_ };

// /proc/stat CPU info
enum CPUStates {
  kCpuKey_ = 0,
  kUser_,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};

// /proc/[pid]/stat
enum PidStat {
  kPid_ = 0,
  kComm_,
  kState_,
  kPPid_,
  kPGrp_,
  kSession_,
  kTTY_,
  kTPGid_,
  kFlags_,
  kMinFlt_,
  kCMinFlt_,
  kMajFlt_,
  kCMajFlt_,
  kUtime_,
  kStime_,
  kCUtime_,
  kCStime_,
  kPriority_,
  kNiceValue_,
  kThreads_,
  kITReal_,
  kStartTime_,
};

// Helpers
std::string GetLineFromFile(const std::string& path, const std::string& key);
std::vector<std::string> GetValuesFromLine(const std::string& line);
std::string GetValueFromLine(const std::string& line, const int index = 0);
void FixTokenInParens(std::string& line);

// System
std::string Kernel();
std::string OperatingSystem();
std::vector<unsigned int> Pids();
float MemoryUtilization();
unsigned long UpTime();
unsigned long Jiffies(int index = -1);
unsigned long ActiveJiffies(int index = -1);
unsigned long IdleJiffies(int index = -1);
unsigned long TotalProcesses();
unsigned long RunningProcesses();
std::vector<std::string> CpuUtilization(int index = -1);
int GetTotalCpus();

// Processes
std::string Command(unsigned int pid);
std::string Filename(unsigned int pid);
std::string Ram(unsigned int pid);
std::string Uid(unsigned int pid);
std::string User(unsigned int pid);
unsigned long UpTime(unsigned int pid);
unsigned long ActiveJiffies(unsigned int pid);
std::string State(unsigned int pid);
};  // namespace LinuxParser

#endif