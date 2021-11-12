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
  std::string Command(int len = -1) const;
  unsigned long Active() const;
  unsigned long UpTime() const;
  float CpuUtilization() const;
  std::string Ram() const;
  std::string State() const;
  bool isKilled() const;
  void Update();
  bool operator<(Process const& a) const;
  bool operator==(unsigned int const& a) const;
  bool operator==(Process const& a) const;

 private:
  unsigned int pid_{0};
  std::string user_;
  std::string command_;
  unsigned long active_{0};
  unsigned long uptime_{0};
  float cpu_util_{0.0};
  std::string ram_;
  std::string state_;
  bool killed_{false};

  void SetActive(unsigned long active);
  void SetUpTime(unsigned long uptime);
  void SetCpuUtilization(float cpu_util);
  void SetRam(std::string ram);
  void SetState(std::string state);
  void SetKilled(bool k);
  void UpdateCpuUtilization();
  void UpdateRam();
  void UpdateState();
};

#endif