#include "Sensor.h"
// System Includes
#include <iomanip>
#include <sstream>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace MDI {
//////////////////
// Sensor Class //
//////////////////
Sensor::Sensor(std::string name, Type type, std::string unit)
    : name(name),
      type(type),
      unit(unit),
      val_converter(Sensor::defaultStrConverter),
      avg_val_converter(Sensor::defaultStrConverter),
      stats{0} {}

///////////////
//// Methods //
///////////////
void Sensor::resetHistory() { stats.reset(); }

void Sensor::setAvgValueStringConverter(
    std::function<std::string(double, std::string)> avg_val_converter) {
  this->avg_val_converter = avg_val_converter;
}

void Sensor::setValueStringConverter(
    std::function<std::string(double, std::string)> val_converter) {
  this->val_converter = val_converter;
}

///////////////
//// Setters //
///////////////
void Sensor::setValue(double value) { stats.update(value); }

///////////////
//// Getters //
///////////////
std::string Sensor::getName() const { return name; }

std::string Sensor::getUnit() const { return unit; }

std::string Sensor::getMaxValueStr() const {
  return val_converter(stats.max_value, unit);
}

std::string Sensor::getAvgValueStr() const {
  return avg_val_converter(stats.average_value, unit);
}

std::string Sensor::getMinValueStr() const {
  return val_converter(stats.min_value, unit);
}

std::string Sensor::getValueStr() const {
  return val_converter(stats.value, unit);
}

double Sensor::getMaxValue() const { return stats.max_value; }

double Sensor::getAvgValue() const { return stats.average_value; }

double Sensor::getMinValue() const { return stats.min_value; }

double Sensor::getValue() const { return stats.value; }

Sensor::Type Sensor::getType() const { return type; }

///////////////
//// Others  //
///////////////
std::string Sensor::defaultStrConverter(double value, std::string unit) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(0) << value << " " << unit;
  return ss.str();
}

//////////////////////////
// Sensor::Value Struct //
//////////////////////////
void Sensor::Value::update(double val) {
  update_count += 1;
  accumulated_values += val;

  max_value = max_value > val ? max_value : val;
  average_value = accumulated_values / update_count;
  min_value = min_value < val ? min_value : val;
  value = val;
}

void Sensor::Value::reset() {
  if (update_count > 0) {
    update_count = 1;
    accumulated_values = max_value = min_value = average_value = value;
  }
}

}  // namespace MDI