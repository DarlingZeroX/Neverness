cmake_minimum_required(VERSION 3.20)

set(MODULE_NAME "NevernessRuntime-RenderAssets")

file(GLOB_RECURSE MODULE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
)

file(GLOB_RECURSE MODULE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cpp"
)

add_library(${MODULE_NAME} SHARED
    ${MODULE_HEADERS}
    ${MODULE_SOURCES}
)

target_compile_definitions(${MODULE_NAME} PRIVATE NN_RUNTIME_RENDER_ASSETS_EXPORT NN_RUNTIME_RENDER_ASSETS_DYNAMIC)

target_include_directories(${MODULE_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Private"
        "${PROJECT_SOURCE_DIR}/Engine/Source/Runtime"
        "${PROJECT_SOURCE_DIR}/Engine/Source/Core"
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# 开启utf-8编码支持（中文注释）
if (MSVC)
    target_compile_options(${MODULE_NAME} PRIVATE /utf-8)
endif()

# Experiments 静态库 include 路径（NNRuntimeRender 接口层）
target_include_directories(${MODULE_NAME}
    PRIVATE
        "${PROJECT_SOURCE_DIR}/Engine/Source/Experiments/NNRuntimeRender"
        "${PROJECT_SOURCE_DIR}/Engine/Source/Experiments/NNRuntimeCore"
)

target_link_libraries(${MODULE_NAME}
    PUBLIC
        NevernessRuntime-Asset         # NNAssetHandleT, NNAssetManager
        NevernessRuntime-NativeEngineAPI  # EngineTypes.h (NNAssetManager 依赖)
    PRIVATE
        NevernessCore-Core             # NN::Ref (shared_ptr)
        NNRuntimeRender                # INNRenderDevice, INNTexture 等接口
)

set_target_properties(${MODULE_NAME} PROPERTIES
    FOLDER "Engine/Runtime"
)
