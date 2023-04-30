#pragma once
// System Includes
#include <string>
#include <vector>
// 3rd Party Include
#pragma comment(lib, "pdh.lib")
#include <Pdh.h>
// Interal Module Includes
#include "CPU.h"
// External Module Includes
#include "Monitor.h"
#include "SensorTree.h"

namespace CDM {
class CDM : public MDI::Monitor {
 public:
  CDM(std::shared_ptr<MDI::SensorTree> sensor_tree) : Monitor(sensor_tree) {}

  LOG_RETURN_TYPE initialize() override;
  LOG_RETURN_TYPE update() override;

 private:
  LOGGER("CDM");

  // CPU
  std::unique_ptr<CPU> cpu;

  // PDH
  PDH_HQUERY query;
  std::vector<PDH_HCOUNTER> counter;
  std::vector<uint8_t> core_usage;

  // Methods
  LOG_RETURN_TYPE initializeCoreUsageSensors(uint16_t n_logical_cores);
  LOG_RETURN_TYPE updateCoreUsageSensors();
};

}  // namespace CDM