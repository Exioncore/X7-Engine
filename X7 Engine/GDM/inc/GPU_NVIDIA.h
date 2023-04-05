#pragma once
// System Includes
#include <memory>
// 3rd Party Include
#include <nvapi.h>
#include <nvml.h>
// Interal Module Includes
// External Module Includes
#include "GPUMetrics.h"
#include "Monitor.h"
#include "SensorTree.h"

/*
NOTE:
NVML is the ideal choice for monitoring however, GPU Core clock and Memory clock
are wrongly reported on non primary GPU's.
NVAPI reports the correct data however, it cannot report Fan Speed and Power
usage thus it must be used in combination with NVML
*/

namespace GDM {
class GPU_NVIDIA : public MDI::Monitor {
 private:
  struct GPUHandle : GPUMetrics {
    GPUHandle() : nvapi_handle(NULL), nvml_handle(NULL){};
    GPUHandle(NvPhysicalGpuHandle nvapi_handle, nvmlDevice_t nvml_handle)
        : nvapi_handle(nvapi_handle), nvml_handle(nvml_handle){};

    nvmlDevice_t nvml_handle;
    NvPhysicalGpuHandle nvapi_handle;
    std::shared_ptr<MDI::Sensor> mem_controller_usage;
  };

 public:
  GPU_NVIDIA(std::shared_ptr<MDI::SensorTree> sensor_tree)
      : Monitor(sensor_tree) {}

  // Methods
  LOG_RETURN_TYPE initialize() override;
  LOG_RETURN_TYPE update() override;

 private:
  LOGGER("GPU_NVIDIA", true, true);

  // Methods
  LOG_RETURN_TYPE initializeGPUHandle(GPUHandle& gpu, uint16_t index);

  std::vector<GPUHandle> gpus;
};

}  // namespace GDM