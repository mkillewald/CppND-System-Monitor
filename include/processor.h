#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  Processor();
  Processor(int id);
  int Id() const;
  unsigned long Jiffies() const;
  unsigned long IdleJiffies() const;
  float Utilization() const;
  void Update();

 private:
  int id_;
  unsigned long total_;
  unsigned long idle_;
  float cpu_util_;

  void SetJiffies(unsigned long total);
  void SetIdleJiffies(unsigned long idle);
  void SetCpuUtilization(float cpu_util);
};

#endif