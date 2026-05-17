# 设置 C++ 标准为 C++17
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

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/NNPlatformCoreSources.cmake)

add_library(NevernessCore-PlatformCore STATIC)
target_sources(NevernessCore-PlatformCore PRIVATE ${NNPlatformCore_SOURCES})

# 仅对动态库目标定义 CORE_MODULE_EXPORT
#target_compile_definitions(NevernessCore-PlatformCore PRIVATE H_CORE_PLATFORM_EXPORT)

# 添加头文件搜索路径
target_include_directories(NevernessCore-PlatformCore PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(NevernessCore-PlatformCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessCore-PlatformCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

find_package(SDL3 CONFIG REQUIRED)
# 公共头（如 Include/SDL3/SDL3Window.h）直接 #include <SDL3/...>，依赖方必须获得 SDL 头路径与链接接口
target_link_libraries(NevernessCore-PlatformCore PUBLIC NevernessCore-Core SDL3::SDL3)
