#include "processor.h"

#include <iostream>

#include "linux_parser.h"
using namespace LinuxParser;

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  /*
  The implementation below is based on the following stackoverflow answer:
  https://stackoverflow.com/a/23376195/1087911
  */

  vector<int> vs = LinuxParser::CpuUtilization();

  // LinuxParser::CPUStates s;

  int current_idle = LinuxParser::IdleJiffies();
  int current_total = current_idle + LinuxParser::ActiveJiffies();

  // differentiate: actual value minus the previous one
  float totald = current_total - previous_total_;
  float idled = current_idle - previous_idle_;

  // Now set current
  previous_total_ = current_total;
  previous_idle_ = current_idle;

  if (totald == 0) {
    return 0;
  }

  return (totald - idled) / totald;
}
