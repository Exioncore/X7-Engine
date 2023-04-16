# CMakeList.txt : CMake project for X7 Engine, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (X7Engine "X7 Engine.cpp" "X7 Engine.h")
find_library(USER32 user32)
target_link_libraries(X7Engine
    PRIVATE
    CDM
    GDM
    MDI
    PCC
    WMW
    KAW
    USER32
)
#add_custom_command(TARGET X7Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/X7 Engine/SDKs/WinRing0/WinRing0x64.dll" $<TARGET_FILE_DIR:X7Engine>)
#add_custom_command(TARGET X7Engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/X7 Engine/SDKs/WinRing0/WinRing0x64.sys" $<TARGET_FILE_DIR:X7Engine>)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET X7Engine PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(CDM)
add_subdirectory(GDM)
add_subdirectory(KAW)
add_subdirectory(MDI)
add_subdirectory(PCC)
add_subdirectory(SDKs)
add_subdirectory(TRL)
add_subdirectory(WMW)

# TODO: Add tests and install targets if needed.
