project ("X7 Engine")

# KAW
add_library(KAW
	"src/KernelAccess.cpp" 
	"inc/KernelAccess.h"
)
target_include_directories(KAW
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/WinRing0/
)
target_link_libraries(KAW
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/WinRing0/WinRing0x64.lib
    TRL
)
add_custom_command(TARGET KAW POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/WinRing0/WinRing0x64.dll" $<TARGET_FILE_DIR:X7Engine>)
add_custom_command(TARGET KAW POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/WinRing0/WinRing0x64.sys" $<TARGET_FILE_DIR:X7Engine>)