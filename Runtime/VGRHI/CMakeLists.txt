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

# 将源代码添加到此项目的可执行文件。
file(GLOB VGRHI_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/OpenGL/*.cpp"
)
 
file(GLOB VGRHI_HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/OpenGL/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

# 启用预编译头支持
#set(CMAKE_PCH_ENABLED ON)

# 当前cmake C++项目为动态库
add_library (VGRHI SHARED ${VGRHI_SRC_FILES} ${VGRHI_HEADER_FILES})

# 添加宏
target_compile_definitions(VGRHI PRIVATE VG_RHI_API_EXPORT)
#target_compile_definitions(VGRHI PRIVATE GLAD_API_CALL_EXPORT)
#target_compile_definitions(VGRHI PRIVATE GLAD_API_CALL_EXPORT_BUILD)

# 添加头文件搜索路径
target_include_directories(VGRHI PRIVATE ${PROJECT_SOURCE_DIR}/Runtime)
target_include_directories(VGRHI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# 配置预编译头
#target_precompile_headers(VGEngine PRIVATE
#    "Include/pch.h"  # 项目级头文件
#)
 
# 包
#find_package(glad CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# 链接动态库
target_link_libraries(VGRHI PRIVATE SDL3::SDL3)
target_link_libraries(VGRHI PRIVATE HCore)

# 链接OpenGL库
if (APPLE)
    # 在macOS上链接OpenGL框架
    target_link_libraries(VGRHI PRIVATE "-framework OpenGL")
elseif (UNIX)
    # 在Linux上链接OpenGL库
    target_link_libraries(VGRHI PRIVATE GL)
elseif (WIN32)
    # 在Windows上，可能需要根据具体情况设置OpenGL库路径和库名
    target_link_libraries(VGRHI PRIVATE opengl32)
endif ()
