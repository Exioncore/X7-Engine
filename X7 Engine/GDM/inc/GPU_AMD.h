#pragma once
// System Includes
#include <memory>
// 3rd Party Include
#include <ADLXHelper.h>
#include <IPerformanceMonitoring.h>
// Interal Module Includes
// External Module Includes
#include "GPUMetrics.h"
#include "Monitor.h"
#include "SensorTree.h"

namespace GDM {
class GPU_AMD : public MDI::Monitor {
 private:
  struct GPUHandle : GPUMetrics {
    adlx::IADLXGPUPtr handle;
    unsigned int total_vram;
  };

 public:
  GPU_AMD(std::shared_ptr<MDI::SensorTree> sensor_tree)
      : Monitor(sensor_tree) {}

  // Methods
  LOG_RETURN_TYPE initialize() override;
  LOG_RETURN_TYPE update() override;

 private:
  LOGGER("GPU_AMD");

  // Methods
  LOG_RETURN_TYPE initializeGPUlist();
  LOG_RETURN_TYPE initializePerfCounters();
  void createSensor(adlx_bool supported, GPUHandle& gpu,
                    std::shared_ptr<MDI::Sensor>& sensor, std::string name,
                    MDI::Sensor::Type type, std::string unit);

  ADLXHelper adlx;
  adlx::IADLXGPUListPtr gpuListPtr;
  adlx::IADLXPerformanceMonitoringServicesPtr perfMonitoringServices;

  std::vector<GPUHandle> gpus;
};

}  // namespace GDM