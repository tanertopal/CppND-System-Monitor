#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = std::stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string value;
  float memtotal{1};
  float memfree{0};
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> std::ws >> value) {
        if (key == "MemTotal:") {
          memtotal = std::stod(value);
        } else if (key == "MemFree:") {
          memfree = std::stod(value);
        }
      }
    }
  }
  return memfree / memtotal;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  string value;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> value;
    return std::stol(value);
  }
  return 0;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  auto vs = LinuxParser::CpuUtilization();
  int sum = 0;

  for (auto& j : vs) sum += j;

  return sum;
}

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  /**
   * Number of active jiffies for the process nessecary to calculate
   * CPU usage of the process.
   *
   * Will return a sum of the following values extracted from
   * /proc/[PID]/stat
   *
   * - #14 utime - CPU time spent in user code, measured in clock ticks
   * - #15 stime - CPU time spent in kernel code, measured in clock ticks
   * - #16 cutime - Waited-for children's CPU time spent in user code (in clock
   * ticks)
   * - #17 cstime - Waited-for children's CPU time spent in kernel code (in
   * clock ticks)
   *
   * Indexes can be found in http://man7.org/linux/man-pages/man5/proc.5.html
   *
   * @param pid ProcessID of the target process
   * @return number of active jiffies
   * {utime,stime,cutime,cstime,starttime} for the process
   */
  string line;
  string key;
  string value;
  long sum{0};
  bool include_children_cpu{false};
  int end_index = include_children_cpu ? 17 : 15;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    for (int i = 1; i < 18; i++) {
      linestream >> value;
      if ((i >= 14 && i <= end_index)) {
        sum += std::stoi(value);
      }
    }
  }

  return sum;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto vs = LinuxParser::CpuUtilization();
  return vs[kUser_] + vs[kNice_] + vs[kSystem_] + vs[kIRQ_] + vs[kSoftIRQ_] +
         vs[kSteal_];
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto vs = LinuxParser::CpuUtilization();
  return vs[kIdle_] + vs[kIOwait_];
}

// DONE: Read and return CPU utilization
vector<int> LinuxParser::CpuUtilization() {
  /*
  Meaning of cpu values in /proc/stat =>
  The very first  "cpu" line aggregates the  numbers in all  of the other
  "cpuN" lines.  These numbers identify the amount of time the CPU has spent
  performing different kinds of work.  Time units are in USER_HZ (typically
  hundredths of a second).  The meanings of the columns are as follows, from
  left to right:
  - user: normal processes executing in user mode
  - nice: niced processes executing in user mode
  - system: processes executing in kernel mode
  - idle: twiddling thumbs
  - iowait: In a word, iowait stands for waiting for I/O to complete. But there
            are several problems:
    1. Cpu will not wait for I/O to complete, iowait is the time that a task is
       waiting for I/O to complete. When cpu goes into idle state for
       outstanding task io, another task will be scheduled on this CPU.
    2. In a multi-core CPU, the task waiting for I/O to complete is not
       running on any CPU, so the iowait of each CPU is difficult to calculate.
    3. The value of iowait field in /proc/stat will decrease in certain
       conditions. So, the iowait is not reliable by reading from /proc/stat.
  - irq: servicing interrupts
  - softirq: servicing softirqs
  - steal: involuntary wait
  - guest: running a normal guest
  - guest_nice: running a niced guest
  */

  string line;
  string key;
  string value;
  vector<int> values;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> value;  // skip cpu
    while (linestream >> value) {
      values.push_back(std::stoi(value));
    }
  }

  return values;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::replace(line.begin(), line.end(), '\0', ' ');
    return line;
  }
  return string();
}

// DONE: Read and return the memory used by a process
int LinuxParser::Ram(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          return std::stoi(value) / 1024;
        }
      }
    }
  }
  return 0;
}
// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }

    return line;
  }

  return string();
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string line;

  // First three values in a line in /etc/passwd
  string username;
  string pass;
  string uid;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> pass >> uid) {
        if (uid == LinuxParser::Uid(pid)) {
          return username;
        }
      }
    }
  }
  return string();
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line;
  string value;
  int starttime{0};
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    for (int i = 1; i <= 22; i++) {
      linestream >> value;
    }

    starttime = std::stoi(value) / sysconf(_SC_CLK_TCK);
  }

  return starttime;
}
