#include "HotkeyManager.h"
// System Includes
#include <iostream>
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
// External Module Includes

namespace PCC {
//////////////////////////
// HotkeyManager Class //
//////////////////////////
HotkeyManager::HotkeyManager() : id(1000) {}

///////////////
//// Methods //
///////////////
HotkeyManager& HotkeyManager::getInstance() {
  static HotkeyManager instance;
  return instance;
}

LOG_RETURN_TYPE HotkeyManager::addHotkey(uint16_t modifiers, uint16_t key,
                                         std::function<void(void)> method) {
  LOG_BEGIN;

  uint32_t macro_id = (((uint32_t)modifiers) << 16) | key;
  LOG_EC(macro_id_to_hotkey.count(macro_id) != 0, "Hotkey macro id check");
  if (IS_LOG_OK) {
    LOG_EC(id_to_macro_id.count(id) != 0, "Hotkey id check");
  }
  if (IS_LOG_OK) {
    LOG_EC(RegisterHotKey(NULL, id, modifiers, key) == 0, "RegisterHotKey");
  }
  if (IS_LOG_OK) {
    macro_id_to_hotkey.emplace(macro_id, method);
    id_to_macro_id.emplace(id, macro_id);
    id += 1;
  }

  return LOG_END;
}

void HotkeyManager::handleHotkeys(int hotkey_id) {
  if (id_to_macro_id.count(hotkey_id) > 0) {
    uint32_t macro_id = id_to_macro_id.at(hotkey_id);
    if (macro_id_to_hotkey.count(macro_id) > 0) {
      macro_id_to_hotkey.at(macro_id)();
    }
  }
}

}  // namespace PCC
