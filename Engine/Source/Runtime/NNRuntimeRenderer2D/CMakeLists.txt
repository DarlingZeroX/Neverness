# NNRuntimeRenderer2D — 最小 2D 渲染管线 MVP
# 职责：Sprite Batch / Quad Renderer / Builtin Shader / Camera MVP / Scene Rendering / Render Target
# 依赖：NNRuntimeScene（ECS Query）、NNRuntimeRender/NNRuntimeDiligent（Diligent 后端）
# 着色器：保留 GLSL，由 Diligent 内部编译为 SPIRV

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(NevernessRuntime-Renderer2D SHARED)

target_sources(NevernessRuntime-Renderer2D
    PRIVATE
        # ── 公开头文件 ──
        Include/Renderer2D/Renderer2DExport.h
        Include/Renderer2D/SpriteDrawCommand.h
        Include/Renderer2D/CameraData.h
        Include/Renderer2D/BuiltinShaders.h
        Include/Renderer2D/FramebufferObject.h
        Include/Renderer2D/Renderer2D.h
        Include/Renderer2D/SpriteRenderSystem.h
        Include/Renderer2D/SceneRenderer.h

        # ── 实现文件 ──
        Source/FramebufferObject.cpp
        Source/Renderer2D.cpp
        Source/SpriteRenderSystem.cpp
        Source/SceneRenderer.cpp
)

target_compile_definitions(NevernessRuntime-Renderer2D PRIVATE NN_RUNTIME_RENDERER2D_EXPORT)

target_include_directories(NevernessRuntime-Renderer2D
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
        ${PROJECT_SOURCE_DIR}/Engine/Source/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Renderer2D
        # Rendering 静态库 include 路径
        ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRender
        ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeCore
        ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent
        ${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent/Include
        # Diligent Core（Texture.h 等）
        ${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/DiligentEngine/DiligentCore
)

# 依赖 Scene（ECS Query）
target_link_libraries(NevernessRuntime-Renderer2D PRIVATE NevernessRuntime-Scene)

# 依赖 RenderAssets + Asset（纹理缓存解析、GUID→GPU 资源映射）
target_link_libraries(NevernessRuntime-Renderer2D PRIVATE NevernessRuntime-RenderAssets)
target_link_libraries(NevernessRuntime-Renderer2D PRIVATE NevernessRuntime-Asset)

# 依赖 Diligent 后端（替代 NevernessRuntime-RHI + opengl32）
target_link_libraries(NevernessRuntime-Renderer2D PRIVATE NNRuntimeRender NNRuntimeDiligent)

# 依赖 Core（基础数学类型）
target_link_libraries(NevernessRuntime-Renderer2D PRIVATE NevernessCore-Core)
