# NNRuntimeNativeEngineAPI -- C API layer for C# interop

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeNativeEngineAPI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeNativeEngineAPI STATIC
    Source/NNEngineContext.cpp
    Source/NNRenderAPI.cpp
    Source/NNResourceAPI.cpp
)

target_include_directories(NNRuntimeNativeEngineAPI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/API
)

target_link_libraries(NNRuntimeNativeEngineAPI PUBLIC
    NNRuntimeCore
    NNRuntimeRender
)

target_link_libraries(NNRuntimeNativeEngineAPI PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
)

target_compile_definitions(NNRuntimeNativeEngineAPI PRIVATE
    PLATFORM_WIN32=1
)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNRuntimeNativeEngineAPI PRIVATE VULKAN_SUPPORTED=1)
endif()
