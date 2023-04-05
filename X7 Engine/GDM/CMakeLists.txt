project("X7 Engine")

# GDM
add_library(GDM
    "inc/GDM.h"
    "src/GDM.cpp"
)
target_include_directories(GDM
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
target_link_libraries(GDM
    GPU_AMD
    GPU_NVIDIA
    MDI
    TRL
    "dxgi.lib"
)

# AMD GPU monitoring
add_library(GPU_AMD
    "inc/GPUMetrics.h"
    "inc/GPU_AMD.h"
    "src/GPU_AMD.cpp"
)
target_include_directories(GPU_AMD
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/ADLX/ADLXHelper/Windows/Cpp/
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/ADLX/Include/
)
target_link_libraries(GPU_AMD
    ADLX
    MDI
    TRL
)

# NVIDIA GPU monitoring
find_package(CUDAToolkit)
add_library(GPU_NVIDIA
    "inc/GPUMetrics.h"
    "inc/GPU_NVIDIA.h"
    "src/GPU_NVIDIA.cpp"
)
target_include_directories(GPU_NVIDIA
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../SDKs/NVAPI/
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/
)
target_link_libraries(GPU_NVIDIA
    CUDA::nvml    
    NVAPI
    MDI
    TRL
)
