#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(unsigned int pid, std::string user, std::string command);
  unsigned int Pid() const;
  std::string User() const;
  std::string Command() const;
  unsigned long Active() const;
  unsigned long UpTime() const;
  std::string Ram() const;
  std::string State() const;
  bool isKilled() const;
  float CpuUtilization() const;
  bool operator<(Process const& a) const;
  bool operator==(unsigned int const& a) const;
  bool operator==(Process const& a) const;

 private:
  unsigned int pid_{0};
  std::string user_;
  std::string command_;
  mutable bool killed_{false};
  mutable unsigned long active_{0};
  mutable unsigned long uptime_{0};
  mutable float cpu_util_{0.0};
};

#endif