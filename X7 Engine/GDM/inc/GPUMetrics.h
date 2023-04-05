#pragma once
// System Includes
#include <memory>
// 3rd Party Include
// Interal Module Includes
// External Module Includes
#include "SensorTree.h"

namespace GDM {
struct GPUMetrics {
  std::shared_ptr<MDI::SensorTree> root_sensor_tree;
  std::shared_ptr<MDI::Sensor> temperature;
  std::shared_ptr<MDI::Sensor> core_usage;
  std::shared_ptr<MDI::Sensor> core_clock;
  std::shared_ptr<MDI::Sensor> vram_usage;
  std::shared_ptr<MDI::Sensor> vram_clock;
  std::shared_ptr<MDI::Sensor> fan_speed;
  std::shared_ptr<MDI::Sensor> power;
};

}  // namespace GDM