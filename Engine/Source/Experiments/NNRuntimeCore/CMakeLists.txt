# NNRuntimeCore — 核心层
# 包含: INNObject, NNRef, HandleRegistry

cmake_minimum_required(VERSION 3.20)
project(NNRuntimeCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NNRuntimeCore STATIC
    Source/Handle/NNObjectHandleRegistry.cpp
)

# 外部使用 #include <NNRuntimeCore/NNObject.h>
# 文件位于 Experiments/NNRuntimeCore/NNObject.h
target_include_directories(NNRuntimeCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..    # Experiments/ → NNRuntimeCore/NNObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}       # NNRuntimeCore/ → 内部可以用 <NNObject.h>
)

# 依赖 NevernessCore-Core（HCoreTypes.h, Math）
target_link_libraries(NNRuntimeCore PUBLIC NevernessCore-Core)
