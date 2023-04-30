#pragma once
// System Includes
#include <string>
#include <vector>
// 3rd Party Include
#pragma comment(lib, "pdh.lib")
#include <Pdh.h>
// Interal Module Includes
// External Module Includes
#include "Monitor.h"
#include "SensorTree.h"

namespace DSM {
class DSM : public MDI::Monitor {
 public:
  DSM(std::shared_ptr<MDI::SensorTree> sensor_tree) : Monitor(sensor_tree) {}

  LOG_RETURN_TYPE initialize() override;
  LOG_RETURN_TYPE update() override;

 private:
  LOGGER("DSM");

  std::shared_ptr<MDI::SensorTree> root_devices_tree;
  // Networks
  struct Drive {
    PDH_HQUERY query;
    PDH_HCOUNTER activity_counter;
    PDH_HCOUNTER read_counter;
    PDH_HCOUNTER write_counter;

    std::shared_ptr<MDI::SensorTree> root;
    std::shared_ptr<MDI::Sensor> activity;
    std::shared_ptr<MDI::Sensor> read;
    std::shared_ptr<MDI::Sensor> write;
  };
  std::vector<Drive> drives;

  // Others
  static std::string strConverter(double value, std::string unit);
};

}  // namespace DSM