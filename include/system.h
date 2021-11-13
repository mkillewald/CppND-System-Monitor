#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

class System {
 public:
  enum Sort_t { kPid_ = 0, kUser_, kState_, kCpu_, kRam_, kUpTime_, kCommand_ };

  System();
  std::vector<Process>& Processes();
  int TotalCpus() const;
  Processor& Cpu();
  std::vector<Processor>& Cpus();
  std::string Kernel() const;
  std::string OperatingSystem() const;
  unsigned long RunningProcesses() const;
  unsigned long TotalProcesses() const;
  unsigned long UpTime() const;
  float MemoryUtilization() const;
  Sort_t Sort() const;
  void SetSort(Sort_t s);
  bool Descending() const;
  void SetDescending(bool d);
  void UpdateProcessors();
  void UpdateProcesses();

 private:
  int total_cpus_;
  Processor aggregate_cpu_;
  std::vector<Processor> cpus_;
  std::vector<Process> processes_;
  std::string kernel_;
  std::string os_;
  Sort_t sort_ = kCpu_;
  bool descending_ = true;

  void AddProcesses();
  void RemoveProcesses();
  void SortProcesses();
};

#endif