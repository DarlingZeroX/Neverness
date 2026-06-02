# Experiments — 顶层 CMakeLists
# 需要在 Engine/Source/CMakeLists.txt 中通过 add_subdirectory("Experiments") 引入

cmake_minimum_required(VERSION 3.21)
project(NNExperiments LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UTF-8 编码支持
if(MSVC)
    add_compile_options(/utf-8)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/DiligentEngine")

# 保存原始 flags
set(CMAKE_CXX_FLAGS_SAVED "${CMAKE_CXX_FLAGS}")

# DiligentCore 配置
set(DILIGENT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_FX OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_DLSS ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_HQ_DSR ON CACHE BOOL "" FORCE)

# 禁用警告转错误（Diligent 第三方代码有警告）
if(MSVC)
    string(REPLACE "/WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif()

add_subdirectory(${DILIGENT_ENGINE_DIR} DiligentEngine)

# 恢复原始 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")

# === 子模块 ===
add_subdirectory(NNRuntimeCore)
add_subdirectory(NNRuntimeRender)
add_subdirectory(NNRuntimeRenderBootstrap)
add_subdirectory(NNRuntimeDiligent)

# === RmlDiligent 实验模块 ===
add_subdirectory(RmlDiligent)

# === Phase 1 测试可执行文件 ===
add_executable(NNPhase1Test NNRuntimeDiligent/Tests/TestTriangleDemo.cpp)

target_include_directories(NNPhase1Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase1Test PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeRender
    NNRuntimeCore
)

# 链接 SDL3
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(NNPhase1Test PRIVATE SDL3::SDL3)

# 链接 NNCore（基础类型）
target_link_libraries(NNPhase1Test PRIVATE NevernessCore-Core)

# 平台定义
target_compile_definitions(NNPhase1Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase1Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase1Test PRIVATE GL_SUPPORTED=1)
endif()

# === Phase 2 测试可执行文件 ===
add_executable(NNPhase2Test NNRuntimeDiligent/Tests/TestResourceCreation.cpp)

target_include_directories(NNPhase2Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase2Test PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNPhase2Test PRIVATE SDL3::SDL3)
target_link_libraries(NNPhase2Test PRIVATE NevernessCore-Core)

target_compile_definitions(NNPhase2Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase2Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase2Test PRIVATE GL_SUPPORTED=1)
endif()

# === Phase 3 test executable ===
add_executable(NNPhase3Test NNRuntimeDiligent/Tests/TestPhase3Triangle.cpp)

target_include_directories(NNPhase3Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase3Test PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNPhase3Test PRIVATE SDL3::SDL3)
target_link_libraries(NNPhase3Test PRIVATE NevernessCore-Core)

target_compile_definitions(NNPhase3Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase3Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase3Test PRIVATE GL_SUPPORTED=1)
endif()
