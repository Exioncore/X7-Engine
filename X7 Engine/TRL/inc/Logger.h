#pragma once
// System Includes
#include <mutex>
// 3rd Party Include
// Interal Module Includes
#include <LogResult.h>
// External Module Includes

namespace TRL {
class Logger {
 public:
  static constexpr int32_t NO_LINK = 0;
  static constexpr int32_t OK = 0;

  Logger(std::string module_name);

  // Methods
  void operator()(std::string function, std::string text);
  //// Method to log result of a method returning LogResult
  LogResult operator()(const LogResult& log_result, uint32_t flow_link_id,
                       std::string function, std::string text);
  //// Method to log result of a method returning an error_code
  LogResult operator()(int32_t error_code, uint32_t trace_link_id,
                       uint32_t flow_link_id, std::string function,
                       std::string text);

 private:
  static uint32_t log_id;
  static std::mutex mtx;

  std::string module_name;
};

}  // namespace TRL