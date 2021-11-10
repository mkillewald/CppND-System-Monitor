#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

class System {
 public:
  enum Sort_t { kPid_ = 0, kUser_, kState_, kCpu_, kRam_, kUpTime_, kCommand_ };

  std::vector<Process>& Processes();
  Processor& Cpu();
  std::string Kernel() const;
  std::string OperatingSystem() const;
  int RunningProcesses() const;
  int TotalProcesses() const;
  long UpTime() const;
  float MemoryUtilization() const;
  Sort_t Sort() const;
  void SetSort(Sort_t s);
  bool Descending() const;
  void SetDescending(bool d);

 private:
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};
  Sort_t sort_ = kCpu_;
  bool descending_ = true;

  void AddProcesses();
  void RemoveProcesses();
  void SortProcesses();
};

#endif