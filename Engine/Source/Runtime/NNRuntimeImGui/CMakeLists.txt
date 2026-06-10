# NNRuntimeImGui — ImGui 核心 + 扩展模块
# Phase 4: 移除 OpenGL3 后端，改用 Diligent 后端（通过 NNRuntimeDiligent）
# ImGui 版本 1.92.3（权威版本，C# 也依赖此版本）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeImGuiSources.cmake)

# Phase 4: OpenGL3 后端源码保留编译（NNRuntimeApplication 的 ImGuiLayer 仍依赖 ImGui_ImplOpenGL3_* 符号）
# Phase 5 迁移 Application 后再移除全部 OpenGL3 后端
# imgui_impl_opengl3_loader.h 必须保留（imgui_impl_opengl3.cpp 依赖它）

add_library(NevernessRuntime-ImGui SHARED)
target_sources(NevernessRuntime-ImGui PRIVATE ${NNRuntimeImGui_SOURCES})

target_compile_definitions(NevernessRuntime-ImGui PRIVATE IMGUI_EXPORT)

target_include_directories(NevernessRuntime-ImGui PRIVATE
    ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
    ${PROJECT_SOURCE_DIR}/Engine/Source/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

find_package(SDL3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries(NevernessRuntime-ImGui PRIVATE
    Freetype::Freetype
    SDL3::SDL3
    NevernessCore-Core
    NevernessCore-PlatformCore
    # Diligent 后端（替代 OpenGL3）
    # NNRuntimeDiligent 提供 ImGuiImplDiligent + ImGuiImplSDL3
    # ImGui 核心由本模块编译（1.92.3），NNRuntimeDiligent 使用本模块的头文件
    NNRuntimeDiligent
    NNRuntimeRender
)

# Diligent include 路径（NNDiligentImGuiRenderer 需要）
target_include_directories(NevernessRuntime-ImGui PRIVATE
    ${PROJECT_SOURCE_DIR}/Engine/Source/Experiments/NNRuntimeDiligent
    ${PROJECT_SOURCE_DIR}/Engine/Source/Experiments/NNRuntimeDiligent/Include
    ${PROJECT_SOURCE_DIR}/Engine/Source/Experiments/NNRuntimeRender
    ${PROJECT_SOURCE_DIR}/Engine/Source/Experiments/NNRuntimeCore
    ${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/DiligentEngine/DiligentCore
    ${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/DiligentEngine/DiligentTools/Imgui/interface
)

# 不再链接 opengl32
