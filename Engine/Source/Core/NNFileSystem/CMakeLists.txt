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

set(CMAKE_MSVC_PARALLEL_LEVEL 12) # 按你CPU线程数填（6核→12）

add_library(NevernessCore-FileSystem SHARED)

target_sources(NevernessCore-FileSystem
  PRIVATE
    Interface/HFileHeaders.h
    Interface/HFileSystem.h
    Interface/HFileSystemGenerator.h
    Interface/HVirtualFileSystem.h
    Source/HFileSystem.cpp
    Source/HFileSystemGenerator.cpp
)

# 仅对动态库目标定义 CORE_MODULE_EXPORT
target_compile_definitions(NevernessCore-FileSystem PRIVATE H_FILE_SYSTEM_EXPORT)

# 添加头文件搜索路径
target_include_directories(NevernessCore-FileSystem PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(NevernessCore-FileSystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessCore-FileSystem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

target_link_libraries(NevernessCore-FileSystem PUBLIC NevernessCore-Core)
