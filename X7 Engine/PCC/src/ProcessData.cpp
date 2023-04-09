#include "ProcessData.h"
// System Includes
#include <iostream>
// 3rd Party Include
#include <Psapi.h>
// Interal Module Includes
// External Module Includes

namespace PCC {
#define OK 0
///////////////////
// Process Class //
///////////////////
ProcessData::ProcessData() : handle(NULL), path("") {}

ProcessData::~ProcessData() {
  if (this->handle != NULL) {
    (void)CloseHandle(this->handle);
  }
}

///////////////
//// Setters //
///////////////
void ProcessData::set(HANDLE handle, std::string path) {
  // Close Previous handle if it was set
  if (this->handle != NULL) {
    (void)CloseHandle(this->handle);
  }
  // Store new handle and path
  this->handle = handle;
  this->path = path;
}

///////////////
//// Getters //
///////////////
const std::string& ProcessData::getPath() { return path; }

///////////////
//// Methods //
///////////////
int16_t ProcessData::setAffinity(uint64_t& affinity_mask) {
  int16_t error_code = OK;
  bool result = SetProcessAffinityMask(handle, affinity_mask);
  if (!result) {
    error_code = GetLastError();
  }
  return error_code;
}

int16_t ProcessData::getAffinity(uint64_t& affinity_mask) {
  int16_t error_code = OK;
  uint64_t system_affinity;
  bool result =
      GetProcessAffinityMask(handle, &affinity_mask, &system_affinity);
  if (!result) {
    error_code = GetLastError();
  }
  return error_code;
}

}  // namespace PCC