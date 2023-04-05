#include "ProcessMonitor.h"
// System Includes
#include <format>
#include <iostream>
// 3rd Party Include
#include <Psapi.h>
// Interal Module Includes
// External Module Includes

namespace PCC {
//////////////////////////
// ProcessMonitor Class //
//////////////////////////
ProcessMonitor::ProcessMonitor() {
  LOG_BEGIN;

  LOG_EC(CoInitialize(NULL), "CoInitialize()");
  if (IS_LOG_OK) {
    foreground_window_change_event_hook =
        SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL,
                        ProcessMonitor::WinEventProcCallback, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    LOG_EC(foreground_window_change_event_hook == NULL, "SetWinEventHook()");
  } else {
    foreground_window_change_event_hook = NULL;
  }

  LOG_END;
}

ProcessMonitor::~ProcessMonitor() {
  LOG_BEGIN;

  LOG_EC(UnhookWinEvent(foreground_window_change_event_hook) == TRUE,
         "UnhookWinEvent()");
  CoUninitialize();

  LOG_END;
}

///////////////
//// Methods //
///////////////
ProcessMonitor& ProcessMonitor::getInstance() {
  static ProcessMonitor instance;
  return instance;
}

LOG_RETURN_TYPE ProcessMonitor::setForegroundProcessAffinity(
    uint64_t affinity_mask) {
  LOG_BEGIN;

  HWND handle = GetForegroundWindow();
  LOG_EC(handle == NULL, "GetForegroundWindow()");
  if (IS_LOG_OK) {
    ProcessData process;
    LOG_LR(getProcessDataFromHandle(handle, process),
           "getProcessDataFromHandle()");
    if (IS_LOG_OK) {
      LOG_EC(process.setAffinity(affinity_mask), "setAffinity()");
    }
    if (IS_LOG_OK && proc_affinity_map.count(process.getPath()) == 0) {
      proc_affinity_map.emplace(process.getPath(), affinity_mask);
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE ProcessMonitor::getProcessDataFromHandle(
    HWND handle, ProcessData& out_process_data) {
  LOG_BEGIN;

  // Get Process ID
  DWORD dwProcessID;
  LOG_EC(GetWindowThreadProcessId(handle, &dwProcessID) == 0,
         "GetWindowThreadProcessId()");

  if (IS_LOG_OK) {
    HANDLE hProc = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION,
        FALSE, dwProcessID);
    LOG_EC(hProc == NULL, "OpenProcess()");
    if (IS_LOG_OK) {
      TCHAR filename[MAX_PATH] = {0};
      LOG_EC(GetModuleFileNameEx(hProc, NULL, filename, MAX_PATH) == 0,
             "GetModuleFileNameEx()");
      if (IS_LOG_OK) {
        out_process_data.set(hProc, std::string(filename));
      }
    }
  }

  return LOG_END;
}

///////////////
//// Others  //
///////////////
VOID CALLBACK ProcessMonitor::WinEventProcCallback(HWINEVENTHOOK hWinEventHook,
                                                   DWORD dwEvent, HWND hwnd,
                                                   LONG idObject, LONG idChild,
                                                   DWORD dwEventThread,
                                                   DWORD dwmsEventTime) {
  if (dwEvent == EVENT_OBJECT_CREATE) {
    ProcessData proc;
    (void)ProcessMonitor::getInstance().getProcessDataFromHandle(hwnd, proc);
    LOG_INFO("Process " + proc.getPath());
  }
}

}  // namespace PCC
