# Rendering — 顶层 CMakeLists
# 需要在 Engine/Source/CMakeLists.txt 中通过 add_subdirectory("Rendering") 引入

cmake_minimum_required(VERSION 3.21)
project(NNRendering LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
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

# 禁止 FetchContent 下载（所有依赖必须本地存在，避免清缓存后重新下载）
set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "" FORCE)

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

# NVAPI stub — 位于 ThirdParty/nvapi_stub，Diligent D3D12 的基础依赖
set(DILIGENT_NVAPI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/nvapi_stub" CACHE PATH "" FORCE)

# NVAPI stub 必须先于 Diligent 编译（配置阶段就需要 lib 文件存在）
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/nvapi_stub" "${CMAKE_BINARY_DIR}/ThirdParty/nvapi_stub")

# Diligent Engine — 二进制目录放在主构建树外面
# 这样 Delete Cache and Reconfigure 不会触发 Diligent 重新编译
set(DILIGENT_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Build/ThirdParty/DiligentBin")
add_subdirectory(${DILIGENT_ENGINE_DIR} ${DILIGENT_BIN_DIR})

# 恢复原始 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")

# === 子模块 ===
# add_subdirectory(NNRuntimeCore)
add_subdirectory(NNRuntimeRender)
add_subdirectory(NNRuntimeRenderAssets)
add_subdirectory(NNRuntimeSRP)
add_subdirectory(NNRuntimeRenderBootstrap)
add_subdirectory(NNRuntimeDiligent)
add_subdirectory(NNRuntimeNativeEngineAPI)

# === RmlDiligent 渲染后端 + 测试 ===
add_subdirectory(RmlDiligent)
add_subdirectory(RmlDiligentTest)

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

# NevernessCore-Core lib path (built in main build dir)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../Build/lib")
    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../Build/lib/${CMAKE_BUILD_TYPE}")
endif()
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

# === Phase 4 test executable ===
add_executable(NNPhase4Test NNRuntimeNativeEngineAPI/Tests/TestNativeAPI.cpp)

target_include_directories(NNPhase4Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase4Test PRIVATE
    NNRuntimeNativeEngineAPI
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNPhase4Test PRIVATE SDL3::SDL3)
target_link_libraries(NNPhase4Test PRIVATE NevernessCore-Core)

target_compile_definitions(NNPhase4Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase4Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase4Test PRIVATE GL_SUPPORTED=1)
endif()

# === Phase 5 test executable (RenderGraph) ===
add_executable(NNPhase5Test NNRuntimeDiligent/Tests/TestRenderGraph.cpp)

target_include_directories(NNPhase5Test PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase5Test PRIVATE
    NNRuntimeRender
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeCore
    SDL3::SDL3
    NevernessCore-Core
)

target_compile_definitions(NNPhase5Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase5Test PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNPhase5Test PRIVATE GL_SUPPORTED=1)
endif()

# === Phase 4.3 test executable (ImGui Demo) ===
add_executable(NNImGuiTest NNRuntimeDiligent/Tests/TestImGuiDemo.cpp)

target_include_directories(NNImGuiTest PRIVATE
    ${DILIGENT_ENGINE_DIR}/DiligentCore
    ${DILIGENT_ENGINE_DIR}/DiligentTools/Imgui/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNImGuiTest PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNImGuiTest PRIVATE SDL3::SDL3)
target_link_libraries(NNImGuiTest PRIVATE NevernessCore-Core)

target_compile_definitions(NNImGuiTest PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNImGuiTest PRIVATE VULKAN_SUPPORTED=1)
endif()
if(GL_SUPPORTED)
    target_compile_definitions(NNImGuiTest PRIVATE GL_SUPPORTED=1)
endif()

# === Phase 6 test executable (Material System) ===
add_executable(NNPhase6Test NNRuntimeRenderAssets/Tests/TestMaterial.cpp)

target_include_directories(NNPhase6Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase6Test PRIVATE
    NNRuntimeRenderAssets
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNPhase6Test PRIVATE NevernessCore-Core)

target_compile_definitions(NNPhase6Test PRIVATE PLATFORM_WIN32=1)

# === Phase 7 test executable (SRP) ===
add_executable(NNPhase7Test NNRuntimeSRP/Tests/TestPipeline.cpp)

target_include_directories(NNPhase7Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase7Test PRIVATE
    NNRuntimeSRP
    NNRuntimeRenderAssets
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNPhase7Test PRIVATE NevernessCore-Core)

target_compile_definitions(NNPhase7Test PRIVATE PLATFORM_WIN32=1)

# === Phase 8 test executable (C# Interop) ===
add_executable(NNPhase8Test NNRuntimeNativeEngineAPI/Tests/TestMaterialAPI.cpp)

target_include_directories(NNPhase8Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNPhase8Test PRIVATE
    NNRuntimeNativeEngineAPI
    NNRuntimeSRP
    NNRuntimeRenderAssets
    NNRuntimeRender
    NNRuntimeCore
)

target_link_libraries(NNPhase8Test PRIVATE
    NNRuntimeDiligent
    NNRuntimeRenderBootstrap
    SDL3::SDL3
    NevernessCore-Core
)

target_compile_definitions(NNPhase8Test PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNPhase8Test PRIVATE VULKAN_SUPPORTED=1)
endif()

# === Scene Render (3D cube + screenshot) ===
add_executable(NNSceneRender NNRuntimeDiligent/Tests/TestSceneRender.cpp)

target_include_directories(NNSceneRender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRenderBootstrap/Include
)

target_link_libraries(NNSceneRender PRIVATE
    NNRuntimeDiligent
    NNRuntimeRender
    NNRuntimeCore
    SDL3::SDL3
    NevernessCore-Core
)

target_compile_definitions(NNSceneRender PRIVATE PLATFORM_WIN32=1)
if(VULKAN_SUPPORTED)
    target_compile_definitions(NNSceneRender PRIVATE VULKAN_SUPPORTED=1)
endif()
