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

# 添加宏
add_definitions(-DVGCORE_DYNAMIC) 
add_definitions(-DVG_CORE_API_EXPORT) 
#add_definitions(-DIMGUI_IMPL_OPENGL_ES3)

# 将源代码添加到此项目的可执行文件。
file(GLOB ENGINE_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Data/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils/*.cpp"
)
 
file(GLOB ENGINE_HEADER_FILES 
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Core/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Data/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Interface/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Utils/*.h"
)

# 启用预编译头支持
#set(CMAKE_PCH_ENABLED ON)

# 当前cmake C++项目为动态库
add_library (NevernessRuntime-Core SHARED ${ENGINE_SRC_FILES} ${ENGINE_HEADER_FILES})

# 添加头文件搜索路径
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Kernel)
target_include_directories(NevernessRuntime-Core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/VGLua/Include)

# 配置预编译头
target_precompile_headers(NevernessRuntime-Core PRIVATE
    "Include/pch.h"  # 项目级头文件
)
 
# 包
#find_package(glad CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(RmlUi REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

# 链接动态库
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-Lua)
# Window.h 等公共头暴露 SDL 类型；PRIVATE 不会传给仅链接 NevernessRuntime-Core 的目标，会导致 C1083
target_link_libraries(NevernessRuntime-Core PUBLIC SDL3::SDL3)
target_link_libraries(NevernessRuntime-Core PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessKernel-Kernel)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessKernel-PlatformCore)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessKernel-FileSystem)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-ImGui)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-RHI)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-Pak)


