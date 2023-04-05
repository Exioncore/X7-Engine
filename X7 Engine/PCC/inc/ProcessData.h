#pragma once
// System Includes
#include <string>
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
// External Module Includes

namespace PCC {
class ProcessData {
 public:
  ProcessData();
  ~ProcessData();

  // Setters
  void set(HANDLE handle, std::string path);

  // Getters
  const std::string& getPath();

  // Methods
  int8_t setAffinity(DWORD_PTR affinity_mask);

 private:
  HANDLE handle;
  std::string path;
};

}  // namespace PCC