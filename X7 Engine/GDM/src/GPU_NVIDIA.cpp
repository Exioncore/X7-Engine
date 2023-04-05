#include "GPU_NVIDIA.h"
// System Includes
#include <format>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace GDM {
LOG_RETURN_TYPE GPU_NVIDIA::initialize() {
  LOG_BEGIN;
  LOG_ERROR_TYPE error_code = LOG_OK;

  // API's initialization
  LOG_EC(NvAPI_Initialize(), "Initialize NvAPI");
  if (IS_LOG_OK) {
    LOG_EC(nvmlInit(), "Initialize NVML");
  }
  if (IS_LOG_OK) {
    // NVAPI handles
    NvPhysicalGpuHandle nvapi_gpu_handle[NVAPI_MAX_PHYSICAL_GPUS];
    NvU32 nvapi_device_count;
    LOG_EC(NvAPI_EnumPhysicalGPUs(nvapi_gpu_handle, &nvapi_device_count),
           "NVAPI_Enumerate devices");
    // NVML handles
    if (IS_LOG_OK) {
      unsigned int nvml_device_count;
      LOG_EC(nvmlDeviceGetCount(&nvml_device_count), "NVML Enumerate devices");
      if (IS_LOG_OK)
        LOG_EC(nvml_device_count != nvapi_device_count,
               "Check if NVAPI device count matches with NVML");
    }

    // Initialize each GPU handle (NVAPI and NVML)
    if (IS_LOG_OK) {
      for (uint8_t i = 0; i < nvapi_device_count; i++) {
        nvmlDevice_t nvml_handle;
        LOG_EC(nvmlDeviceGetHandleByIndex(i, &nvml_handle),
               "Get NVML Device handle");
        if (IS_LOG_OK) {
          GPUHandle gpu(nvapi_gpu_handle[i], nvml_handle);
          LOG_LR(initializeGPUHandle(gpu, i), "Initialize GPU Handle");
          if (IS_LOG_OK) {
            gpus.push_back(gpu);
            parent_sensor_tree->addSensorTree(gpu.root_sensor_tree);
          } else {
            error_code = LOG_NOK;
          }
        } else {
          error_code = LOG_NOK;
        }
      }
    }

    LOG_END_EC(error_code);
  }

  return LOG_END;
}

LOG_RETURN_TYPE GPU_NVIDIA::update() {
  LOG_BEGIN;
  LOG_ERROR_TYPE error_code = LOG_OK;
  for (auto& gpu : this->gpus) {
    /* NVAPI */
    NV_GPU_THERMAL_SETTINGS thermals = {NV_GPU_THERMAL_SETTINGS_VER};
    LOG_EC(NvAPI_GPU_GetThermalSettings(gpu.nvapi_handle,
                                        NVAPI_THERMAL_TARGET_ALL, &thermals),
           "Retrieve GPU Temperature");
    if (IS_LOG_OK)
      gpu.temperature->setValue(
          thermals.sensor[NVAPI_THERMAL_TARGET_NONE].currentTemp);
    else
      error_code = LOG_NOK;

    NV_GPU_DYNAMIC_PSTATES_INFO_EX states_info = {
        NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER};
    LOG_EC(NvAPI_GPU_GetDynamicPstatesInfoEx(gpu.nvapi_handle, &states_info),
           "Retrieve GPU Usages");
    if (IS_LOG_OK) {
      gpu.core_usage->setValue(
          states_info.utilization[NV_GPU_CLIENT_UTIL_DOMAIN_GRAPHICS]
              .percentage);
      gpu.mem_controller_usage->setValue(
          states_info.utilization[NV_GPU_CLIENT_UTIL_DOMAIN_FRAME_BUFFER]
              .percentage);
    } else {
      error_code = LOG_NOK;
    }

    NV_GPU_CLOCK_FREQUENCIES clocks_info = {NV_GPU_CLOCK_FREQUENCIES_VER};
    LOG_EC(NvAPI_GPU_GetAllClockFrequencies(gpu.nvapi_handle, &clocks_info),
           "Retrieve GPU Clocks");
    if (IS_LOG_OK) {
      gpu.core_clock->setValue(
          clocks_info.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency /
          1000.0);
      gpu.vram_clock->setValue(
          clocks_info.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency / 1000.0);
    } else {
      error_code = LOG_NOK;
    }

    NV_GPU_MEMORY_INFO_EX memory_info = {NV_GPU_MEMORY_INFO_EX_VER};
    LOG_EC(NvAPI_GPU_GetMemoryInfoEx(gpu.nvapi_handle, &memory_info),
           "Retrieve GPU Memory Usage");
    if (IS_LOG_OK)
      gpu.vram_usage->setValue(
          100.0 - ((double)memory_info.curAvailableDedicatedVideoMemory /
                   memory_info.dedicatedVideoMemory * 100.0));
    else
      error_code = LOG_NOK;

    /* NVML*/
    unsigned int value;
    LOG_EC(nvmlDeviceGetPowerUsage(gpu.nvml_handle, &value),
           "Retrieve GPU Power Usage");
    if (IS_LOG_OK)
      gpu.power->setValue(value / 1000);
    else
      error_code = LOG_NOK;

    LOG_EC(nvmlDeviceGetFanSpeed(gpu.nvml_handle, &value),
           "Retrieve GPU Fan Speed");
    if (IS_LOG_OK)
      gpu.fan_speed->setValue(value);
    else
      error_code = LOG_NOK;
  }

  return LOG_END_EC(error_code);
}

LOG_RETURN_TYPE GPU_NVIDIA::initializeGPUHandle(GPUHandle& gpu,
                                                uint16_t index) {
  LOG_BEGIN;

  NvAPI_ShortString name;
  LOG_EC(NvAPI_GPU_GetFullName(gpu.nvapi_handle, name), "Get Device name");

  if (IS_LOG_OK) {
    gpu.root_sensor_tree = std::make_shared<MDI::SensorTree>(std::string(name));
    gpu.temperature = std::make_shared<MDI::Sensor>(
        "Temperature", MDI::Sensor::TEMPERATURE, "°C");
    gpu.root_sensor_tree->addSensor(gpu.temperature);
    gpu.core_usage =
        std::make_shared<MDI::Sensor>("Core Usage", MDI::Sensor::USAGE, "%");
    gpu.root_sensor_tree->addSensor(gpu.core_usage);
    gpu.core_clock = std::make_shared<MDI::Sensor>(
        "Core Clock", MDI::Sensor::FREQUENCY, "MHZ");
    gpu.root_sensor_tree->addSensor(gpu.core_clock);
    gpu.mem_controller_usage = std::make_shared<MDI::Sensor>(
        "Memory Controller Usage", MDI::Sensor::USAGE, "%");
    gpu.root_sensor_tree->addSensor(gpu.mem_controller_usage);
    gpu.vram_usage =
        std::make_shared<MDI::Sensor>("VRAM Usage", MDI::Sensor::USAGE, "%");
    gpu.root_sensor_tree->addSensor(gpu.vram_usage);
    gpu.vram_clock = std::make_shared<MDI::Sensor>(
        "VRAM Clock", MDI::Sensor::FREQUENCY, "MHZ");
    gpu.root_sensor_tree->addSensor(gpu.vram_clock);
    gpu.fan_speed = std::make_shared<MDI::Sensor>(
        "Fan Speed", MDI::Sensor::ROTATIONAL_SPEED, "%");
    gpu.root_sensor_tree->addSensor(gpu.fan_speed);
    gpu.power = std::make_shared<MDI::Sensor>("Power", MDI::Sensor::POWER, "W");
    gpu.root_sensor_tree->addSensor(gpu.power);
    LOG_INFO(std::format("GPU {0} named {1} initialized", index,
                         gpu.root_sensor_tree->getName()));
  }

  return LOG_END;
}

}  // namespace GDM