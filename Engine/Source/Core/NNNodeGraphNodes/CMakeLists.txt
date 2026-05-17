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

add_library(HNGRuntimeNodes SHARED)

target_sources(HNGRuntimeNodes
  PRIVATE
    HNGRuntimeNodesConfig.h
    Include/HNGRuntimeNodes.h
    Source/HNGRuntimeNodes.cpp
)

target_compile_definitions(HNGRuntimeNodes PRIVATE H_NG_RUNTIME_NODES_API_EXPORT)

# 需要能包含到 Kernel 根路径（访问 HNGRuntimeCore 头）
target_include_directories(HNGRuntimeNodes PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(HNGRuntimeNodes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(HNGRuntimeNodes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(HNGRuntimeNodes PUBLIC HNGRuntimeCore)
