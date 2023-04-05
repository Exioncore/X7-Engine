project("X7 Engine")

# PCC
add_library(PCC
	"src/ProcessMonitor.cpp"
	"inc/ProcessMonitor.h"
	"inc/ProcessData.h"
	"src/ProcessData.cpp"
	"inc/HotkeyManager.h"
	"src/HotkeyManager.cpp"
)
target_include_directories(PCC
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
find_library(USER32 user32)
target_link_libraries(PCC
	USER32
	TRL
)