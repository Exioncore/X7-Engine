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
class ProcessMonitor {
 public:
  ProcessMonitor(const ProcessMonitor&) = delete;
  ProcessMonitor& operator=(const ProcessMonitor&) = delete;

  // Methods
  LOG_RETURN_TYPE initialize();
  LOG_RETURN_TYPE deInitialize();
  LOG_RETURN_TYPE setForegroundProcessAffinity(uint64_t affinity_mask);

  static ProcessMonitor& getInstance();

 private:
  LOGGER("ProcessMonitor", true, true);

  bool initialized;
  std::unordered_map<uint32_t, std::string> live_processes;
  uint16_t new_process_begin_callback_id, new_process_end_callback_id;

  std::mutex mtx;
  std::unordered_map<std::string, uint64_t> proc_affinity_map;

  ProcessMonitor();
  ~ProcessMonitor();

  // Methods
  LOG_RETURN_TYPE getAllRunningProcesses();
  LOG_RETURN_TYPE setProcessAffinity(uint32_t pid, uint64_t affinity_mask);

  LOG_RETURN_TYPE loadProcAffinityMapFromDisk();
  LOG_RETURN_TYPE saveProcAffinityMapToDisk();

  // Others
  void newProcessBegin(IWbemClassObject* data);
  void newProcessEnd(IWbemClassObject* data);
};

}  // namespace PCC