project ("X7 Engine")

# CDM
add_library(CDM
    "inc/CDM.h"
    "src/CDM.cpp"
    "inc/CPU.h"
    "src/CPU.cpp"
    "inc/AMD_ZEN.h"
    "src/AMD_ZEN.cpp"
)
target_include_directories(CDM
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
target_link_libraries(CDM
    MDI
    KAW
    TRL
)
