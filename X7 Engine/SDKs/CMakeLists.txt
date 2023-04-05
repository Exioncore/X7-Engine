project("X7 Engine")

# AMD SDK
add_library(ADLX
    ADLX/ADLXHelper/Windows/Cpp/ADLXHelper.cpp
    ADLX/platform/Windows/WinAPIs.cpp
)
target_include_directories(ADLX
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ADLX/ADLXHelper/Windows/Cpp/
    ${CMAKE_CURRENT_SOURCE_DIR}/ADLX/Include/
)

# NVAPI SDK
add_library(NVAPI INTERFACE)
target_compile_definitions(NVAPI INTERFACE _WIN32)
target_include_directories(NVAPI
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/NVAPI/
)
if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "win32")
    target_link_libraries(NVAPI 
        INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/NVAPI/x86/nvapi.lib
    )
else()
    target_link_libraries(NVAPI 
        INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/NVAPI/amd64/nvapi64.lib
    )
endif()
