project ("X7 Engine")

# MDI
add_library(MDI
    "inc/Monitor.h"
    "inc/SensorTree.h"
    "inc/Sensor.h"
    "src/SensorTree.cpp"
    "src/Sensor.cpp"
)
target_include_directories(MDI
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
target_link_libraries(MDI
    TRL
)