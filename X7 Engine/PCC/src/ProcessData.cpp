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
int8_t ProcessData::setAffinity(uint64_t& affinity_mask) {
  return SetProcessAffinityMask(handle, affinity_mask) != 0 ? OK : 1;
}

int8_t ProcessData::getAffinity(uint64_t& affinity_mask) { 
  uint64_t system_affinity;
  return (GetProcessAffinityMask(handle, &affinity_mask, &system_affinity) == 0);
}

}  // namespace PCC