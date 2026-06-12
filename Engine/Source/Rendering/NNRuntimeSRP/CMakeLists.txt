# NNRuntimeSRP — Scriptable Render Pipeline
# No Diligent dependency - uses NNRuntimeRender interfaces

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeSRP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeSRP STATIC
    Source/Pipeline/NNPipelinePass.cpp
    Source/Pipeline/NNForwardPipeline.cpp
    Source/Context/NNRenderContext.cpp
    Source/Features/NNShadowPass.cpp
    Source/Features/NNForwardPass.cpp
    Source/Features/NNImGuiPass.cpp
)

# External: #include <NNRuntimeSRP/Pipeline/INNRenderPipeline.h>
target_include_directories(NNRuntimeSRP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Rendering/
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeSRP/
)

# Dependencies
target_link_libraries(NNRuntimeSRP PUBLIC
    NNRuntimeRender
    NNRuntimeRenderAssets
    NNRuntimeCore
)
