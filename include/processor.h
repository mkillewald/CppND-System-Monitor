#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  unsigned long total_{0};
  unsigned long idle_{0};
};

#endif