project("X7 Engine")

# WMW
add_library(WMW
	"inc/WMIwatcher.h"
	"src/WMIwatcher.cpp"
	"inc/COMmanager.h" 
	"src/COMmanager.cpp"
	"inc/EventSink.h" 
	"src/EventSink.cpp"
	"inc/PDHhelper.h"
	"src/PDHhelper.cpp"
)
target_include_directories(WMW
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
find_library(USER32 user32)
target_link_libraries(WMW
	USER32
	TRL
)