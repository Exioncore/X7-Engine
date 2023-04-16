#include "CPU.h"
// System Includes
#include <format>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace CDM {
CPU::VENDOR CPU::parseVendor(uint32_t ebx, uint32_t edx, uint32_t ecx) {
  std::string str;
  str += std::string((const char*)&ebx, 4);
  str += std::string((const char*)&edx, 4);
  str += std::string((const char*)&ecx, 4);
  if (str == "AuthenticAMD") {
    return AMD;
  } else if (str == "GenuineIntel") {
    return INTEL;
  } else {
    return VENDOR_UNKNOWN;
  }
}

CPU::FAMILY CPU::parseFamily(uint8_t family, uint8_t e_family, uint8_t model,
                             uint8_t e_model) {
  if (family == 0xF && e_family == 0xA && model == 0x1 && e_model == 0x6)
    return AMD_ZEN4;
  else
    return FAMILY_UNKNOWN;
}

}  // namespace CDM