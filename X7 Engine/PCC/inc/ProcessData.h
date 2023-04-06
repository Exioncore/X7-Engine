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
  int8_t setAffinity(uint64_t& affinity_mask);
  int8_t getAffinity(uint64_t& affinity_mask);

 private:
  HANDLE handle;
  std::string path;
};

}  // namespace PCC