#include "KernelAccess.h"
// System Includes
#include <windows.h>

#include <string>
// 3rd Party Include
#include "OlsApi.h"
// Interal Module Includes
// External Module Includes

// Ensure DWORD size matches uint32_t size
// Ensure DWORD_PTR size matches uint64_t size
C_ASSERT(sizeof(DWORD) == sizeof(uint32_t));
C_ASSERT(sizeof(DWORD_PTR) == sizeof(uint64_t));

namespace KAW {
LOG_RETURN_TYPE KernelAccess::initialize() {
  LOG_BEGIN;

  if (!is_initialized) {
    LOG_EC(InitializeOls() ? LOG_OK : LOG_NOK, "WinRing0 InitializeOls");
    if (IS_LOG_OK) {
      LOG_EC(GetDllStatus(), "WinRing0 GetDllStatus");
      if (IS_LOG_OK) {
        BYTE dll_major, dll_minor, dll_revision, dll_release;
        (void)GetDllVersion(&dll_major, &dll_minor, &dll_revision,
                            &dll_release);
        BYTE driver_major, driver_minor, driver_revision, driver_release;
        (void)GetDriverVersion(&driver_major, &driver_minor, &driver_revision,
                               &driver_release);
        LOG_INFO("WinRing0 (DLL: " + std::to_string(dll_major) + "." +
                 std::to_string(dll_minor) + "." +
                 std::to_string(dll_revision) + "." +
                 std::to_string(dll_release) + ", " +
                 "Driver: " + std::to_string(driver_major) + "." +
                 std::to_string(driver_minor) + "." +
                 std::to_string(driver_revision) + "." +
                 std::to_string(driver_release) + ") initialized");
        is_initialized = true;
      }
    }
  }

  return LOG_END;
}

void KernelAccess::deInitialize() {
  if (is_initialized) {
    DeinitializeOls();
    is_initialized = false;
  }
}

LOG_ERROR_TYPE KernelAccess::supportCpuId() {
  return IsCpuid() ? LOG_OK : LOG_NOK;
}

LOG_ERROR_TYPE KernelAccess::supportMsr() { return IsMsr() ? LOG_OK : LOG_NOK; }

LOG_ERROR_TYPE KernelAccess::cpuId(uint32_t index, uint32_t* eax, uint32_t* ebx,
                                   uint32_t* ecx, uint32_t* edx) {
  return Cpuid(index, (PDWORD)eax, (PDWORD)ebx, (PDWORD)ecx, (PDWORD)edx)
             ? LOG_OK
             : LOG_NOK;
}

LOG_ERROR_TYPE KernelAccess::readMsr(uint32_t index, uint32_t* eax,
                                     uint32_t* edx) {
  return Rdmsr(index, (PDWORD)eax, (PDWORD)edx) ? LOG_OK : LOG_NOK;
}

LOG_ERROR_TYPE KernelAccess::readMsr(uint32_t index, uint32_t* eax,
                                     uint32_t* edx, uint64_t cpu_mask) {
  return RdmsrPx(index, (PDWORD)eax, (PDWORD)edx, (DWORD_PTR)cpu_mask)
             ? LOG_OK
             : LOG_NOK;
}

LOG_ERROR_TYPE KernelAccess::writePciRegister(uint32_t pci_addr,
                                              uint32_t register_addr,
                                              uint32_t value) {
  return WritePciConfigDwordEx(pci_addr, register_addr, (DWORD)value) ? LOG_OK
                                                                      : LOG_NOK;
}

LOG_ERROR_TYPE KernelAccess::readPciRegister(uint32_t pci_addr,
                                             uint32_t register_addr,
                                             uint32_t* value) {
  return ReadPciConfigDwordEx(pci_addr, register_addr, (PDWORD)value) ? LOG_OK
                                                                      : LOG_NOK;
}
}  // namespace KAW