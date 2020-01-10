#include "process.h"

#include <linux_parser.h>
#include <unistd.h>

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "format.h"

using std::string;
using std::to_string;
using std::vector;

// DONE: Return this process's ID
int Process::Pid() const { return pid_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() {
  /**
   * Implementation is based on:
   * https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
   */
  long current_active = LinuxParser::ActiveJiffies(pid_);
  long current_total = LinuxParser::ActiveJiffies();

  float actived = current_active - active_jiffies_;
  float totald = current_total - total_jiffies_;

  active_jiffies_ = current_active;
  total_jiffies_ = current_total;

  if (totald <= 0) {
    return 0.0;
  }

  return actived / totald;
}

// DONE: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// DONE: Return this process's memory utilization
string Process::Ram() { return to_string(LinuxParser::Ram(pid_)); }

// DONE: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const &a) const {
  bool sort_by_ram{true};

  if (sort_by_ram) {
    return LinuxParser::Ram(pid_) > LinuxParser::Ram(a.Pid());
  }

  return LinuxParser::ActiveJiffies(pid_) > LinuxParser::ActiveJiffies(a.Pid());
}

long Process::TotalJiffies() { return total_jiffies_; }
long Process::ActiveJiffies() { return active_jiffies_; }
