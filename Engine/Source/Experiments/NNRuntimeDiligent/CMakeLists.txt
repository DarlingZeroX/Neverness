# NNRuntimeDiligent — Diligent Engine 后端实现

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeDiligent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Diligent Core 路径
set(DILIGENT_CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/DiligentEngine/DiligentCore")

add_library(NNRuntimeDiligent STATIC
    Source/Device/NNDiligentDevice.cpp
    Source/Command/NNDiligentCommandList.cpp
    Source/Resources/NNDiligentBuffer.cpp
    Source/Resources/NNDiligentTexture.cpp
    Source/Resources/NNDiligentSampler.cpp
    Source/Resources/NNDiligentRenderTarget.cpp
    Source/Pipeline/NNDiligentShader.cpp
    Source/Pipeline/NNDiligentPipelineState.cpp
    Source/Command/NNDiligentRenderQueue.cpp
)

# 外部使用 #include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
target_include_directories(NNRuntimeDiligent PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Experiments/
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeDiligent/
)

# 依赖接口层
target_link_libraries(NNRuntimeDiligent PUBLIC NNRuntimeRender)

# DiligentCore 头文件
target_include_directories(NNRuntimeDiligent PUBLIC
    ${DILIGENT_CORE_DIR}
)

# 链接 Diligent 后端库（所有已启用的后端）
if(VULKAN_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PRIVATE Diligent-GraphicsEngineVk-static)
    target_compile_definitions(NNRuntimeDiligent PRIVATE VULKAN_SUPPORTED=1)
endif()
if(D3D12_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PRIVATE Diligent-GraphicsEngineD3D12-static)
    target_compile_definitions(NNRuntimeDiligent PRIVATE D3D12_SUPPORTED=1)
endif()
if(D3D11_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PRIVATE Diligent-GraphicsEngineD3D11-static)
    target_compile_definitions(NNRuntimeDiligent PRIVATE D3D11_SUPPORTED=1)
endif()
if(GL_SUPPORTED OR GLES_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PRIVATE Diligent-GraphicsEngineOpenGL-static)
    target_compile_definitions(NNRuntimeDiligent PRIVATE GL_SUPPORTED=1)
endif()
if(METAL_SUPPORTED)
    target_compile_definitions(NNRuntimeDiligent PRIVATE METAL_SUPPORTED=1)
endif()

# 平台定义
if(WIN32)
    target_compile_definitions(NNRuntimeDiligent PRIVATE PLATFORM_WIN32=1)
elseif(APPLE)
    target_compile_definitions(NNRuntimeDiligent PRIVATE PLATFORM_MACOS=1)
elseif(UNIX)
    target_compile_definitions(NNRuntimeDiligent PRIVATE PLATFORM_LINUX=1)
endif()

target_compile_definitions(NNRuntimeDiligent PUBLIC NN_WITH_DILIGENT)
