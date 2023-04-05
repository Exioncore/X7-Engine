#pragma once
// System Includes
// 3rd Party Include
// Interal Module Includes
#include "SensorTree.h"
// External Module Includes
#include "EasyLogger.h"

namespace MDI {
class Monitor {
 public:
  Monitor(std::shared_ptr<MDI::SensorTree> sensor_tree)
      : parent_sensor_tree(sensor_tree){};

  virtual LOG_RETURN_TYPE initialize() = 0;
  virtual LOG_RETURN_TYPE update() = 0;

 protected:
  std::shared_ptr<MDI::SensorTree> parent_sensor_tree;
};

}  // namespace MDI