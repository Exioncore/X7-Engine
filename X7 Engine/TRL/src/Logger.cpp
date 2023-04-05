#include "Logger.h"
// System Includes
#include <iostream>
// 3rd Party Include
// Interal Module Includes
// External Module Includes

namespace TRL {
uint32_t Logger::log_id = 1;
std::mutex Logger::mtx = std::mutex();

//////////////////
// Logger Class //
//////////////////
Logger::Logger(std::string module_name, bool logging_enabled,
               bool log_on_error_only)
    : module_name(module_name),
      logging_enabled(logging_enabled),
      log_on_error_only(log_on_error_only){};
Logger::Logger(std::string module_name, bool logging_enabled)
    : module_name(module_name),
      logging_enabled(logging_enabled),
      log_on_error_only(true){};
Logger::Logger(std::string module_name)
    : module_name(module_name),
      logging_enabled(true),
      log_on_error_only(true){};

///////////////
//// Methods //
///////////////
void Logger::operator()(std::string function, std::string text) {
  mtx.lock();
  std::cout << "[" << module_name << "," << function << "] " << text << "\n";
  mtx.unlock();
}

LogResult Logger::operator()(const LogResult& log_result, uint32_t flow_link_id,
                             std::string function, std::string text) {
  return this->operator()(log_result.error_code, log_result.id, flow_link_id,
                          function, text);
}

LogResult Logger::operator()(int32_t error_code, uint32_t trace_link_id,
                             uint32_t flow_link_id, std::string function,
                             std::string text) {
  if (logging_enabled) {
    uint32_t trace_id = 0;
    if ((log_on_error_only && error_code != 0) || (!log_on_error_only)) {
      mtx.lock();
      trace_id = log_id;
      std::cout << "[" << module_name << "," << trace_id << "," << trace_link_id
                << "," << flow_link_id << "," << function << "," << error_code
                << "] " << text << "\n";
      log_id += 1;
      mtx.unlock();
    } else {
      trace_id = trace_link_id;
    }
    return {trace_id, error_code};
  }
  return {0, 0};
}

}  // namespace TRL
