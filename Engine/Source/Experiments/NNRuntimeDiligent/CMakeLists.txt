# NNRuntimeDiligent — Diligent Engine 后端实现

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeDiligent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/DiligentEngine")

# Diligent Core 路径
set(DILIGENT_CORE_DIR "${DILIGENT_ENGINE_DIR}/DiligentCore")

# ImGui core sources — use Diligent's bundled ImGui (clean, no dllimport)
set(DILIGENT_IMGUI_ROOT "${DILIGENT_ENGINE_DIR}/DiligentTools/ThirdParty/imgui")
set(IMGUI_CORE_DIR "${DILIGENT_IMGUI_ROOT}")

# Diligent ImGui backend sources (from ThirdParty/DiligentEngine)
set(DILIGENT_IMGUI_DIR "${DILIGENT_ENGINE_DIR}/DiligentTools/Imgui")

add_library(NNRuntimeDiligent STATIC
    # NNRuntimeDiligent core
    Source/Device/NNDiligentDevice.cpp
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
    # ImGui core sources (Diligent bundled ImGui)
    ${IMGUI_CORE_DIR}/imgui.cpp
    ${IMGUI_CORE_DIR}/imgui_draw.cpp
    ${IMGUI_CORE_DIR}/imgui_tables.cpp
    ${IMGUI_CORE_DIR}/imgui_widgets.cpp
    ${IMGUI_CORE_DIR}/imgui_demo.cpp
    ${IMGUI_CORE_DIR}/misc/freetype/imgui_freetype.cpp
    # Diligent ImGui backend sources
    ${DILIGENT_IMGUI_DIR}/src/ImGuiImplDiligent.cpp
    ${DILIGENT_IMGUI_DIR}/src/ImGuiImplSDL3.cpp
    ${DILIGENT_IMGUI_DIR}/src/ImGuiDiligentRenderer.cpp
    # ImGui SDL3 platform backend (official)
    ${DILIGENT_IMGUI_ROOT}/backends/imgui_impl_sdl3.cpp
)

# 外部使用 #include <NNRuntimeDiligent/Device/NNDiligentDevice.h>
target_include_directories(NNRuntimeDiligent PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Experiments/
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

# ImGui 头文件 (Diligent bundled ImGui)
target_include_directories(NNRuntimeDiligent PUBLIC
    ${IMGUI_CORE_DIR}              # imgui.h from Diligent's ThirdParty/imgui
    ${IMGUI_CORE_DIR}/backends      # imgui_impl_sdl3.h
    ${DILIGENT_IMGUI_DIR}/interface # ImGuiImplDiligent.hpp, ImGuiImplSDL3.hpp
    ${DILIGENT_ENGINE_DIR}          # for DiligentTools/Imgui relative includes
)

# ImGui: enable FreeType
target_compile_definitions(NNRuntimeDiligent PRIVATE
    IMGUI_ENABLE_FREETYPE
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
