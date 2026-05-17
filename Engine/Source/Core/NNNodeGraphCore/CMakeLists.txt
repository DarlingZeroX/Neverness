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

add_library(HNGRuntimeCore SHARED)

target_sources(HNGRuntimeCore
  PRIVATE
    Include/Events.h
    Include/ExpressionEvaluator.h
    Include/NodeMeta.h
    Include/NodeRegistry.h
    Include/RuntimeContext.h
    Include/RuntimeGraph.h
    Interface/RuntimeCore.h
    Interface/Types.h
    Interface/Value.h
    Source/ExpressionEvaluator.cpp
    Source/RuntimeContext.cpp
    Source/RuntimeGraph.cpp
)

# 添加宏
target_compile_definitions(HNGRuntimeCore PRIVATE H_NODE_GRAPH_API_EXPORT)

# 添加头文件搜索路径
target_include_directories(HNGRuntimeCore PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(HNGRuntimeCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(HNGRuntimeCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

# 寻找CMake包
find_package(SDL3 CONFIG REQUIRED)

# 链接库
target_link_libraries(HNGRuntimeCore PRIVATE SDL3::SDL3)
target_link_libraries(HNGRuntimeCore PRIVATE NevernessCore-Core)
