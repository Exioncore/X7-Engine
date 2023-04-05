#pragma once
// System Includes
#include <functional>
#include <string>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace MDI {
class Sensor {
 public:
  enum Type {
    UNKNOWN,
    ROTATIONAL_SPEED,
    FREQUENCY,
    TEMPERATURE,
    POWER,
    BANDWIDTH,
    USAGE,
    VOLTAGE,
    CURRENT,
    FLAG,
    SIZE
  };

  Sensor(std::string name, Type type, std::string unit);

  // Methods
  void resetHistory();
  void setAvgValueStringConverter(
      std::function<std::string(double, std::string)> avg_val_converter);
  void setValueStringConverter(
      std::function<std::string(double, std::string)> val_converter);

  // Setters
  void setValue(double value);

  // Getters
  std::string getName() const;
  std::string getUnit() const;
  std::string getMaxValueStr() const;
  std::string getAvgValueStr() const;
  std::string getMinValueStr() const;
  std::string getValueStr() const;
  double getMaxValue() const;
  double getAvgValue() const;
  double getMinValue() const;
  double getValue() const;
  Type getType() const;

  // Others
  static std::string defaultStrConverter(double value, std::string unit);

 private:
  const std::string name, unit;
  const Type type;
  std::function<std::string(double, std::string)> val_converter,
      avg_val_converter;

  struct Value {
    uint32_t update_count;
    double accumulated_values;
    double max_value, average_value, min_value, value;
    void update(double val);
    void reset();
  } stats;
};

}  // namespace MDI