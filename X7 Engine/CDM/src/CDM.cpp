#include "CDM.h"
// System Includes
#include <format>
// 3rd Party Include
// Interal Module Includes
#include "AMD_ZEN.h"
// External Module Includes
#include "KernelAccess.h"

using namespace KAW;

namespace CDM {
LOG_RETURN_TYPE CDM::initialize() {
  LOG_BEGIN;

  std::shared_ptr<MDI::SensorTree> cpu_sensor_tree;
  CPU::VENDOR vendor = CPU::VENDOR_UNKNOWN;
  CPU::FAMILY family = CPU::FAMILY_UNKNOWN;
  uint16_t logical_cores_count = 0;

  LOG_LR(KernelAccess::initialize(), "Initialize Kernel Access Module");
  if (IS_LOG_OK) {
    LOG_EC(KernelAccess::supportCpuId(), "CPUID support");
  }
  if (IS_LOG_OK) {
    LOG_EC(KernelAccess::supportMsr(), "MSR support");
  }
  if (IS_LOG_OK) {
    std::string name;
    // Retrieve CPU vendor
    uint32_t eax, ebx, ecx, edx;
    LOG_EC(KernelAccess::cpuId(KernelAccess::CPUID::VENDOR, &eax, &ebx, &ecx,
                               &edx),
           "CPUID brand information");
    if (IS_LOG_OK) {
      vendor = CPU::parseVendor(ebx, edx, ecx);
      LOG_EC(KernelAccess::cpuId(KernelAccess::CPUID::EXTENDED_INFO, &eax, &ebx,
                                 &ecx, &edx),
             "CPUID family information");
    }
    // Retrieve CPU family
    if (IS_LOG_OK) {
      uint8_t r_family = ((eax & 0x000F00) >> 8);
      uint8_t r_efamily = ((eax & 0xF00000) >> 20);
      uint8_t r_model = ((eax & 0x0000F0) >> 4);
      uint8_t r_emodel = ((eax & 0x0F0000) >> 16);
      family = CPU::parseFamily(vendor, r_family, r_efamily, r_model, r_emodel);
    }
    // Retrieve CPU name
    if (IS_LOG_OK) {
      uint32_t regs[4];
      KernelAccess::CPUID regs_addr[3] = {KernelAccess::CPUID::BRAND_STRING_1,
                                          KernelAccess::CPUID::BRAND_STRING_2,
                                          KernelAccess::CPUID::BRAND_STRING_3};
      for (uint8_t i = 0; i < 3 && IS_LOG_OK; i++) {
        LOG_EC(KernelAccess::cpuId(regs_addr[i], &regs[0], &regs[1], &regs[2],
                                   &regs[3]),
               "CPUID name information");
        if (IS_LOG_OK) {
          for (uint8_t s = 0; s < 4; s++) {
            name += std::string((const char*)&regs[s], 4);
          }
        }
      }
      if (IS_LOG_OK) {
        while (name.back() == ' ' || name.back() == '\0') name.pop_back();
        cpu_sensor_tree = std::make_shared<MDI::SensorTree>(name);
      }
    }
    // Retrieve CPU logical cores count
    if (IS_LOG_OK) {
      LOG_EC(KernelAccess::cpuId(KernelAccess::CPUID::EXTENDED_INFO, &eax, &ebx,
                                 &ecx, &edx),
             "CPUID logical cores count");
      if (IS_LOG_OK) {
        logical_cores_count = ((ebx >> 16) & 0xFF);
      }
    }
  }

  // Setup CPU Core Usage sensors
  if (IS_LOG_OK) {
    LOG_LR(initializeCoreUsageSensors(logical_cores_count),
           "Initialize CPU Core Usage Sensors");
  }

  // Instantiate CPU
  if (IS_LOG_OK) {
    cpu = std::make_unique<AMD_ZEN>();
    LOG_LR(cpu->initialize(cpu_sensor_tree, family, logical_cores_count),
           "Initialize CPU monitoring");
    if (IS_LOG_OK) {
      parent_sensor_tree->addSensorTree(cpu_sensor_tree);
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE CDM::update() {
  LOG_BEGIN;

  LOG_LR(updateCoreUsageSensors(), "Update Core Usage Sensors");
  if (IS_LOG_OK) {
    LOG_LR(cpu->update(core_usage), "Update CPU specific Sensors");
  }

  return LOG_END;
}

LOG_RETURN_TYPE CDM::initializeCoreUsageSensors(uint16_t n_logical_cores) {
  LOG_BEGIN;

  LOG_EC(PdhOpenQuery(NULL, 0, &query), "Open PDH query");
  if (IS_LOG_OK) {
    std::string instance = "_Total";
    std::string path;
    for (uint16_t i = 0; i <= n_logical_cores; i++) {
      counter.push_back(PDH_HCOUNTER());
      path = "\\Processor(" + instance + ")\\% Idle Time";
      LOG_EC(PdhAddCounter(query, std::string(path.begin(), path.end()).c_str(),
                           0, &counter[i]),
             "Create PDH counter for instance " + instance);
      if (IS_LOG_OK) {
        instance = std::to_string(i);
        core_usage.push_back(0);
      }
    }
  }
  if (IS_LOG_OK) {
    LOG_EC(PdhCollectQueryData(query), "Collect PDH query data for cpu usage");
  }

  return LOG_END;
}

LOG_RETURN_TYPE CDM::updateCoreUsageSensors() {
  LOG_BEGIN;

  LOG_EC(PdhCollectQueryData(query), "Collect PDH query data for cpu usage");
  for (uint16_t i = 0; i < counter.size(); i++) {
    DWORD dwType;
    PDH_FMT_COUNTERVALUE Value;
    LOG_EC(PdhGetFormattedCounterValue(counter[i], PDH_FMT_DOUBLE, &dwType,
                                       &Value),
           "Get PDH instance data");
    if (IS_LOG_OK) {
      core_usage[i] =
          static_cast<uint8_t>(std::round(100.0 - Value.doubleValue));
    }
  }

  return LOG_END;
}

}  // namespace CDM