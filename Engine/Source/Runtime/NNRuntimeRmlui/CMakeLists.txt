# CMakeList.txt: GalEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
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

# 将源代码添加到此项目的可执行文件。
file(GLOB HCORE_INTERFACE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB HCORE_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Sol/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Rml/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua/Elements/*.h"
)

file(GLOB HCORE_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Sol/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Rml/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Sol/Elements/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/Elements/*.cpp"
)

# 库
add_library (NevernessRuntime-RmlUI SHARED ${HCORE_INTERFACE_FILES} ${HCORE_HEADER_FILES} ${HCORE_SRC_FILES})

# 仅对动态库目标定义 CORE_MODULE_EXPORT
target_compile_definitions(NevernessRuntime-RmlUI PRIVATE VG_UI_EXPORT)

# 添加头文件搜索路径
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Kernel)
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-RmlUI PUBLIC ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeLua/Include)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE SDL3::SDL3)

find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)

find_package(RmlUi CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE RmlUi::RmlUi)
 
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessKernel-Kernel)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessKernel-PlatformCore)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessKernel-FileSystem)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessKernel-MediaCore)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-Lua)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-Core)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-RHI)

