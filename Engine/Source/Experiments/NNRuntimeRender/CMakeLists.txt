# NNRuntimeRender — 渲染接口层
# 纯接口，不包含任何后端实现

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeRender LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeRender INTERFACE)

# 外部模块使用 #include <NNRuntimeRender/Device/INNRenderDevice.h>
# 文件位于 Experiments/NNRuntimeRender/Device/INNRenderDevice.h
# 所以 Experiments/ 目录需要在搜索路径中
target_include_directories(NNRuntimeRender INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Experiments/ → 可以找到 NNRuntimeRender/Device/...
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeRender/ → 内部可以用 <Device/...>
)

# 依赖 NNRuntimeCore
target_link_libraries(NNRuntimeRender INTERFACE NNRuntimeCore)
