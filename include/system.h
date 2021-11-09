#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

class System {
 public:
  enum Sort {
    kMinCpu_ = 0,
    kMaxCpu_,
    kMinRam_,
    kMaxRam_,
    kMinPid_,
    kMaxPid_,
    kMinState_,
    kMaxState_
  };

  Processor& Cpu();
  std::vector<Process>& Processes();
  float MemoryUtilization() const;
  long UpTime() const;
  int TotalProcesses() const;
  int RunningProcesses() const;
  std::string Kernel() const;
  std::string OperatingSystem() const;
  int GetSort() const;
  void SetSort(int s);

 private:
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};
  int sort_ = kMaxCpu_;

  void AddProcesses();
  void RemoveProcesses();
  void SortProcesses();
};

#endif