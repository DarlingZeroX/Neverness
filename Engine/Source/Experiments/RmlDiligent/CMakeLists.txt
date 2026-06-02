# RmlDiligent — RmlUi + Diligent Engine 集成实验模块

cmake_minimum_required(VERSION 3.21)
project(RmlDiligent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UTF-8 编码支持
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/DiligentEngine")

# 保存原始 flags
set(CMAKE_CXX_FLAGS_SAVED "${CMAKE_CXX_FLAGS}")

# DiligentCore 配置
set(DILIGENT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_FX OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_DLSS ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_HQ_DSR ON CACHE BOOL "" FORCE)

# 禁用警告转错误
if(MSVC)
    string(REPLACE "/WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif()

# ATL stub — 通过编译器 flags 添加 include 路径（让 try_compile 也能找到）
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I\"${CMAKE_CURRENT_SOURCE_DIR}/Stubs\"" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /I\"${CMAKE_CURRENT_SOURCE_DIR}/Stubs\"" CACHE STRING "" FORCE)
endif()

# 跳过 ATL 检查（系统已有 ATL）
set(SKIP_ATL_CHECK TRUE CACHE BOOL "" FORCE)

# 使用 NVAPI stub（避免下载 GitHub 依赖）
set(DILIGENT_NVAPI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Stubs/nvapi" CACHE PATH "" FORCE)

# 禁用 SuperResolution（避免下载 GitHub 依赖）
set(DILIGENT_NO_DSR ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_DLSS ON CACHE BOOL "" FORCE)

# 编译 NVAPI stub 库
add_library(nvapi_stub STATIC Stubs/nvapi/nvapi_stub.c)
target_include_directories(nvapi_stub PUBLIC Stubs/nvapi)

# 先添加 Diligent Engine 子目录（定义 Diligent-* targets）
add_subdirectory(${DILIGENT_ENGINE_DIR} DiligentEngine)

# 查找 SDL3
find_package(SDL3 CONFIG REQUIRED)

# Phase 0 测试（无窗口，只验证 Shader + PSO）
add_executable(RmlDiligentPhase0Test Tests/TestPhase0.cpp)
target_include_directories(RmlDiligentPhase0Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    ${DILIGENT_ENGINE_DIR}/DiligentCore
)
target_link_libraries(RmlDiligentPhase0Test PRIVATE Diligent-GraphicsEngine)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase0Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
)

# Phase 1 测试（SDL3 窗口 + RmlUi Demo）
add_executable(RmlDiligentPhase1Test Tests/TestPhase1.cpp)
target_include_directories(RmlDiligentPhase1Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    ${DILIGENT_ENGINE_DIR}/DiligentCore
)
target_link_libraries(RmlDiligentPhase1Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase1Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase1Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase1Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
)

# 恢复原始 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")
