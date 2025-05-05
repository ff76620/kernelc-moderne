#ifndef PROCESS_MANAGER_HPP
#define PROCESS_MANAGER_HPP

#include 
#include 
#include 
#include 
#include 

namespace Kernel {
namespace Process {

enum class ProcessState {
  NEW,
  RUNNING,
  WAITING,
  SUSPENDED,
  TERMINATED
};

enum class Priority {
  LOW,
  NORMAL,
  HIGH,
  REALTIME
};

enum class SchedulerPolicy {
  DEFAULT,
  MULTITHREADED,
  LOW_LATENCY,
  GAMING
};

enum class TimeSlice {
  DEFAULT,
  GAMING,
  LOW_LATENCY
};

enum class MemoryPolicy {
  DEFAULT,
  GAMING,
  LOW_LATENCY
};

class ProcessManager {
public:
  static ProcessManager& getInstance() {
    static ProcessManager instance;
    return instance;
  }

  ProcessID createProcess(const std::string& name, uint32_t priority = 1) {
    std::lock_guard lock(processMutex);
    Process newProcess(nextPID++, name, priority);
    setupProcessEnvironment(&newProcess);
    processes[newProcess.getPID()] = newProcess;
    scheduler.addProcess(newProcess);
    return newProcess.getPID();
  }

  bool terminateProcess(ProcessID pid) {
    std::lock_guard lock(processMutex);
    auto it = processes.find(pid);
    if (it != processes.end()) {
      cleanupProcessResources(&it->second);
      scheduler.removeProcess(pid);
      processes.erase(it);
      return true;
    }
    return false;
  }

  void optimizeForGaming() {
    scheduler.setPolicy(SchedulerPolicy::GAMING);
    scheduler.setTimeSlice(TimeSlice::GAMING);
    enableGamingOptimizations();
    threadPool.setPriority(Priority::HIGH);
    configureLowLatency();
  }

  void enableMultiThreading(bool enabled) {
    useMultiThreading = enabled;
    if (enabled) {
      threadPool.resize(std::thread::hardware_concurrency());
      scheduler.setPolicy(SchedulerPolicy::MULTITHREADED);
    }
  }

  void schedule() {
    scheduler.schedule([this](Process& process) {
      if (useMultiThreading) {
        threadPool.execute([&]() {
          executeProcess(process);
        });
      } else {
        executeProcess(process);
      }
    });
  }

private:
  ProcessManager() : nextPID(0), useMultiThreading(true) {
    initialize();
  }

  void initialize() {
    scheduler.initialize();
    vmm.initialize();
    threadPool.initialize();
  }

  void executeProcess(Process& process) {
    process.setState(ProcessState::RUNNING);
    // Process execution logic
    contextSwitch(process);
  }

  void contextSwitch(Process& process) {
    scheduler.saveContext(process);
    scheduler.loadContext(process);
  }

  void enableGamingOptimizations() {
    setMemoryPolicy(MemoryPolicy::GAMING);
    enableLargePages(true);
    configureIOPriority(Priority::HIGH);
    enableAsyncLoading(true);
  }

  std::mutex processMutex;
  std::map processes;
  ProcessID nextPID;
  Scheduler scheduler;
  VirtualMemoryManager vmm;
  ThreadPool threadPool;
  bool useMultiThreading;

  ProcessManager(const ProcessManager&) = delete;
  ProcessManager& operator=(const ProcessManager&) = delete;
};

} // namespace Process
} // namespace Kernel

#endif