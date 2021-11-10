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
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  long Active() const;
  long UpTime() const;
  std::string Ram() const;
  std::string State() const;
  bool isKilled() const;
  float CpuUtilization() const;
  bool operator<(Process const& a) const;
  bool operator==(int const& a) const;

 private:
  int pid_{0};
  std::string user_;
  std::string command_;
  mutable bool killed_{false};
  mutable long active_{0};
  mutable long uptime_{0};
  mutable float cpu_util_{0.0};
};

#endif