#pragma once
// System Includes
#include <vector>
// 3rd Party Include
#pragma comment(lib, "pdh.lib")
#include <Pdh.h>
// Interal Module Includes
// External Module
#include "EasyLogger.h"

namespace WMW {
class PDHhelper {
 public:
  // Methods
  static LOG_RETURN_TYPE getInstancesInCategory(
      std::string category, std::vector<std::string>& instances);

 private:
  LOGGER("PDHhelper", true, true);
};

}  // namespace WMW