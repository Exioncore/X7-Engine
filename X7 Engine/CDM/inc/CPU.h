#pragma once
// System Includes
#include <string>
// 3rd Party Include
// Interal Module Includes
// External Module Includes
#include "EasyLogger.h"
#include "SensorTree.h"

namespace CDM {
class CPU {
 public:
  enum VENDOR { AMD, INTEL, VENDOR_UNKNOWN };
  enum FAMILY { AMD_ZEN4, AMD_ZEN3, FAMILY_UNKNOWN };

  static VENDOR parseVendor(uint32_t ebx, uint32_t edx, uint32_t ecx);
  static FAMILY parseFamily(CPU::VENDOR vendor, uint8_t family,
                            uint8_t e_family, uint8_t model, uint8_t e_model);

  virtual LOG_RETURN_TYPE initialize(
      std::shared_ptr<MDI::SensorTree> sensor_tree, FAMILY family,
      uint16_t logical_cores) = 0;
  virtual LOG_RETURN_TYPE update(std::vector<uint8_t>& core_usage) = 0;
};

}  // namespace CDM