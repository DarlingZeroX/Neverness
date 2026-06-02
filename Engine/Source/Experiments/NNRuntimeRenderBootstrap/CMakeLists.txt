# NNRuntimeRenderBootstrap — 后端引导

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeRenderBootstrap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeRenderBootstrap STATIC
    Source/NNRenderBootstrap.cpp
)

target_include_directories(NNRuntimeRenderBootstrap PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NNRuntimeRenderBootstrap PUBLIC NNRuntimeRender)

# 平台定义（Diligent 需要）
if(WIN32)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE PLATFORM_WIN32=1)
elseif(APPLE)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE PLATFORM_MACOS=1)
elseif(UNIX)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE PLATFORM_LINUX=1)
endif()

# 后端支持定义
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE VULKAN_SUPPORTED=1)
endif()
if(D3D12_SUPPORTED)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE D3D12_SUPPORTED=1)
endif()
if(D3D11_SUPPORTED)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE D3D11_SUPPORTED=1)
endif()
if(GL_SUPPORTED OR GLES_SUPPORTED)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE GL_SUPPORTED=1)
endif()
if(METAL_SUPPORTED)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE METAL_SUPPORTED=1)
endif()

if(NN_WITH_DILIGENT)
    target_compile_definitions(NNRuntimeRenderBootstrap PRIVATE NN_WITH_DILIGENT)
    target_link_libraries(NNRuntimeRenderBootstrap PRIVATE NNRuntimeDiligent)
endif()
