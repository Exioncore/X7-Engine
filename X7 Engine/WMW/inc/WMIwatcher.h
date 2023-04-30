#pragma once
// System Includes
#include <functional>
#include <memory>
#include <unordered_map>
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
#include "EventSink.h"
// External Module
#include "EasyLogger.h"

namespace WMW {
class WMIwatcher {
 public:
  WMIwatcher(const WMIwatcher&) = delete;
  WMIwatcher& operator=(const WMIwatcher&) = delete;

  // Methods
  LOG_RETURN_TYPE initialize();
  LOG_RETURN_TYPE deInitialize();
  LOG_RETURN_TYPE registerCallback(
      uint16_t& id, std::string query,
      std::function<void(IWbemClassObject*)> callback);
  LOG_RETURN_TYPE deRegisterCallback(uint16_t id);

  static WMIwatcher& getInstance();

 private:
  LOGGER("WMIwatcher");

  bool initialized;
  uint16_t id;
  std::unordered_map<uint16_t, std::unique_ptr<EventSink>>
      registered_callbacks;

  IWbemLocator* pLoc;
  IWbemServices* pSvc;

  WMIwatcher();
  ~WMIwatcher() = default;
};

}  // namespace WMW