#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  long user_;
  long nice_;
  long system_;
  long idle_;
  long iowait_;
  long irq_;
  long softirq_;
  long steal_;
  long guest_;
  long guest_nice_;
};

#endif