#pragma once
// System Includes
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
#include "ProcessData.h"
// External Module
#include "EasyLogger.h"
#include "WMIwatcher.h"

namespace PCC {
class ProcessMonitor {
 public:
  ProcessMonitor(const ProcessMonitor&) = delete;
  ProcessMonitor& operator=(const ProcessMonitor&) = delete;

  // Methods
  static ProcessMonitor& getInstance();
  LOG_RETURN_TYPE setForegroundProcessAffinity(uint64_t affinity_mask);

 private:
  LOGGER("ProcessMonitor", true, true);

  ProcessData foreground_process;
  HWINEVENTHOOK foreground_window_change_event_hook;

  std::unordered_map<uint32_t, std::string> live_processes;
  uint16_t new_process_begin_callback_id, new_process_end_callback_id;

  std::mutex mtx;
  std::unordered_map<std::string, uint64_t> proc_affinity_map;

  ProcessMonitor();
  ~ProcessMonitor();

  // Methods
  LOG_RETURN_TYPE getProcessDataFromHandle(HWND handle,
                                           ProcessData& out_process_data);

  LOG_RETURN_TYPE loadProcAffinityMapFromDisk();
  LOG_RETURN_TYPE saveProcAffinityMapToDisk();

  // Others
  void newProcessBegin(IWbemClassObject* data);
  void newProcessEnd(IWbemClassObject* data);

  static VOID CALLBACK WinEventProcCallback(HWINEVENTHOOK hWinEventHook,
                                            DWORD dwEvent, HWND hwnd,
                                            LONG idObject, LONG idChild,
                                            DWORD dwEventThread,
                                            DWORD dwmsEventTime);
};

}  // namespace PCC