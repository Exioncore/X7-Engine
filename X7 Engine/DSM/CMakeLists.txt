project("X7 Engine")

# DSM
add_library(DSM
    "inc/DSM.h"
    "src/DSM.cpp"
)
target_include_directories(DSM
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
target_link_libraries(DSM
    MDI
    TRL
    WMW
)
