#include "ProcessMonitor.h"
// System Includes
#include <codecvt>
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
ProcessMonitor::ProcessMonitor() : initialized(false) {}

ProcessMonitor::~ProcessMonitor() { (void)deInitialize(); }

///////////////
//// Methods //
///////////////
ProcessMonitor& ProcessMonitor::getInstance() {
  static ProcessMonitor instance;
  return instance;
}

LOG_RETURN_TYPE ProcessMonitor::initialize() {
  LOG_BEGIN;

  // Register WMI callbacks
  if (!initialized) {
    auto& wmi_watcher = WMW::WMIwatcher::getInstance();
    LOG_LR(wmi_watcher.initialize(), "WMI Initialization");
    if (IS_LOG_OK) {
      LOG_LR(wmi_watcher.registerCallback(
                 new_process_begin_callback_id,
                 "SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE "
                 "TargetInstance ISA 'Win32_Process'",
                 std::bind(&ProcessMonitor::newProcessBegin, this,
                           std::placeholders::_1)),
             "Register New Process callback");
    }
    if (IS_LOG_OK) {
      LOG_LR(wmi_watcher.registerCallback(
                 new_process_end_callback_id,
                 "SELECT * FROM __InstanceDeletionEvent WITHIN 1 WHERE "
                 "TargetInstance ISA 'Win32_Process'",
                 std::bind(&ProcessMonitor::newProcessEnd, this,
                           std::placeholders::_1)),
             "Register End Process callback");
    }
    // Retrieve all currently running processes
    if (IS_LOG_OK) {
      LOG_LR(getAllRunningProcesses(), "Get all running processes");
    }
    // Load process affinity map from storage
    if (IS_LOG_OK) {
      LOG_LR(loadProcAffinityMapFromDisk(),
             "Load Process Affinity Map from Disk");
    }
    // Check if we already have any running processes who have a profile
    // associated
    for (auto& [pid, name] : live_processes) {
      if (proc_affinity_map.count(name) == 1) {
        (void)setProcessAffinity(pid, proc_affinity_map.at(name));
      }
    }

    initialized = IS_LOG_OK;
  }

  return LOG_END;
}

LOG_RETURN_TYPE ProcessMonitor::deInitialize() {
  LOG_BEGIN;
  // Unhook wmi callbacks
  auto& wmi_watcher = WMW::WMIwatcher::getInstance();
  if (new_process_begin_callback_id != 0) {
    LOG_LR(wmi_watcher.deRegisterCallback(new_process_begin_callback_id),
           "De-register New Process callback");
    new_process_begin_callback_id = 0;
  }
  if (new_process_end_callback_id != 0) {
    LOG_LR(wmi_watcher.deRegisterCallback(new_process_end_callback_id),
           "De-register End Process callback");
    new_process_end_callback_id = 0;
  }
  return LOG_END;
}

LOG_RETURN_TYPE ProcessMonitor::setForegroundProcessAffinity(
    uint64_t affinity_mask) {
  LOG_BEGIN;

  HWND handle = GetForegroundWindow();
  LOG_EC(handle == NULL ? LOG_NOK : LOG_OK, "GetForegroundWindow()");
  if (IS_LOG_OK) {
    // Get Process ID
    DWORD pid;
    LOG_EC(GetWindowThreadProcessId(handle, &pid) == 0,
           "GetWindowThreadProcessId()");
    if (IS_LOG_OK) {
      // Check if we have a record of this process id
      LOG_EC(live_processes.count(pid) == 1 ? LOG_OK : LOG_NOK,
             "Check if process is tracked");
      // Get Process Name
      if (IS_LOG_OK) {
        std::string proc_name = live_processes.at(pid);
        // Store Process profile
        (void)proc_affinity_map.erase(proc_name);
        proc_affinity_map.emplace(proc_name, affinity_mask);
        LOG_LR(saveProcAffinityMapToDisk(),
               "Save Process Affinity map to disk");
        // Set affinity for all processes with the same name
        if (IS_LOG_OK) {
          for (auto& [pid, pid_to_name] : live_processes) {
            if (pid_to_name == proc_name) {
              // Set Process Affinity
              LOG_LR(setProcessAffinity(pid, affinity_mask),
                     "Set Process Affinity");
            }
          }
        }
      }
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE ProcessMonitor::getAllRunningProcesses() {
  LOG_BEGIN;

  DWORD pids[1024], processes_count;
  LOG_EC(EnumProcesses(pids, sizeof(pids), &processes_count) != 0
             ? LOG_OK
             : GetLastError(),
         "Enumerate Processes");

  // Here we do not log errors as otherwise we will get errors for every process
  // that requires administrative rights to be "queried" for its name
  if (IS_LOG_OK) {
    // Clear out current list of running processes
    live_processes.clear();
    // Retrieve name of each process pid
    processes_count /= sizeof(DWORD);
    for (uint16_t i = 0; i < processes_count; i++) {
      HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                  FALSE, pids[i]);
      // If the handle is NULL (Process that requires administrative rights to
      // be queried) we will not keep track of it
      if (handle != NULL) {
        TCHAR name[MAX_PATH] = {0};
        DWORD name_length = GetModuleBaseName(handle, NULL, name, MAX_PATH);
        if (name_length != 0) {
          live_processes.emplace(pids[i], std::string(name, name_length));
        }
        CloseHandle(handle);
      }
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE ProcessMonitor::setProcessAffinity(uint32_t pid,
                                                   uint64_t affinity_mask) {
  LOG_BEGIN;

  HANDLE handle = OpenProcess(
      PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_SET_INFORMATION,
      FALSE, pid);
  LOG_EC(handle == NULL ? LOG_NOK : LOG_OK, "OpenProcess()");

  if (IS_LOG_OK) {
    bool result;
    // Retrieve current process affinity mask
    uint64_t proc_affinity, system_affinity;
    result = GetProcessAffinityMask(handle, &proc_affinity, &system_affinity);
    LOG_EC(result ? LOG_OK : LOG_NOK, "GetProcessAffinityMask");
    // Apply new affinity mask only if it does not already match what we want
    if (IS_LOG_OK) {
      if (proc_affinity != affinity_mask) {
        result = SetProcessAffinityMask(handle, affinity_mask);
        LOG_EC(result ? LOG_OK : LOG_NOK, "SetProcessAffinityMask");
        if (IS_LOG_OK) {
          (void)SetPriorityClass(handle, HIGH_PRIORITY_CLASS);
          LOG_INFO(live_processes.at(pid) + " (" + std::to_string(pid) +
                   ") set to " + std::to_string(affinity_mask));
        }
      }
    }
    (void)CloseHandle(handle);
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
void ProcessMonitor::newProcessBegin(IWbemClassObject* data) {
  LOG_BEGIN;

  // Get TargetInstance which is an object of type Win32_Process
  _variant_t target_instance_v;
  LOG_EC(data->Get(L"TargetInstance", 0, &target_instance_v, NULL, NULL),
         "Get TargetInstance");
  if (IS_LOG_OK) {
    IUnknown* target_instance = target_instance_v;
    LOG_EC(target_instance->QueryInterface(IID_IWbemClassObject,
                                           reinterpret_cast<void**>(&data)),
           "Query TargetInstance interface");
    if (IS_LOG_OK) {
      // Get Process Name
      variant_t value;
      LOG_EC(data->Get(L"Name", 0, &value, NULL, NULL), "Get Process Name");
      std::string proc_name;
      if (IS_LOG_OK) {
        // UTF16 to UTF8
        int len = SysStringLen(value.bstrVal);
        int utf8_size = WideCharToMultiByte(CP_UTF8, 0, value.bstrVal, len,
                                            NULL, 0, NULL, NULL);
        proc_name = std::string(utf8_size, '\0');
        (void)WideCharToMultiByte(CP_UTF8, 0, value.bstrVal, len,
                                  proc_name.data(), proc_name.size(), NULL,
                                  NULL);
      }
      value.Clear();

      if (IS_LOG_OK) {
        // Get Process ID
        uint32_t proc_id;
        LOG_EC(data->Get(L"ProcessId", 0, &value, NULL, NULL),
               "Get Process ID");
        if (IS_LOG_OK) {
          proc_id = value.uintVal;
          live_processes.emplace(proc_id, proc_name);
        }
        value.Clear();

        // Apply affinity mask if there is a profile for the given process
        if (IS_LOG_OK) {
          if (proc_affinity_map.count(proc_name) == 1) {
            setProcessAffinity(proc_id, proc_affinity_map.at(proc_name));
          }
        }
      }
    }
    (void)target_instance->Release();
  }

  target_instance_v.Clear();
  (void)data->Release();

  LOG_END;
}

void ProcessMonitor::newProcessEnd(IWbemClassObject* data) {
  LOG_BEGIN;

  // Get TargetInstance which is an object of type Win32_Process
  _variant_t target_instance_v;
  LOG_EC(data->Get(L"TargetInstance", 0, &target_instance_v, NULL, NULL),
         "Get TargetInstance");
  if (IS_LOG_OK) {
    IUnknown* target_instance = target_instance_v;
    LOG_EC(target_instance->QueryInterface(IID_IWbemClassObject,
                                           reinterpret_cast<void**>(&data)),
           "Query TargetInstance interface");
    if (IS_LOG_OK) {
      variant_t value;

      uint32_t proc_id;
      LOG_EC(data->Get(L"ProcessId", 0, &value, NULL, NULL), "Get Process ID");
      if (IS_LOG_OK) {
        proc_id = value.uintVal;
        (void)live_processes.erase(proc_id);
      }

      value.Clear();
    }
    (void)target_instance->Release();
  }

  target_instance_v.Clear();
  (void)data->Release();

  LOG_END;
}

}  // namespace PCC
