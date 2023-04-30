#pragma once
// System Includes
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
// External Module
#include "EasyLogger.h"
#include "WMIwatcher.h"

namespace PCC {
struct ProcessAffinity {
  bool modified;
  uint64_t mask;
};

enum ProcessPriority {
  UNMODIFIED = 0,
  BELOW_NORMAL = BELOW_NORMAL_PRIORITY_CLASS,
  NORMAL = NORMAL_PRIORITY_CLASS,
  ABOVE_NORMAL = ABOVE_NORMAL_PRIORITY_CLASS,
  HIGH = HIGH_PRIORITY_CLASS,
  REALTIME = REALTIME_PRIORITY_CLASS
};

class ProcessMonitor {
 private:
  struct ProcessStorageEntry {
    ProcessAffinity affinity;
    ProcessPriority priority;
  };

 public:
  ProcessMonitor(const ProcessMonitor&) = delete;
  ProcessMonitor& operator=(const ProcessMonitor&) = delete;

  // Methods
  LOG_RETURN_TYPE initialize();
  LOG_RETURN_TYPE deInitialize();
  LOG_RETURN_TYPE setForegroundProcessModifiers(ProcessAffinity affinity,
                                                ProcessPriority priority);

  static ProcessMonitor& getInstance();

 private:
  LOGGER("ProcessMonitor");

  bool initialized;
  std::unordered_map<uint32_t, std::string> live_processes;
  uint16_t new_process_begin_callback_id, new_process_end_callback_id;

  std::mutex mtx;
  std::unordered_map<std::string, ProcessStorageEntry> profiles;

  ProcessMonitor();
  ~ProcessMonitor();

  // Methods
  LOG_RETURN_TYPE getAllRunningProcesses();
  LOG_RETURN_TYPE setProcessModifiers(uint32_t pid, ProcessAffinity affinity,
                                      ProcessPriority priority);

  LOG_RETURN_TYPE loadProcAffinityMapFromDisk();
  LOG_RETURN_TYPE saveProcAffinityMapToDisk();

  // Others
  void newProcessBegin(IWbemClassObject* data);
  void newProcessEnd(IWbemClassObject* data);
};

}  // namespace PCC