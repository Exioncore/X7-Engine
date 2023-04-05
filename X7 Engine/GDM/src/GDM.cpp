#include "GDM.h"
// System Includes
#include <format>
#include <iostream>
// 3rd Party Include
// Interal Module Includes
#include "GPU_AMD.h"
#include "GPU_NVIDIA.h"
// External Module Includes

namespace GDM {
LOG_RETURN_TYPE GDM::initialize() {
  LOG_BEGIN;

  bool has_amd, has_nvidia, has_intel;
  short gpu_count = 0;
  has_amd = has_nvidia = has_intel = false;
#ifdef _WIN32
  IDXGIAdapter* pAdapter;
  IDXGIFactory* pFactory;
  // Query Video Adapters
  HRESULT r = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
  LOG_EC(SUCCEEDED(r) ? 0 : r, "Create DXGIFactory");
  if (IS_LOG_OK) {
    for (uint8_t i = 0;
         pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
      DXGI_ADAPTER_DESC pDesc;
      r = pAdapter->GetDesc(&pDesc);
      LOG_EC(SUCCEEDED(r) ? 0 : r, "Get Adapter Description");
      if (IS_LOG_OK) {
        switch (pDesc.VendorId) {
          case 0x1002:
            LOG_INFO("AMD GPU detected");
            has_amd = true;
            gpu_count++;
            break;
          case 0x8086:
            LOG_INFO("INTEL GPU detected");
            has_intel = true;
            gpu_count++;
            break;
          case 0x10DE:
            LOG_INFO("NVIDIA GPU detected");
            has_nvidia = true;
            gpu_count++;
            break;
          default:
            LOG_INFO(
                std::format("Unknown GPU vendor ({0:#x})", pDesc.VendorId));
            break;
        }
      }
      (void)pAdapter->Release();
    }
    (void)pFactory->Release();
  }
#endif
  // Determine root for GPU sensors
  if (gpu_count > 1) {
    root_devices_tree = std::make_shared<MDI::SensorTree>("GPUs");
    parent_sensor_tree->addSensorTree(root_devices_tree);
  } else {
    root_devices_tree = parent_sensor_tree;
  }
  // Initialize AMD devices
  if (has_amd) {
    amd_devices = std::make_shared<GPU_AMD>(root_devices_tree);
    amd_devices->initialize();
  }
  // Initialize NVIDIA devices
  if (has_nvidia) {
    nvidia_devices = std::make_shared<GPU_NVIDIA>(root_devices_tree);
    nvidia_devices->initialize();
  }

  return LOG_END;
}

LOG_RETURN_TYPE GDM::update() {
  LOG_BEGIN;

  LOG_ERROR_TYPE error_code = LOG_OK;
  if (amd_devices) {
    LOG_LR(amd_devices->update(), "Update AMD GPUs telemetry");
    if (!IS_LOG_OK) error_code = LOG_NOK;
  }
  if (intel_devices) {
    LOG_LR(intel_devices->update(), "Update Intel GPUs telemetry");
    if (!IS_LOG_OK) error_code = LOG_NOK;
  }
  if (nvidia_devices) {
    LOG_LR(nvidia_devices->update(), "Update NVIDIA GPUs telemetry");
    if (!IS_LOG_OK) error_code = LOG_NOK;
  }

  return LOG_END_EC(error_code);
}

}  // namespace GDM