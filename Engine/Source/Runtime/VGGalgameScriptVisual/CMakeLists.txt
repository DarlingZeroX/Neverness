# CMakeList.txt: VGGalgameRuntime
#
# 目标：
# - 纯运行时逻辑库（节点执行函数等）
# - 不依赖 Editor

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 大对象模版文件需要开启
if (MSVC)
    add_compile_options(/bigobj)
endif()

if (MSVC)
    add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

#开启MSVC多线程编译	# windows 并行编译选项
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->) #高版本已被废弃，但是低版本的Gm会影响并行
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

file(GLOB RUNTIME_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/RuntimeSystem/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/VisualSequence/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB RUNTIME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/RuntimeSystem/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/VisualSequence/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface/*.cpp"
)

add_library(VGGalgameScriptVisual SHARED ${RUNTIME_HEADERS} ${RUNTIME_SOURCES})

target_compile_definitions(VGGalgameScriptVisual PRIVATE VG_GALGAME_SCRIPT_VISUAL_EXPORT)

target_include_directories(VGGalgameScriptVisual PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGGalgameScriptVisual PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGGalgameScriptVisual PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

target_link_libraries(VGGalgameScriptVisual PUBLIC VGGalgameCore)
target_link_libraries(VGGalgameScriptVisual PUBLIC VGGalgameRuntime)
