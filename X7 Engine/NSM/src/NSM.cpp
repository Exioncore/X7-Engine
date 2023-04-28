#include "NSM.h"
// System Includes
#include <format>
#include <iomanip>
#include <sstream>
// 3rd Party Include
// Interal Module Includes
// External Module Includes
#include "PDHhelper.h"

namespace NSM {
LOG_RETURN_TYPE NSM::initialize() {
  LOG_BEGIN;

  std::vector<std::string> names;
  LOG_LR(WMW::PDHhelper::getInstancesInCategory("Network Interface", names),
         "Retrieve Network Interfaces names");
  if (IS_LOG_OK) {
    root_devices_tree = std::make_shared<MDI::SensorTree>("Networks");
    parent_sensor_tree->addSensorTree(root_devices_tree);
    for (auto& name : names) {
      (void)name.pop_back();
      Network network;
      LOG_EC(PdhOpenQuery(NULL, 0, &network.query), "Open PDH query");
      if (IS_LOG_OK) {
        std::string path = "\\Network Interface(" + name + ")\\";
        std::string download_path = path + "Bytes Received/sec";
        LOG_EC(PdhAddCounter(network.query, download_path.c_str(), 0,
                             &network.download_counter),
               "Network Interface Download Counter");
        if (IS_LOG_OK) {
          std::string upload_path = path + "Bytes Sent/sec";
          LOG_EC(PdhAddCounter(network.query, upload_path.c_str(), 0,
                               &network.upload_counter),
                 "Network Interface Upload Counter");
          if (IS_LOG_OK) {
            network.root = std::make_shared<MDI::SensorTree>(name);
            network.download = std::make_shared<MDI::Sensor>(
                "Download", MDI::Sensor::BANDWIDTH, "B/s");
            network.download->setValueStringConverter(NSM::strConverter);
            network.root->addSensor(network.download);
            network.upload = std::make_shared<MDI::Sensor>(
                "Upload", MDI::Sensor::BANDWIDTH, "B/s");
            network.upload->setValueStringConverter(NSM::strConverter);
            network.root->addSensor(network.upload);
          }
        }
      }
      // Store Network
      if (IS_LOG_OK) {
        root_devices_tree->addSensorTree(network.root);
        networks.push_back(std::move(network));
      }
    }
  }

  return LOG_END;
}

LOG_RETURN_TYPE NSM::update() {
  LOG_BEGIN;

  DWORD dwType;
  PDH_FMT_COUNTERVALUE Value;
  for (auto& network : networks) {
    LOG_EC(PdhCollectQueryData(network.query),
           "Collect PDH query data for network");
    if (IS_LOG_OK) {
      LOG_EC(PdhGetFormattedCounterValue(network.download_counter,
                                         PDH_FMT_DOUBLE, &dwType, &Value),
             "Get Network Download speed");
      if (IS_LOG_OK) {
        network.download->setValue(Value.doubleValue);
      }
      LOG_EC(PdhGetFormattedCounterValue(network.upload_counter, PDH_FMT_DOUBLE,
                                         &dwType, &Value),
             "Get Network Upload speed");
      if (IS_LOG_OK) {
        network.upload->setValue(Value.doubleValue);
      }
    }
  }

  return LOG_END;
}

std::string NSM::strConverter(double value, std::string unit) {
  static const std::string units[5] = {"B/s", "KB/s", "MB/s", "GB/s", "TB/s"};

  std::stringstream ss;
  uint8_t i = 0;
  while (value > 1000 && i < 4) {
    value /= 1024;
    i++;
  }
  ss << std::fixed;
  if (value < 10) {
    ss << std::setprecision(3);
  } else if (value < 100) {
    ss << std::setprecision(2);
  } else if (value < 1000) {
    ss << std::setprecision(1);
  } else {
    ss << std::setprecision(0);
  }
  ss << value << " " << units[i];

  return ss.str();
}
}  // namespace NSM