#include "X7 Engine.h"
// System Includes
#include <iostream>
#include <thread>
// 3rd Party Include
#include <windows.h>
// Interal Module Includes
// External Module Includes
#include "GDM.h"
#include "HotkeyManager.h"
#include "ProcessMonitor.h"
#include "COMmanager.h"

using namespace std;

void printSensorsTree(std::shared_ptr<MDI::SensorTree> tree, uint16_t depth) {
  std::cout << std::string(depth, '\t') << "> " << tree->getName() << std::endl;
  // Enter SubTree
  for (uint16_t i = 0; i < tree->getNumberOfSubTrees(); i++) {
    printSensorsTree(tree->getSubTree(i), depth + 1);
  }
  // Print Sensor
  for (uint16_t i = 0; i < tree->getNumberOfSensorsInTree(); i++) {
    auto sensor = tree->getSensor(i);
    std::cout << std::string(depth + 1, '\t') << sensor->getName() << ": "
              << sensor->getValueStr() << std::endl;
  }
  std::cout << std::string(depth, '\t') << "< " << tree->getName() << std::endl;
}

void setAffinityToCCD0() {
  std::cout << "setAffinityToCCD0()" << std::endl;
  PCC::ProcessMonitor::getInstance().setForegroundProcessAffinity(
      0b1111111111111111);
}

void setAffinityToCCD1() {
  std::cout << "setAffinityToCCD1()" << std::endl;
  PCC::ProcessMonitor::getInstance().setForegroundProcessAffinity(
      (static_cast<uint64_t>(0b1111111111111111) << 16));
}

void resetAffinity() {
  std::cout << "resetAffinity()" << std::endl;
  PCC::ProcessMonitor::getInstance().setForegroundProcessAffinity(4294967296 -
                                                                  1);
}

void message_handler() {
  WMW::COMmanager com_manager;
  com_manager.initialize();

  PCC::HotkeyManager::getInstance().addHotkey(MOD_ALT | MOD_CONTROL | MOD_SHIFT,
                                              VK_F1, setAffinityToCCD0);
  PCC::HotkeyManager::getInstance().addHotkey(MOD_ALT | MOD_CONTROL | MOD_SHIFT,
                                              VK_F2, setAffinityToCCD1);
  PCC::HotkeyManager::getInstance().addHotkey(MOD_ALT | MOD_CONTROL | MOD_SHIFT,
                                              VK_F3, resetAffinity);

  (void)PCC::ProcessMonitor::getInstance();
  MSG msg;

  while (GetMessage(&msg, NULL, 0, 0) != -1) {
    if (msg.message == WM_HOTKEY) {
      PCC::HotkeyManager::getInstance().handleHotkeys(
          static_cast<int>(msg.wParam));
    } else {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
}

int main() {
  std::thread t_msg_handler(message_handler);



  std::shared_ptr<MDI::SensorTree> root =
      std::make_shared<MDI::SensorTree>("X7 Engine");
  GDM::GDM gdm(root);
  gdm.initialize();

  while (true) {
    gdm.update();
    //printSensorsTree(root, 0);
    _sleep(1000);
  }
  t_msg_handler.join();

  return 0;
}
