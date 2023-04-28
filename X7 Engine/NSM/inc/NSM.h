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

namespace NSM {
class NSM : public MDI::Monitor {
 public:
  NSM(std::shared_ptr<MDI::SensorTree> sensor_tree) : Monitor(sensor_tree) {}

  LOG_RETURN_TYPE initialize() override;
  LOG_RETURN_TYPE update() override;

 private:
  LOGGER("NSM", true, true);

  std::shared_ptr<MDI::SensorTree> root_devices_tree;
  // Networks
  struct Network {
    PDH_HQUERY query;
    PDH_HCOUNTER download_counter;
    PDH_HCOUNTER upload_counter;

    std::shared_ptr<MDI::SensorTree> root;
    std::shared_ptr<MDI::Sensor> download;
    std::shared_ptr<MDI::Sensor> upload;
  };
  std::vector<Network> networks;

  // Others
  static std::string strConverter(double value, std::string unit);
};

}  // namespace NSM