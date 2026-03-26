# CMakeList.txt: VGGalgameRuntime
#
# 目标：
# - 纯运行时逻辑库（节点执行函数等）
# - 不依赖 Editor

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

file(GLOB RUNTIME_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
)

file(GLOB RUNTIME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_library(VGGalgameRuntime SHARED ${RUNTIME_HEADERS} ${RUNTIME_SOURCES})

target_compile_definitions(VGGalgameRuntime PRIVATE VG_GALGAME_RUNTIME_EXPORT)

target_include_directories(VGGalgameRuntime PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGGalgameRuntime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(VGGalgameRuntime PUBLIC HNGRuntimeCore)

