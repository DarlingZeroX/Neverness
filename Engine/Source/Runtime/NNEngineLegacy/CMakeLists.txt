# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

#开启MSVC多线程编译	# windows 并行编译选项
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->) #高版本已被废弃，但是低版本的Gm会影响并行
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

# 大对象模版文件需要开启
if (MSVC)
    add_compile_options(/bigobj)
endif()

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()
 
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 添加宏
add_definitions(-DENGINE_API_EXPORT) 
#add_definitions(-DIMGUI_IMPL_OPENGL_ES3)

include(${CMAKE_CURRENT_SOURCE_DIR}/NNEngineLegacySources.cmake)

set(CMAKE_PCH_ENABLED ON)

add_library(NevernessRuntime-EngineLegacy SHARED)
target_sources(NevernessRuntime-EngineLegacy PRIVATE ${NNEngineLegacy_SOURCES})

# 添加头文件搜索路径
target_include_directories(NevernessRuntime-EngineLegacy PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-EngineLegacy PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-EngineLegacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-EngineLegacy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-EngineLegacy PUBLIC ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeLua/Include)

# 配置预编译头
target_precompile_headers(NevernessRuntime-EngineLegacy PRIVATE
    "Include/pch.h"  # 项目级头文件
)
 
# 包
#find_package(glad CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(RmlUi REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# FFmpeg
#find_package(FFMPEG REQUIRED)
#target_include_directories(VGEngine PUBLIC ${FFMPEG_INCLUDE_DIRS})
#target_link_directories(VGEngine PUBLIC ${FFMPEG_LIBRARY_DIRS})
#target_link_libraries(VGEngine PUBLIC ${FFMPEG_LIBRARIES})

# 链接动态库
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC SDL3::SDL3)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC RmlUi::RmlUi)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessCore-Core)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessCore-PlatformCore)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessCore-FileSystem)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessCore-MediaCore)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-Lua)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-RHI)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-VFS)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-Pak)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-ImGui)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-Core)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-Application)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-Asset)
target_link_libraries(NevernessRuntime-EngineLegacy PUBLIC NevernessRuntime-RmlUI)

# 链接OpenGL库
if (APPLE)
    # 在macOS上链接OpenGL框架
    target_link_libraries(NevernessRuntime-EngineLegacy PRIVATE "-framework OpenGL")
elseif (UNIX)
    # 在Linux上链接OpenGL库
    target_link_libraries(NevernessRuntime-EngineLegacy PRIVATE GL)
elseif (WIN32)
    # 在Windows上，可能需要根据具体情况设置OpenGL库路径和库名
    target_link_libraries(NevernessRuntime-EngineLegacy PRIVATE opengl32)
endif ()

#if (CMAKE_VERSION VERSION_GREATER 3.12)
#  set_property(TARGET VGEngine PROPERTY CXX_STANDARD 20)
#endif()

# TODO: 如有需要，请添加测试并安装目标。
