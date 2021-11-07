#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  std::string Ram() const;
  long UpTime() const;
  std::string State() const;
  bool operator<(Process const& a) const;
  bool operator==(Process const& a) const;

 private:
  int pid_;
  std::string user_;
  std::string command_;
  mutable long active_;
  mutable long uptime_;
  mutable float cpu_util_;
};

#endif