project("X7 Engine")

include(ExternalProject)
include(FetchContent)

# AMD SDK
FetchContent_Declare(
  adlx
  GIT_REPOSITORY "https://github.com/GPUOpen-LibrariesAndSDKs/ADLX.git"
  GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(adlx)
add_library(ADLX
    ${adlx_SOURCE_DIR}/SDK/ADLXHelper/Windows/Cpp/ADLXHelper.cpp
    ${adlx_SOURCE_DIR}/SDK/Platform/Windows/WinAPIs.cpp
)
target_include_directories(ADLX
    PUBLIC
    ${adlx_SOURCE_DIR}/SDK/ADLXHelper/Windows/Cpp/
    ${adlx_SOURCE_DIR}/SDK/Include/
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

# RapidJSON
ExternalProject_Add(
    rapidjson
    PREFIX "rapidjson"
    GIT_REPOSITORY "https://github.com/Tencent/rapidjson.git"
    GIT_TAG "origin/master"
    CMAKE_ARGS
        -DRAPIDJSON_BUILD_TESTS=OFF
        -DRAPIDJSON_BUILD_DOC=OFF
        -DRAPIDJSON_BUILD_EXAMPLES=OFF
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)
ExternalProject_Get_Property(rapidjson source_dir)
add_library(RAPIDJSON INTERFACE)
target_include_directories(RAPIDJSON
    INTERFACE
    ${source_dir}/include
)
