# CMakeList.txt : CMake project for X7 Engine, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (X7_Engine "X7 Engine.cpp" "X7 Engine.h")
find_library(USER32 user32)
target_link_libraries(X7_Engine
    PRIVATE
    GDM
    MDI
    PCC
    WMW
    USER32
)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET X7_Engine PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(GDM)
add_subdirectory(MDI)
add_subdirectory(PCC)
add_subdirectory(SDKs)
add_subdirectory(TRL)
add_subdirectory(WMW)

# TODO: Add tests and install targets if needed.
