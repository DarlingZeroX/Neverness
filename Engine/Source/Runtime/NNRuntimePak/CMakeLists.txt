# CMakeList.txt: VGEditor CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
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

add_library(NevernessRuntime-Pak SHARED)

target_sources(NevernessRuntime-Pak
  PRIVATE
    Include/Config.h
    Include/PackageFile.h
    Include/PackageFileSystem.h
    Include/PakCrc32.h
    Include/PakFormat.h
    Include/PakReader.h
    Include/PakWriter.h
    Source/PakCrc32.cpp
    Source/PakFormat.cpp
    Source/PakReader.cpp
    Source/PakWriter.cpp
)

# 添加宏
target_compile_definitions(NevernessRuntime-Pak PRIVATE VG_PACKAGE_API_EXPORT)

# 添加头文件搜索路径
target_include_directories(NevernessRuntime-Pak PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Pak PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-Pak PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 寻找CMake包
find_package(SDL3 CONFIG REQUIRED)

# 链接库
target_link_libraries(NevernessRuntime-Pak PRIVATE SDL3::SDL3)
target_link_libraries(NevernessRuntime-Pak PRIVATE NevernessCore-Core)
