#include "GPU_AMD.h"
// System Includes
#include <format>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

// Use ADLX namespace
using namespace adlx;

namespace GDM {
LOG_RETURN_TYPE GPU_AMD::initialize() {
  LOG_BEGIN;

  // Initialize ADLX
  ADLX_RESULT res = ADLX_FAIL;
  res = this->adlx.Initialize();
  LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "ADLX Initialize");
  if (IS_LOG_OK) {
    // Initialize GPUs
    LOG_LR(this->initializeGPUlist(), "Initialize list of GPUs");
    // Initialize Performance Monitoring service
    if (IS_LOG_OK) {
      LOG_LR(this->initializePerfCounters(),
             "Initialize GPUs Performance Counters");
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE GPU_AMD::update() {
  LOG_BEGIN;
  LOG_ERROR_TYPE error_code = LOG_OK;

  for (auto& gpu : this->gpus) {
    IADLXGPUMetricsPtr gpuMetrics;
    ADLX_RESULT res = this->perfMonitoringServices->GetCurrentGPUMetrics(
        gpu.handle, &gpuMetrics);
    LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
           "Retrieve current GPU metrics from performance monitoring service");
    if (IS_LOG_OK) {
      int sensor_index = 0;
      // Temperature
      if (gpu.temperature) {
        adlx_double value = 0;
        res = gpuMetrics->GPUTemperature(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Get GPU Temperature");
        if (IS_LOG_OK)
          gpu.temperature->setValue(value);
        else
          error_code = LOG_NOK;
      }
      // Core Usage
      if (gpu.core_usage) {
        adlx_double value = 0;
        res = gpuMetrics->GPUUsage(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Get GPU Utilization");
        if (IS_LOG_OK)
          gpu.core_usage->setValue(value);
        else
          error_code = LOG_NOK;
      }
      // Core Clock
      if (gpu.core_clock) {
        adlx_int value = 0;
        res = gpuMetrics->GPUClockSpeed(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Retrieve GPU Core Clock");
        if (IS_LOG_OK)
          gpu.core_clock->setValue(value);
        else
          error_code = LOG_NOK;
      }
      // VRAM Usage
      if (gpu.vram_usage) {
        adlx_int value = 0;
        res = gpuMetrics->GPUVRAM(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Retrieve GPU Memory Usage");
        if (IS_LOG_OK)
          gpu.vram_usage->setValue((double)value / gpu.total_vram * 100.0);
        else
          error_code = LOG_NOK;
      }
      // VRAM Clock
      if (gpu.vram_clock) {
        adlx_int value = 0;
        res = gpuMetrics->GPUVRAMClockSpeed(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Retrive GPU VRAM Clock");
        if (IS_LOG_OK)
          gpu.vram_clock->setValue(value);
        else
          error_code = LOG_NOK;
      }
      // Fan Speed
      if (gpu.fan_speed) {
        adlx_int value = 0;
        res = gpuMetrics->GPUFanSpeed(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Retrieve GPU Fan Speed");
        if (IS_LOG_OK)
          gpu.fan_speed->setValue(value);
        else
          error_code = LOG_NOK;
      }
      // Power
      if (gpu.power) {
        adlx_double value = 0;
        res = gpuMetrics->GPUPower(&value);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Retrieve GPU Power Usage");
        if (IS_LOG_OK)
          gpu.power->setValue(value);
        else
          error_code = LOG_NOK;
      }
    } else {
      error_code = LOG_NOK;
    }
  }

  return LOG_END_EC(error_code);
}

LOG_RETURN_TYPE GPU_AMD::initializeGPUlist() {
  LOG_BEGIN;

  // Get GPU list
  ADLX_RESULT res = this->adlx.GetSystemServices()->GetGPUs(&this->gpuListPtr);
  LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Enumerate devices");

  for (adlx_uint i = 0; ((i < this->gpuListPtr->Size()) && IS_LOG_OK); i++) {
    GPUHandle gpu;
    res = this->gpuListPtr->At(i, &gpu.handle);
    LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Get Device handle");
    if (IS_LOG_OK) {
      const char* gpu_name = nullptr;
      res = gpu.handle->Name(&gpu_name);
      LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Get Device name");
      if (IS_LOG_OK) {
        res = gpu.handle->TotalVRAM(&gpu.total_vram);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Get Device VRAM capacity");
        if (IS_LOG_OK) {
          gpu.root_sensor_tree = std::make_shared<MDI::SensorTree>(gpu_name);
          parent_sensor_tree->addSensorTree(gpu.root_sensor_tree);
          this->gpus.push_back(gpu);
          LOG_INFO(std::format("GPU {0} named {1} initialized", i,
                               gpu.root_sensor_tree->getName()));
        }
      } else {
        gpu.handle.Release();
      }
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE GPU_AMD::initializePerfCounters() {
  LOG_BEGIN;

  ADLX_RESULT res = adlx.GetSystemServices()->GetPerformanceMonitoringServices(
      &this->perfMonitoringServices);
  LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
         "Initialize ADLX Performance Monitoring Service");

  if (IS_LOG_OK) {
    LOG_ERROR_TYPE error_code = LOG_OK;
    // Loop through GPUS to query for available metrics
    for (auto& gpu : gpus) {
      // Query SupportedGPUMetrics
      IADLXGPUMetricsSupportPtr gpuMetricsSupport;
      res = this->perfMonitoringServices->GetSupportedGPUMetrics(
          gpu.handle, &gpuMetricsSupport);
      LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res, "Get GPU supported metrics");
      if (IS_LOG_OK) {
        adlx_bool supported;
        // Temperature
        res = gpuMetricsSupport->IsSupportedGPUTemperature(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports Temperature metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.temperature, "Temperature",
                       MDI::Sensor::TEMPERATURE, "°C");
        } else {
          error_code = LOG_NOK;
        }
        // Core Usage
        res = gpuMetricsSupport->IsSupportedGPUUsage(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports GPU Usage metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.core_usage, "Core Usage",
                       MDI::Sensor::USAGE, "%");
        } else {
          error_code = LOG_NOK;
        }
        // Core Clock
        res = gpuMetricsSupport->IsSupportedGPUClockSpeed(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports GPU Clock metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.core_clock, "Core Clock",
                       MDI::Sensor::FREQUENCY, "MHZ");
        } else {
          error_code = LOG_NOK;
        }
        // VRAM Usage
        res = gpuMetricsSupport->IsSupportedGPUVRAM(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports Memory Usage metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.vram_usage, "VRAM Usage",
                       MDI::Sensor::USAGE, "%");
        } else {
          error_code = LOG_NOK;
        }
        // VRAM Clock
        res = gpuMetricsSupport->IsSupportedGPUVRAMClockSpeed(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports VRAM Clock metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.vram_clock, "VRAM Clock",
                       MDI::Sensor::FREQUENCY, "MHZ");
        } else {
          error_code = LOG_NOK;
        }
        // Fan Percentage
        gpuMetricsSupport->IsSupportedGPUFanSpeed(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports Fan Speed metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.fan_speed, "Fan Speed",
                       MDI::Sensor::ROTATIONAL_SPEED, "RPM");
        } else {
          error_code = LOG_NOK;
        }
        // Power
        res = gpuMetricsSupport->IsSupportedGPUPower(&supported);
        LOG_EC(ADLX_SUCCEEDED(res) ? LOG_OK : res,
               "Check if GPU supports Power metrics");
        if (IS_LOG_OK) {
          createSensor(supported, gpu, gpu.power, "Power", MDI::Sensor::POWER,
                       "W");
        } else {
          error_code = LOG_NOK;
        }

        gpuMetricsSupport.Release();
      } else {
        error_code = LOG_NOK;
      }
    }

    return LOG_END_EC(error_code);
  }

  return LOG_END;
}

void GPU_AMD::createSensor(adlx_bool supported, GPUHandle& gpu,
                           std::shared_ptr<MDI::Sensor>& sensor,
                           std::string name, MDI::Sensor::Type type,
                           std::string unit) {
  if (supported) {
    sensor = std::make_shared<MDI::Sensor>(name, type, unit);
    gpu.root_sensor_tree->addSensor(sensor);
  }
}

}  // namespace GDM