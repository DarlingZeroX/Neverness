# RmlDiligent — RmlUi + Diligent Engine 渲染后端（静态库）
# 测试/示例可执行文件在 RmlDiligentTest 模块中

cmake_minimum_required(VERSION 3.21)
project(RmlDiligent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/DiligentEngine")

# DiligentCore 配置（如父级已配置则跳过）
set(DILIGENT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_FX OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_DLSS ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_HQ_DSR ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_DSR ON CACHE BOOL "" FORCE)

if(MSVC)
    string(REPLACE "/WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif()

set(SKIP_ATL_CHECK TRUE CACHE BOOL "" FORCE)

# Diligent Engine — 仅在父级未添加时才添加
if(NOT TARGET Diligent-BasicPlatform)
    add_subdirectory(${DILIGENT_ENGINE_DIR} DiligentEngine)
endif()

find_package(RmlUi CONFIG REQUIRED)

# ── RmlDiligent 核心渲染源码 ──
set(RML_DILIGENT_RENDER_SOURCES
    Private/RmlDiligentRenderInterface.cpp
    Renderer/RmlDiligentRenderTargetPool.cpp
    Renderer/RmlDiligentLayerStack.cpp
    Renderer/RmlDiligentSrbCache.cpp
)

add_library(RmlDiligent STATIC ${RML_DILIGENT_RENDER_SOURCES})

target_include_directories(RmlDiligent PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    ${DILIGENT_ENGINE_DIR}/DiligentCore
)

target_link_libraries(RmlDiligent PUBLIC
    Diligent-GraphicsEngine
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligent PUBLIC Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligent PUBLIC Diligent-GraphicsEngineD3D11-static)
endif()

target_compile_definitions(RmlDiligent PUBLIC
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
)
