# NNRuntimeRender — 渲染接口层 + RenderGraph 实验模块
# 接口定义 + 纯 C++ 的 RenderGraph 实现（无 Diligent 依赖）

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeRender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeRender STATIC
    Source/RenderGraph/NNRenderGraphBuilder.cpp
    Source/RenderGraph/NNRenderGraph.cpp
    Source/RenderGraph/NNRenderGraphCompiler.cpp
)

# 外部模块使用 #include <NNRuntimeRender/Device/INNRenderDevice.h>
# 文件位于 Rendering/NNRuntimeRender/Device/INNRenderDevice.h
# 所以 Rendering/ 目录需要在搜索路径中
target_include_directories(NNRuntimeRender PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Rendering/ → 可以找到 NNRuntimeRender/Device/...
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeRender/ → 内部可以用 <Device/...>
)

# 依赖 NNRuntimeCore
target_link_libraries(NNRuntimeRender PUBLIC NNRuntimeCore)
