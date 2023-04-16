#pragma once
// System Includes
#include <string>
#include <vector>
// 3rd Party Include
// Interal Module Includes
#include "CPU.h"
// External Module Includes

namespace CDM {
class AMD_ZEN : public CPU {
 public:
  LOG_RETURN_TYPE initialize(std::shared_ptr<MDI::SensorTree> sensor_tree,
                             FAMILY family, uint16_t logical_cores) override;
  LOG_RETURN_TYPE update(std::vector<uint8_t>& core_usage) override;

 private:
  LOGGER("AMD_ZEN", true, true);

  // Processor layout information
  uint16_t logical_cores, physical_cores;
  uint8_t ccd_count;

  // Per-CCD data
  std::vector<std::shared_ptr<MDI::Sensor>> ccd_peak_frequency;
  std::vector<std::shared_ptr<MDI::Sensor>> ccd_peak_usage;
  std::vector<std::shared_ptr<MDI::Sensor>> ccd_total_usage;
  // CPU Overall data
  std::shared_ptr<MDI::Sensor> temperature;
  std::shared_ptr<MDI::Sensor> total_usage;
  std::shared_ptr<MDI::Sensor> peak_frequency;
  std::shared_ptr<MDI::Sensor> peak_usage;
  std::shared_ptr<MDI::Sensor> power;

  // Energy data
  std::chrono::steady_clock::time_point last_update_time;
  double energy_unit, prev_energy;

};

}  // namespace CDM