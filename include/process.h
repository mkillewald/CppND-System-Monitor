#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid, std::string user, std::string command);
  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();  // TODO: See src/process.cpp
  std::string Ram();
  long int UpTime();
  bool operator<(Process const& a) const;  // TODO: See src/process.cpp

 private:
  const int pid_;
  const std::string user_;
  const std::string command_;
  long prev_cpu_total_;
  long prev_cpu_idle_;
};

#endif