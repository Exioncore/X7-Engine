#include "ProcessMonitor.h"
// System Includes
#include <filesystem>
#include <format>
#include <iostream>
// 3rd Party Include
#include <Psapi.h>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
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
        SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL,
                        ProcessMonitor::WinEventProcCallback, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    LOG_EC(foreground_window_change_event_hook == NULL, "SetWinEventHook()");
  } else {
    foreground_window_change_event_hook = NULL;
  }

  if (IS_LOG_OK) {
    LOG_LR(loadProcAffinityMapFromDisk(),
           "Load Process Affinity Map from Disk");
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
      if (IS_LOG_OK) {
        LOG_INFO("Process " + process.getPath() + " affinity set to " +
                 std::to_string(affinity_mask));
      }
    }
    if (IS_LOG_OK && proc_affinity_map.count(process.getPath()) == 0) {
      proc_affinity_map.emplace(process.getPath(), affinity_mask);
      LOG_LR(saveProcAffinityMapToDisk(), "Save Process Affinity map to disk");
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

LOG_RETURN_TYPE ProcessMonitor::loadProcAffinityMapFromDisk() {
  using namespace rapidjson;

  LOG_BEGIN;

  // Load JSON file from disk
  std::string path = std::filesystem::current_path()
                         .parent_path()
                         .append("ProcessAffinityMap.json")
                         .string();
  FILE* fp = fopen(path.c_str(), "rb");
  if (fp != NULL) {
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document document;
    document.ParseStream(is);
    fclose(fp);

    // Read JSON file and populate proce_affinity_map
    for (auto& entry : document["Programs"].GetArray()) {
      std::string path = entry["path"].GetString();
      uint64_t affinity = entry["affinity"].GetUint64();
      proc_affinity_map.emplace(path, affinity);
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE ProcessMonitor::saveProcAffinityMapToDisk() {
  using namespace rapidjson;

  LOG_BEGIN;

  // Build JSON file
  Document document;
  document.SetObject();
  Value array(rapidjson::kArrayType);

  Document::AllocatorType& allocator = document.GetAllocator();
  for (auto& [path, affinity] : proc_affinity_map) {
    Value value(rapidjson::kObjectType);
    {
      {
        Value path_txt;
        path_txt.SetString(path.c_str(), allocator);
        value.AddMember("path", path_txt, allocator);
      }
      value.AddMember("affinity", affinity, allocator);
    }
    array.PushBack(value, allocator);
  }
  document.AddMember("Programs", array, allocator);

  // Write JSON file to disk
  std::string path = std::filesystem::current_path()
                         .parent_path()
                         .append("ProcessAffinityMap.json")
                         .string();
  FILE* fp = fopen(path.c_str(), "wb");
  char writeBuffer[65536];
  FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

  Writer<FileWriteStream> writer(os);
  document.Accept(writer);

  fclose(fp);

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
  if (dwEvent == EVENT_SYSTEM_FOREGROUND) {
    LOG_BEGIN;

    ProcessData proc;
    ProcessMonitor& proc_monitor = ProcessMonitor::getInstance();
    LOG_LR(proc_monitor.getProcessDataFromHandle(hwnd, proc),
           "Get process from handle");
    if (IS_LOG_OK) {
      // Check if we have an affinity mask expectation for the given process
      const std::string& proc_path = proc.getPath();
      if (proc_monitor.proc_affinity_map.count(proc_path) == 1) {
        uint64_t expected_affinity_mask =
            proc_monitor.proc_affinity_map.at(proc_path);
        uint64_t curr_affinity_mask;
        LOG_EC(proc.getAffinity(curr_affinity_mask), "Get Process Affinity");

        // Set affinity mask only if it doesn't already match our preset
        if (curr_affinity_mask != expected_affinity_mask) {
          LOG_EC(proc.setAffinity(expected_affinity_mask),
                 "Set Process Affinity");
          if (IS_LOG_OK) {
            LOG_INFO(
                "Process " + proc_path + " affinity set to " +
                std::to_string(proc_monitor.proc_affinity_map.at(proc_path)));
          }
        }
      }
    }

    LOG_END;
  }
}

}  // namespace PCC
