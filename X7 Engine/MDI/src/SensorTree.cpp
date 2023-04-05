#include "SensorTree.h"

namespace MDI {
///////////////////////
// SensorTree Class //
//////////////////////
SensorTree::SensorTree(std::string name) : name(name) {}

///////////////
//// Methods //
///////////////
void SensorTree::addSensorTree(std::shared_ptr<SensorTree> sensor_tree) {
  sensor_trees.push_back(sensor_tree);
}

void SensorTree::addSensor(std::shared_ptr<Sensor> sensor) {
  sensors.push_back(sensor);
}

void SensorTree::resetHistory() {
  // Reset SubTree's
  for (auto& sensor_tree : sensor_trees) {
    sensor_tree->resetHistory();
  }
  // Reset Tree Sensor's
  for (auto& sensor : sensors) {
    sensor->resetHistory();
  }
}

///////////////
//// Getters //
///////////////
std::string SensorTree::getName() const { return name; }

std::shared_ptr<SensorTree> SensorTree::getSubTree(uint16_t i) const {
  return sensor_trees.at(i);
}

size_t SensorTree::getNumberOfSubTrees() const { return sensor_trees.size(); }

std::shared_ptr<Sensor> SensorTree::getSensor(uint16_t i) const {
  return sensors.at(i);
}

size_t SensorTree::getNumberOfSensorsInTree() const { return sensors.size(); }

}  // namespace MDI