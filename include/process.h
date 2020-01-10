#ifndef PROCESS_H
#define PROCESS_H

#include <string>
using std::string;

/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid, long total_jiffies, long active_jiffies)
      : pid_(pid),
        total_jiffies_(total_jiffies),
        active_jiffies_(active_jiffies) {}

  int Pid() const;                         // DONE: See src/process.cpp
  std::string User();                      // DONE: See src/process.cpp
  std::string Command();                   // DONE: See src/process.cpp
  float CpuUtilization();                  // DONE: See src/process.cpp
  std::string Ram();                       // DONE: See src/process.cpp
  long UpTime();                       // DONE: See src/process.cpp
  bool operator<(Process const& a) const;  // DONE: See src/process.cpp

  long TotalJiffies();
  long ActiveJiffies();

  // DONE: Declare any necessary private members
 private:
  int pid_;
  long total_jiffies_{0};   // total in system
  long active_jiffies_{0};  // jiffies of process
};

#endif
