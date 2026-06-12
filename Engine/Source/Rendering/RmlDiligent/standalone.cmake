# 独立构建 RmlDiligent Phase 0 测试
# 使用方法: cmake -B build -G "Visual Studio 17 2022" -A x64 -S .

cmake_minimum_required(VERSION 3.21)
project(RmlDiligentPhase0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UTF-8 编码支持
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/DiligentEngine")

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

add_subdirectory(${DILIGENT_ENGINE_DIR} DiligentEngine)

# 恢复原始 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")

# Phase 0 测试可执行文件
add_executable(RmlDiligentPhase0Test
    Tests/TestPhase0.cpp
)

target_include_directories(RmlDiligentPhase0Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
)

target_link_libraries(RmlDiligentPhase0Test PRIVATE
    DiligentGraphicsEngine
)

# 平台后端接口
if(TARGET DiligentGraphicsEngineD3D12Interface)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE DiligentGraphicsEngineD3D12Interface)
    target_compile_definitions(RmlDiligentPhase0Test PRIVATE D3D12_SUPPORTED=1)
endif()
if(TARGET DiligentGraphicsEngineD3D11Interface)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE DiligentGraphicsEngineD3D11Interface)
    target_compile_definitions(RmlDiligentPhase0Test PRIVATE D3D11_SUPPORTED=1)
endif()
if(TARGET DiligentGraphicsEngineVkInterface)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE DiligentGraphicsEngineVkInterface)
    target_compile_definitions(RmlDiligentPhase0Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(TARGET DiligentGraphicsEngineOpenGLInterface)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE DiligentGraphicsEngineOpenGLInterface)
    target_compile_definitions(RmlDiligentPhase0Test PRIVATE GL_SUPPORTED=1)
endif()

# 平台定义
target_compile_definitions(RmlDiligentPhase0Test PRIVATE PLATFORM_WIN32=1)
