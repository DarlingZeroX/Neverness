# NNRuntimeRmlui — RmlUI 渲染模块
# Phase 3: 替换 GL3 后端为 RmlDiligent（Diligent Engine）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRmluiSources.cmake)

# 移除 GL3 渲染后端源码（被 RmlDiligent 替代）
list(REMOVE_ITEM NNRuntimeRmlui_SOURCES
    "Source/Rml/RenderInterfaceGL3SDL.cpp"
    "Source/Rml/RmlUi_Renderer_GL3.cpp"
    "Include/Rml/RenderInterfaceGL3SDL.h"
    "Include/Rml/RmlUi_Renderer_GL3.h"
)

# 添加 RmlDiligent 渲染源码（只取渲染部分，不取 Platform 层避免冲突）
set(RML_DILIGENT_DIR "${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/RmlDiligent")
list(APPEND NNRuntimeRmlui_SOURCES
    ${RML_DILIGENT_DIR}/Private/RmlDiligentRenderInterface.cpp
    ${RML_DILIGENT_DIR}/Renderer/RmlDiligentRenderTargetPool.cpp
    ${RML_DILIGENT_DIR}/Renderer/RmlDiligentLayerStack.cpp
    ${RML_DILIGENT_DIR}/Renderer/RmlDiligentSrbCache.cpp
)

add_library(NevernessRuntime-RmlUI SHARED)
target_sources(NevernessRuntime-RmlUI PRIVATE ${NNRuntimeRmlui_SOURCES})

target_compile_definitions(NevernessRuntime-RmlUI PRIVATE VG_UI_EXPORT)

# ── 依赖包查找（必须在 include 路径之前，确保 RmlUi PUBLIC include 生效）──
find_package(SDL3 CONFIG REQUIRED)
#find_package(SDL3_image CONFIG REQUIRED)
find_package(RmlUi CONFIG REQUIRED)

# ── 链接依赖（顺序重要：RmlUi 先于 RmlDiligent 源码）──
target_link_libraries(NevernessRuntime-RmlUI PRIVATE
    # RmlUi（提供 RmlUi/Core/*.h PUBLIC include 路径）
    RmlUi::RmlUi
    SDL3::SDL3
    #$<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
    # NNRuntimeRender + NNRuntimeDiligent（替代 NevernessRuntime-RHI）
    NNRuntimeRender
    NNRuntimeDiligent
    # Neverness 内部依赖
    NevernessCore-Core
    NevernessCore-PlatformCore
    NevernessCore-FileSystem
    NevernessCore-MediaCore
    NevernessRuntime-Lua
    NevernessRuntime-Core
    NevernessRuntime-VFS
    NevernessRuntime-Scene
)

# ── Include 路径 ──
target_include_directories(NevernessRuntime-RmlUI PRIVATE
    ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
    ${PROJECT_SOURCE_DIR}/Engine/Source/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Interface
)
target_include_directories(NevernessRuntime-RmlUI PUBLIC
    ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeLua/Include
)

# RmlDiligent include 路径（在 RmlUi include 之后，避免覆盖）
target_include_directories(NevernessRuntime-RmlUI PRIVATE
    ${RML_DILIGENT_DIR}/Public
    ${RML_DILIGENT_DIR}/Private
    ${RML_DILIGENT_DIR}/Renderer
    ${RML_DILIGENT_DIR}/Shaders
    # Rendering 核心 include
    ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRender
    ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeCore
    ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent
    ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent/Include
    # Diligent Core（RmlDiligent 需要）
    ${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/DiligentEngine/DiligentCore
)
