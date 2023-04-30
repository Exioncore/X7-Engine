#pragma once
// System Includes
#include <vector>
// 3rd Party Include
#include <dxgi.h>
// Interal Module Includes
// External Module Includes
#include "Monitor.h"
#include "SensorTree.h"

namespace GDM {
class GDM : public MDI::Monitor {
 public:
  GDM(std::shared_ptr<MDI::SensorTree> sensor_tree) : Monitor(sensor_tree) {}

  LOG_RETURN_TYPE initialize() override;
  LOG_RETURN_TYPE update() override;

 private:
  LOGGER("GDM");

  std::shared_ptr<MDI::SensorTree> root_devices_tree;
  std::shared_ptr<MDI::Monitor> amd_devices;
  std::shared_ptr<MDI::Monitor> intel_devices;
  std::shared_ptr<MDI::Monitor> nvidia_devices;
};

}  // namespace GDM