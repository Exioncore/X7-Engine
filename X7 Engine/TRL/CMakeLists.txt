project("X7 Engine")

# TRL
add_library(TRL
	"inc/Logger.h"
	"src/Logger.cpp"
	"inc/LogResult.h"
	"inc/EasyLogger.h"
)
target_include_directories(TRL
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)