# NNRuntimeDiligent — Diligent Engine 后端实现

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeDiligent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/DiligentEngine")

# Diligent Core 路径
set(DILIGENT_CORE_DIR "${DILIGENT_ENGINE_DIR}/DiligentCore")

# ImGui 核心：不再编译 Diligent 捆绑的 ImGui，改用 NNRuntimeImGui 的版本（1.92.3）
# Diligent ImGui 后端源码保留，但 include 路径指向 NNRuntimeImGui
set(DILIGENT_IMGUI_ROOT "${DILIGENT_ENGINE_DIR}/DiligentTools/ThirdParty/imgui")
set(DILIGENT_IMGUI_DIR "${DILIGENT_ENGINE_DIR}/DiligentTools/Imgui")

# NNRuntimeImGui 路径（权威 ImGui 版本 1.92.3）
set(RUNTIME_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../Runtime/NNRuntimeImGui")

add_library(NNRuntimeDiligent STATIC
    # NNRuntimeDiligent core
    Source/Device/NNDiligentDevice.cpp
    Source/Device/NNDiligentViewportSurface.cpp
    Source/Command/NNDiligentCommandList.cpp
    Source/Resources/NNDiligentBuffer.cpp
    Source/Resources/NNDiligentTexture.cpp
    Source/Resources/NNDiligentSampler.cpp
    Source/Resources/NNDiligentRenderTarget.cpp
    Source/Pipeline/NNDiligentShader.cpp
    Source/Pipeline/NNDiligentPipelineState.cpp
    Source/Command/NNDiligentRenderQueue.cpp
    # ImGui adapter
    Source/ImGui/NNDiligentImGuiRenderer.cpp
    # ImGui 核心源码由 NNRuntimeImGui 编译，此处不再重复编译
    # Diligent ImGui 后端源码（使用 NNRuntimeImGui 的 ImGui 头文件）
    ${DILIGENT_IMGUI_DIR}/src/ImGuiImplDiligent.cpp
    ${DILIGENT_IMGUI_DIR}/src/ImGuiImplSDL3.cpp
    ${DILIGENT_IMGUI_DIR}/src/ImGuiDiligentRenderer.cpp
)

# 外部使用 #include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
target_include_directories(NNRuntimeDiligent PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Rendering/
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeDiligent/
)

# 依赖接口层
target_link_libraries(NNRuntimeDiligent PUBLIC NNRuntimeRender)

# SDL3 (for window handle extraction)
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(NNRuntimeDiligent PRIVATE SDL3::SDL3)

# FreeType (for ImGui font rendering)
find_package(Freetype REQUIRED)
target_link_libraries(NNRuntimeDiligent PRIVATE Freetype::Freetype)

# DiligentCore 头文件
target_include_directories(NNRuntimeDiligent PUBLIC
    ${DILIGENT_CORE_DIR}
)

# ImGui 头文件 — 使用 NNRuntimeImGui 的版本（1.92.3），而非 Diligent 捆绑的 1.92.1
# 关键：NNRuntimeImGui 的 imgui.h 路径必须在 DILIGENT_IMGUI_ROOT 之前
#       这样 #include "imgui.h" 优先找到 1.92.3 版本
target_include_directories(NNRuntimeDiligent PUBLIC
    ${RUNTIME_IMGUI_DIR}/Include/Imgui   # imgui.h (v1.92.3，权威版本)
    ${RUNTIME_IMGUI_DIR}/Include         # imconfig.h
    ${DILIGENT_IMGUI_DIR}/interface      # ImGuiImplDiligent.hpp, ImGuiImplSDL3.hpp
    ${DILIGENT_ENGINE_DIR}               # for DiligentTools/Imgui relative includes
)
# PRIVATE：DILIGENT_IMGUI_ROOT 仅供 ImGuiImplSDL3.cpp 的 #include "backends/imgui_impl_sdl3.h" 使用
# 放在 PUBLIC 路径之后，确保 imgui.h 优先从 NNRuntimeImGui 找到
target_include_directories(NNRuntimeDiligent PRIVATE
    ${DILIGENT_IMGUI_ROOT}               # backends/imgui_impl_sdl3.h
)

# ImGui: enable FreeType + 配置宏（与 NNRuntimeImGui 的 imconfig.h 对齐）
target_compile_definitions(NNRuntimeDiligent PRIVATE
    IMGUI_ENABLE_FREETYPE
    IMGUI_API=           # Diligent 后端不需要 dllexport，由 NNRuntimeImGui 的 imconfig.h 控制
)

# Diligent 内部模块头文件 (ImGuiDiligentRenderer/ImGuiImplSDL3 依赖)
target_include_directories(NNRuntimeDiligent PRIVATE
    ${DILIGENT_CORE_DIR}/Primitives/interface
    ${DILIGENT_CORE_DIR}/Common/interface
    ${DILIGENT_CORE_DIR}/Graphics/GraphicsTools/interface
    ${DILIGENT_CORE_DIR}/Graphics/GraphicsAccessories/interface
    ${DILIGENT_CORE_DIR}/Graphics/GraphicsEngine/interface
)

# 链接 Diligent 后端库（所有已启用的后端）
if(VULKAN_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineVk-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC VULKAN_SUPPORTED=1)
endif()
if(D3D12_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineD3D12-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC D3D12_SUPPORTED=1)
endif()
if(D3D11_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineD3D11-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC D3D11_SUPPORTED=1)
endif()
if(GL_SUPPORTED OR GLES_SUPPORTED)
    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineOpenGL-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC GL_SUPPORTED=1)
endif()
if(METAL_SUPPORTED)
    target_compile_definitions(NNRuntimeDiligent PUBLIC METAL_SUPPORTED=1)
endif()

# 平台定义（PUBLIC — 传播给所有链接 NNRuntimeDiligent 的 target）
if(WIN32)
    target_compile_definitions(NNRuntimeDiligent PUBLIC PLATFORM_WIN32=1)

    # D3D12 和 Vulkan 后端在 Windows 平台上可用
    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineD3D12-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC D3D12_SUPPORTED=1)

    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineVk-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC VULKAN_SUPPORTED=1)
elseif(APPLE)
    target_compile_definitions(NNRuntimeDiligent PUBLIC PLATFORM_MACOS=1)

    # Metal 后端在 macOS 平台上可用
    target_compile_definitions(NNRuntimeDiligent PUBLIC METAL_SUPPORTED=1)
elseif(UNIX)
    target_compile_definitions(NNRuntimeDiligent PUBLIC PLATFORM_LINUX=1)

    # Vulkan 后端在 Linux 平台上可用
    target_link_libraries(NNRuntimeDiligent PUBLIC Diligent-GraphicsEngineVk-static)
    target_compile_definitions(NNRuntimeDiligent PUBLIC VULKAN_SUPPORTED=1)
endif()

target_compile_definitions(NNRuntimeDiligent PUBLIC NN_WITH_DILIGENT)

# 构建后自动复制 Diligent 后端 DLL 到输出目录（D3D12、Vulkan 等）
# 函数由 DiligentCore/BuildTools/CMake/BuildUtils.cmake 提供
if(COMMAND copy_engine_dlls)
    copy_engine_dlls(NNRuntimeDiligent)
endif()
