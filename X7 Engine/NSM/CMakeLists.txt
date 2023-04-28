project ("X7 Engine")

# NSM
add_library(NSM
    "inc/NSM.h"
    "src/NSM.cpp"
)
target_include_directories(NSM
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
target_link_libraries(NSM
    MDI
    TRL
    WMW
)
