#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>

// data indexes for /proc/version
#define KERNEL_INDEX 2

// data indexes for /proc/meminfo
#define MEM_TOTAL_INDEX 0
#define MEM_AVAIL_INDEX 2

// data indexes for CPU info lines in /proc/stat
#define CPU_USER_INDEX 1
#define CPU_NICE_INDEX 2
#define CPU_SYSTEM_INDEX 3
#define CPU_IDLE_INDEX 4
#define CPU_IOWAIT_INDEX 5
#define CPU_IRQ_INDEX 6
#define CPU_SOFTIRQ_INDEX 7
#define CPU_STEAL_INDEX 8

// data indexes for /proc/pid/stat
#define PID_UTIME_INDEX 13
#define PID_STIME_INDEX 14
#define PID_CUTIME_INDEX 15
#define PID_CSTIME_INDEX 16
#define PID_STARTTIME_INDEX 21

// data indexes for /etc/password
#define UID_INDEX 2

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
const std::string kVmSize{"VmSize:"};
const std::string kUid{"Uid:"};

std::string GetLineFromFile(const std::string&, const std::string&);
std::vector<std::string> GetValuesFromLine(const std::string&);
std::string GetValueFromLine(const std::string&);

// System
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

// CPU
enum CPUStates {
  kUser_ = 0,
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
std::vector<std::string> CpuUtilization();
long Jiffies();
long ActiveJiffies();
long ActiveJiffies(int pid);
long IdleJiffies();

// Processes
std::string Command(int pid);
std::string Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);
};  // namespace LinuxParser

#endif