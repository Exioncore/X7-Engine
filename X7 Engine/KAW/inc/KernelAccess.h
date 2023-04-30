#pragma once
// System Includes
#include <stdint.h>
// 3rd Party Include
// Interal Module Includes
// External Module Includes
#include "EasyLogger.h"

namespace KAW {
/**
 * @brief Kernel Access Wrapper
 * @details This class is designed to provide cross-platform access to
 *          cpuid data and msr data
 *          Windows: WinRing0
 *          Linux: ToDo
 */
class KernelAccess {
 public:
  enum CPUID {
    VENDOR = 0,
    EXTENDED_INFO = 1,
    BRAND_STRING_1 = 0x80000002,
    BRAND_STRING_2 = 0x80000003,
    BRAND_STRING_3 = 0x80000004
  };

  static LOG_RETURN_TYPE initialize();
  static void deInitialize();
  static LOG_ERROR_TYPE supportCpuId();
  static LOG_ERROR_TYPE supportMsr();
  static LOG_ERROR_TYPE cpuId(uint32_t index, uint32_t* eax, uint32_t* ebx,
                              uint32_t* ecx, uint32_t* edx);
  static LOG_ERROR_TYPE readMsr(uint32_t index, uint32_t* eax, uint32_t* edx);
  static LOG_ERROR_TYPE readMsr(uint32_t index, uint32_t* eax, uint32_t* edx,
                                uint64_t cpu_mask);
  static LOG_ERROR_TYPE writePciRegister(uint32_t pci_addr,
                                         uint32_t register_addr,
                                         uint32_t value);
  static LOG_ERROR_TYPE readPciRegister(uint32_t pci_addr,
                                        uint32_t register_addr,
                                        uint32_t* value);

 private:
  LOGGER("KernelAccess");

  inline static bool is_initialized = false;
};
}  // namespace KAW