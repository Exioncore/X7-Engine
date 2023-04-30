#include "DSM.h"
// System Includes
#include <algorithm>
#include <format>
#include <iomanip>
#include <sstream>
// 3rd Party Include
// Interal Module Includes
// External Module Includes
#include "PDHhelper.h"

namespace DSM {
LOG_RETURN_TYPE DSM::initialize() {
  LOG_BEGIN;

  // Retrieve all PhysicalDisk instances
  std::vector<std::string> instances;
  LOG_LR(WMW::PDHhelper::getInstancesInCategory("PhysicalDisk", instances),
         "Retrieve Drives names");

  // Loop through PhysicalDisks
  if (IS_LOG_OK) {
    root_devices_tree = std::make_shared<MDI::SensorTree>("Drives");
    std::sort(instances.begin(), instances.end(),
              [](const std::string& first, const std::string& second) {
                return (first < second);
              });
    for (auto& instance : instances) {
      (void)instance.pop_back();
      if (instance == "_Total") {
        continue;
      }

      Drive drive;
      LOG_EC(PdhOpenQuery(NULL, 0, &drive.query), "Open PDH query");
      if (IS_LOG_OK) {
        std::string path = "\\PhysicalDisk(" + instance + ")\\";
        std::string activity_path = path + "% Disk Time";
        LOG_EC(PdhAddCounter(drive.query, activity_path.c_str(), 0,
                             &drive.activity_counter),
               "Drive Activity Counter");
        if (IS_LOG_OK) {
          std::string read_path = path + "Disk Read Bytes/sec";
          LOG_EC(PdhAddCounter(drive.query, read_path.c_str(), 0,
                               &drive.read_counter),
                 "Drive Read Counter");
          if (IS_LOG_OK) {
            std::string write_path = path + "Disk Write Bytes/sec";
            LOG_EC(PdhAddCounter(drive.query, write_path.c_str(), 0,
                                 &drive.write_counter),
                   "Drive Write Counter");
            if (IS_LOG_OK) {
              drive.root = std::make_shared<MDI::SensorTree>(instance);
              drive.activity = std::make_shared<MDI::Sensor>(
                  "Activity", MDI::Sensor::USAGE, "%");
              drive.root->addSensor(drive.activity);
              drive.read = std::make_shared<MDI::Sensor>(
                  "Read", MDI::Sensor::BANDWIDTH, "MB/s");
              drive.read->setValueStringConverter(DSM::strConverter);
              drive.root->addSensor(drive.read);
              drive.write = std::make_shared<MDI::Sensor>(
                  "Write", MDI::Sensor::BANDWIDTH, "MB/s");
              drive.write->setValueStringConverter(DSM::strConverter);
              drive.root->addSensor(drive.write);
              // Store Drive
              root_devices_tree->addSensorTree(drive.root);
              drives.push_back(std::move(drive));
            }
          }
        }
      }
    }
    // Add Drives SensorTree to root
    if (IS_LOG_OK) {
      parent_sensor_tree->addSensorTree(root_devices_tree);
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE DSM::update() {
  LOG_BEGIN;

  DWORD dwType;
  PDH_FMT_COUNTERVALUE Value;
  for (auto& drive : drives) {
    LOG_EC(PdhCollectQueryData(drive.query),
           "Collect PDH query data for drive");
    if (IS_LOG_OK) {
      LOG_EC(PdhGetFormattedCounterValue(drive.activity_counter,
                                         PDH_FMT_DOUBLE, &dwType, &Value),
             "Get Drive activity");
      if (IS_LOG_OK) {
        drive.activity->setValue(Value.doubleValue);
      }
      LOG_EC(PdhGetFormattedCounterValue(drive.read_counter, PDH_FMT_DOUBLE,
                                         &dwType, &Value),
             "Get Drive Read speed");
      if (IS_LOG_OK) {
        drive.read->setValue(Value.doubleValue);
      }
      LOG_EC(PdhGetFormattedCounterValue(drive.write_counter, PDH_FMT_DOUBLE,
                                         &dwType, &Value),
             "Get Drive Write speed");
      if (IS_LOG_OK) {
        drive.write->setValue(Value.doubleValue);
      }
    }
  }

  return LOG_END;
}

std::string DSM::strConverter(double value, std::string unit) {
  static const std::string units[5] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s"};

  std::stringstream ss;
  uint8_t i = 0;
  while (value > 1000 && i < 4) {
    value /= 1024;
    i++;
  }
  ss << std::fixed;
  if (value < 10) {
    ss << std::setprecision(2);
  } else if (value < 100) {
    ss << std::setprecision(1);
  } else {
    ss << std::setprecision(0);
  }
  ss << value << " " << units[i];

  return ss.str();
}

}  // namespace DSM