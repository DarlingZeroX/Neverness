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

# 将源代码添加到此项目的可执行文件。
file(GLOB HNODEGRAPH_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB HNODEGRAPH_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
)

add_library (HNGRuntimeCore SHARED ${HNODEGRAPH_HEADER_FILES} ${HNODEGRAPH_SRC_FILES})

# 添加宏
target_compile_definitions(HNGRuntimeCore PRIVATE H_NODE_GRAPH_API_EXPORT)

# 添加头文件搜索路径
target_include_directories(HNGRuntimeCore PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(HNGRuntimeCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(HNGRuntimeCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

# 寻找CMake包
find_package(SDL3 CONFIG REQUIRED)

# 链接库
target_link_libraries(HNGRuntimeCore PRIVATE SDL3::SDL3)
target_link_libraries(HNGRuntimeCore PRIVATE HCore)

