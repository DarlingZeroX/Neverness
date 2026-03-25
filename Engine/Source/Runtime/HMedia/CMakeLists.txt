# 设置 C++ 标准为 C++17
#cmake_minimum_required(VERSION 3.10)
#project(HMedia LANGUAGES CXX)  # 明确指定为C++项目

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

#开启MSVC多线程编译	# windows 并行编译选项
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->) #高版本已被废弃，但是低版本的Gm会影响并行
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

# 添加宏
add_definitions(-DH_MEDIA_API_EXPORT) 

# 将源代码添加到此项目的可执行文件。
file(GLOB H_MEDIA_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Audio/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Video/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Common/*.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/SDL/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/FFmpeg/*.cpp"
)
 
file(GLOB H_MEDIA_HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Audio/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Video/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Common/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/SDL/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/FFmpeg/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

# 启用预编译头支持
#set(CMAKE_PCH_ENABLED ON)

# 当前cmake C++项目为动态库
add_library (HMedia SHARED ${H_MEDIA_SRC_FILES} ${H_MEDIA_HEADER_FILES})

# 添加头文件搜索路径
target_include_directories(HMedia PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(HMedia PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 配置预编译头
#target_precompile_headers(VGEngine PRIVATE
#    "Include/pch.h"  # 项目级头文件
#)
 
# 包
#find_package(SDL3 CONFIG REQUIRED)
 
# FFmpeg
find_package(FFMPEG REQUIRED)
target_include_directories(HMedia PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(HMedia PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(HMedia PRIVATE ${FFMPEG_LIBRARIES})

# 链接动态库
#target_link_libraries(HMedia PRIVATE SDL3::SDL3)
target_link_libraries(HMedia PRIVATE HCore)
