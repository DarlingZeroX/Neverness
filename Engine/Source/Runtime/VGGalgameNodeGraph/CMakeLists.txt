# CMakeList.txt: VGGalgameNodeGraph
#
# 中文：Galgame 节点图运行时执行库（DialogueList 数据结构、NodeExecuteFn 实现等），
# 不依赖编辑器 UI；编辑器仅注册节点元数据并引用本库导出的执行函数指针。

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

cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

file(GLOB RUNTIME_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
)

file(GLOB RUNTIME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_library(VGGalgameNodeGraph SHARED ${RUNTIME_HEADERS} ${RUNTIME_SOURCES})

target_compile_definitions(VGGalgameNodeGraph PRIVATE VG_GALGAME_NODEGRAPH_EXPORT)

target_include_directories(VGGalgameNodeGraph PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGGalgameNodeGraph PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# HNGRuntimeCore：节点图 RuntimeContext / ExecResult 等类型
# HCore（PUBLIC）：DialogueListNodeData.h 内联使用 nlohmann::json，消费方包含该头时需可见 HCore 头路径
target_link_libraries(VGGalgameNodeGraph PUBLIC HNGRuntimeCore)
target_link_libraries(VGGalgameNodeGraph PUBLIC HCore)
