#include "AMD_ZEN.h"
// System Includes
#include <chrono>
// 3rd Party Include
// Interal Module Includes
// External Module Includes
#include "KernelAccess.h"

using namespace KAW;

namespace CDM {
LOG_RETURN_TYPE AMD_ZEN::initialize(
    std::shared_ptr<MDI::SensorTree> sensor_tree, FAMILY family,
    uint16_t logical_cores) {
  LOG_BEGIN;

  // Retrieve amount of physical cores
  uint32_t eax, ebx, ecx, edx;
  LOG_EC(KernelAccess::cpuId(0x8000001E, &eax, &ebx, &ecx, &edx),
         "Retrieve amount of cores");
  if (IS_LOG_OK) {
    uint8_t threads_per_core = ((ebx >> 8) & 0xF) + 1;
    this->logical_cores = logical_cores;
    physical_cores = logical_cores / threads_per_core;
  }

  // Retrieve Energy Unit (Used for power calculations)
  if (IS_LOG_OK) {
    LOG_EC(KernelAccess::readMsr(0xC0010299, &eax, &edx),
           "Retrieve Energy Unit");
    if (IS_LOG_OK) {
      energy_unit = 1.0 / pow(2, ((eax >> 8) & 0x1F));
    }
  }

  // Setup sensor tree
  if (IS_LOG_OK) {
    // CPU Overall data
    temperature = std::make_shared<MDI::Sensor>("Temperature",
                                                MDI::Sensor::TEMPERATURE, "°C");
    sensor_tree->addSensor(temperature);
    peak_frequency = std::make_shared<MDI::Sensor>(
        "Frequency", MDI::Sensor::FREQUENCY, "MHz");
    sensor_tree->addSensor(peak_frequency);
    peak_usage = std::make_shared<MDI::Sensor>("Peak Thread Usage",
                                               MDI::Sensor::USAGE, "%");
    sensor_tree->addSensor(peak_usage);
    power = std::make_shared<MDI::Sensor>("Power", MDI::Sensor::POWER, "W");
    sensor_tree->addSensor(power);
    total_usage =
        std::make_shared<MDI::Sensor>("Usage", MDI::Sensor::USAGE, "%");
    sensor_tree->addSensor(total_usage);
    // Per-CCD data
    uint8_t n_ccds = static_cast<uint8_t>(std::ceil(logical_cores / 16.0));
    // Zen CCDs have at most 8 cores (16 threads)
    if (n_ccds > 1) {
      for (uint8_t i = 0; i < n_ccds; i++) {
        std::shared_ptr<MDI::SensorTree> ccd =
            std::make_shared<MDI::SensorTree>("CCD " + std::to_string(i));
        ccd_peak_frequency.push_back(std::make_shared<MDI::Sensor>(
            "Frequency", MDI::Sensor::FREQUENCY, "MHz"));
        ccd->addSensor(ccd_peak_frequency[i]);
        ccd_peak_usage.push_back(std::make_shared<MDI::Sensor>(
            "Peak Thread Usage", MDI::Sensor::USAGE, "%"));
        ccd->addSensor(ccd_peak_usage[i]);
        ccd_total_usage.push_back(
            std::make_shared<MDI::Sensor>("Usage", MDI::Sensor::USAGE, "%"));
        ccd->addSensor(ccd_total_usage[i]);
        sensor_tree->addSensorTree(ccd);
      }
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE AMD_ZEN::update(std::vector<uint8_t>& core_usage) {
  LOG_BEGIN;

  // Usage Data
  uint8_t logical_cores_per_ccd = 0;
  uint8_t overall_peak_usage = 0;
  total_usage->setValue(core_usage[0]);
  if (ccd_total_usage.size() > 0) {
    logical_cores_per_ccd = logical_cores / ccd_total_usage.size();
    //// Usage Data per CCD
    for (uint8_t i = 0; i < ccd_total_usage.size(); i++) {
      uint16_t total_usage = 0;
      uint16_t peak_usage = 0;
      for (uint8_t c = 0; c < logical_cores_per_ccd; c++) {
        uint8_t usage = core_usage[logical_cores_per_ccd * i + c + 1];
        total_usage += usage;
        peak_usage = peak_usage > usage ? peak_usage : usage;
        overall_peak_usage =
            usage > overall_peak_usage ? usage : overall_peak_usage;
      }
      total_usage = static_cast<uint8_t>(
          std::round(total_usage / static_cast<double>(logical_cores_per_ccd)));
      // Update usage Sensors
      ccd_peak_usage[i]->setValue(peak_usage);
      ccd_total_usage[i]->setValue(total_usage);
    }
  } else {
    for (uint8_t c = 1; c < core_usage.size(); c++) {
      uint8_t usage = core_usage[c];
      overall_peak_usage =
          usage > overall_peak_usage ? usage : overall_peak_usage;
    }
  }
  peak_usage->setValue(overall_peak_usage);

  // Power
  uint32_t eax, edx;
  LOG_EC(KernelAccess::readMsr(0xC001029B, &eax, &edx), "Retrieve CPU Power");
  if (IS_LOG_OK) {
    if (prev_energy != 0) {
      std::chrono::steady_clock::time_point time =
          std::chrono::high_resolution_clock::now();
      double delta_time = static_cast<double>(
          std::chrono::duration_cast<std::chrono::microseconds>(
              time - last_update_time)
              .count());
      last_update_time = time;
      double delta_energy = eax - prev_energy;
      power->setValue(
          std::round(delta_energy * energy_unit / (delta_time / 1000000.0)));
    }
    prev_energy = eax;
  }

  // Temperature
  LOG_EC(KernelAccess::writePciRegister(0x00, 0x60, 0x00059800),
         "Send Request to CPU SMU to read CPU Temperature");
  if (IS_LOG_OK) {
    LOG_EC(KernelAccess::readPciRegister(0x00, 0x64, &eax),
           "Read CPU Temperature");
    if (IS_LOG_OK) {
      temperature->setValue(std::round((((eax >> 21) & 0x7FF) / 8.0f) - 49));
    }
  }

  // Frequency data
  std::vector<uint16_t> frequency;
  uint64_t core_mask = 1;
  uint16_t overall_peak_frequency = 0;
  for (uint8_t c = 0; c < physical_cores; c++) {
    LOG_EC(KernelAccess::readMsr(0xC0010293, &eax, &edx, core_mask),
           "Get CPU Core Frequency");
    if (IS_LOG_OK) {
      double freq = ((eax & 0xFF) / (double)((eax >> 8) & 0x3F)) * 200.0;
      frequency.push_back(freq);
      overall_peak_frequency =
          freq > overall_peak_frequency ? freq : overall_peak_frequency;
    } else {
      break;
    }
    core_mask <<= (logical_cores / physical_cores);
  }
  if (IS_LOG_OK) {
    peak_frequency->setValue(overall_peak_frequency);
    //// Frequency Data per CCD
    if (ccd_total_usage.size() > 0) {
      uint8_t physical_cores_per_ccd = logical_cores_per_ccd / 2;
      for (uint8_t i = 0; i < ccd_total_usage.size(); i++) {
        overall_peak_frequency = 0;
        for (uint8_t c = physical_cores_per_ccd * i;
             c < physical_cores_per_ccd * (i + 1); c++) {
          overall_peak_frequency = overall_peak_frequency > frequency[c]
                                       ? overall_peak_frequency
                                       : frequency[c];
        }
        ccd_peak_frequency[i]->setValue(overall_peak_frequency);
      }
    }
  }

  return LOG_END;
}

}  // namespace CDM