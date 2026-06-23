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

# === RmlDiligent 渲染后端 ===
add_subdirectory(RmlDiligent)

# === 测试代码 ===
# 测试文件已移至 Test/ 目录，通常不编译
# 如需编译测试，取消注释以下内容或使用 cmake -DNN_BUILD_TESTS=ON
# add_subdirectory(Test/RmlDiligent)
# add_executable(NNPhase1Test Test/Diligent/TestTriangleDemo.cpp)
# add_executable(NNPhase2Test Test/Diligent/TestResourceCreation.cpp)
# add_executable(NNPhase3Test Test/Diligent/TestPhase3Triangle.cpp)
# add_executable(NNPhase4Test Test/NativeAPI/TestNativeAPI.cpp)
# add_executable(NNPhase5Test Test/Diligent/TestRenderGraph.cpp)
# add_executable(NNImGuiTest Test/Diligent/TestImGuiDemo.cpp)
# add_executable(NNPhase6Test Test/RenderAssets/TestMaterial.cpp)
# add_executable(NNPhase7Test Test/SRP/TestPipeline.cpp)
# add_executable(NNPhase8Test Test/NativeAPI/TestMaterialAPI.cpp)
# add_executable(NNSceneRender Test/Diligent/TestSceneRender.cpp)
