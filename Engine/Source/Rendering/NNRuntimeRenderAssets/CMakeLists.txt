# NNRuntimeRenderAssets — Material + Shader Asset + PSO Cache
# No Diligent dependency - uses NNRuntimeRender interfaces only

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeRenderAssets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeRenderAssets STATIC
    Source/Material/NNMaterial.cpp
    Source/Material/NNMaterialInstance.cpp
    Source/Shader/NNShaderAsset.cpp
    Source/Shader/NNShaderVariant.cpp
    Source/Shader/NNShaderSource.cpp
    Source/Cache/NNPipelineCache.cpp
    Source/Cache/NNAssetRegistry.cpp
)

# External: #include <NNRuntimeRenderAssets/Material/NNMaterial.h>
target_include_directories(NNRuntimeRenderAssets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Rendering/
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeRenderAssets/
)

# Dependencies: interfaces + core
target_link_libraries(NNRuntimeRenderAssets PUBLIC
    NNRuntimeRender
    NNRuntimeCore
)
