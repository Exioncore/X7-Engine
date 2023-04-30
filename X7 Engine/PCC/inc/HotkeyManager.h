#pragma once
// System Includes
#include <functional>
#include <unordered_map>
// 3rd Party Include
// Interal Module Includes
// External Module
#include "EasyLogger.h"

namespace PCC {
class HotkeyManager {
 public:
  HotkeyManager(const HotkeyManager&) = delete;
  HotkeyManager& operator=(const HotkeyManager&) = delete;

  // Methods
  static HotkeyManager& getInstance();
  LOG_RETURN_TYPE addHotkey(uint16_t modifiers, uint16_t key,
                            std::function<void(void)> method);
  void handleHotkeys(int hotkey_id);

 private:
  LOGGER("HotkeyManager");

  int id;
  std::unordered_map<uint32_t, std::function<void(void)>> macro_id_to_hotkey;
  std::unordered_map<int, uint32_t> id_to_macro_id;

  HotkeyManager();
  ~HotkeyManager() = default;
};

}  // namespace PCC