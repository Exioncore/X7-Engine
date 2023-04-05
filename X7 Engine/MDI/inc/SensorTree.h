#pragma once
// System Includes
#include <memory>
#include <string>
#include <vector>
// 3rd Party Include
// Interal Module Includes
#include "Sensor.h"
// External Module Includes

namespace MDI {
class SensorTree {
 public:
  SensorTree(std::string name);

  // Methods
  void addSensorTree(std::shared_ptr<SensorTree> sensor_tree);
  void addSensor(std::shared_ptr<Sensor> sensor);
  void resetHistory();

  // Getters
  std::string getName() const;
  std::shared_ptr<SensorTree> getSubTree(uint16_t i) const;
  size_t getNumberOfSubTrees() const;
  std::shared_ptr<Sensor> getSensor(uint16_t i) const;
  size_t getNumberOfSensorsInTree() const;

 private:
  const std::string name;

  std::vector<std::shared_ptr<SensorTree>> sensor_trees;
  std::vector<std::shared_ptr<Sensor>> sensors;
};

}  // namespace MDI