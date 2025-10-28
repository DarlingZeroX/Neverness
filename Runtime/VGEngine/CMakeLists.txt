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
#if (MSVC)
#    add_compile_options(/bigobj)
#endif()

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

# 将源代码添加到此项目的可执行文件。
file(GLOB ENGINE_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Audio/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Sprite/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Asset/Accessor/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Engine/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Engine/Manager/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Galgame/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/OpenGL/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Project/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Render/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Resource/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Resource/Video/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Scene/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils/*.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/Lua/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/Sol/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/Sol/Elements/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UI/Lua/Elements/*.cpp"
)

file(GLOB ENGINE_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Animation/Interface/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Animation/Audio/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Animation/Sprite/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Asset/Accessor/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Core/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Engine/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Engine/Manager/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Galgame/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Game/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Graphics/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Graphics/Interface/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Graphics/OpenGL/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Interface/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Project/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua/Interface/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Render/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Resource/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Resource/Video/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Scene/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Utils/*.h"

	"${CMAKE_CURRENT_SOURCE_DIR}/Include/UI/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/UI/Lua/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/UI/Sol/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/UI/Lua/Elements/*.h"
)

# 启用预编译头支持
set(CMAKE_PCH_ENABLED ON)

# 当前cmake C++项目为动态库
add_library (VGEngine SHARED ${ENGINE_SRC_FILES} ${ENGINE_HEADER_FILES})

# 添加头文件搜索路径
target_include_directories(VGEngine PRIVATE ${PROJECT_SOURCE_DIR}/Runtime)
target_include_directories(VGEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGEngine PUBLIC "${PROJECT_SOURCE_DIR}/Runtime/VGLua/Include")

# 配置预编译头
target_precompile_headers(VGEngine PRIVATE
    "Include/pch.h"  # 项目级头文件
)

# 包
#find_package(glad CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(RmlUi REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# FFmpeg
find_package(FFMPEG REQUIRED)
target_include_directories(VGEngine PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(VGEngine PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(VGEngine PRIVATE ${FFMPEG_LIBRARIES})

# 链接动态库
target_link_libraries(VGEngine PRIVATE VGLua)
target_link_libraries(VGEngine PRIVATE SDL3::SDL3)
target_link_libraries(VGEngine PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)
target_link_libraries(VGEngine PRIVATE HCore)
target_link_libraries(VGEngine PRIVATE VGPackage)
target_link_libraries(VGEngine PRIVATE VGImgui)
target_link_libraries(VGEngine PRIVATE RmlUi::RmlUi)

# 链接OpenGL库
if (APPLE)
    # 在macOS上链接OpenGL框架
    target_link_libraries(VGEngine PRIVATE "-framework OpenGL")
elseif (UNIX)
    # 在Linux上链接OpenGL库
    target_link_libraries(VGEngine PRIVATE GL)
elseif (WIN32)
    # 在Windows上，可能需要根据具体情况设置OpenGL库路径和库名
    target_link_libraries(VGEngine PRIVATE opengl32)
endif ()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VGEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
