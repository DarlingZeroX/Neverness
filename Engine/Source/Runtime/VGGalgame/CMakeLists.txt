# CMakeList.txt: GalEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
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

# 将源代码添加到此项目的可执行文件。
file(GLOB HCORE_INTERFACE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB HCORE_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	   "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Core/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Interface/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/DialogueSystem/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/UISystem/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/SceneSystem/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ScriptSystem/*.h"
)

file(GLOB HCORE_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/DialogueSystem/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UISystem/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/SceneSystem/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ScriptSystem/*.cpp"
) 

# 库
add_library (VGGalgame SHARED ${HCORE_INTERFACE_FILES} ${HCORE_HEADER_FILES} ${HCORE_SRC_FILES})

# 仅对动态库目标定义 CORE_MODULE_EXPORT
target_compile_definitions(VGGalgame PRIVATE VG_GALGAME_EXPORT)

# 添加头文件搜索路径
target_include_directories(VGGalgame PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGGalgame PUBLIC ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/VGLua/Include)
target_include_directories(VGGalgame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGGalgame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

target_link_libraries(VGGalgame PUBLIC VGGalgameNodeGraph)
target_link_libraries(VGGalgame PUBLIC VGGalgameCore)
target_link_libraries(VGGalgame PUBLIC VGGalgameScriptLua)
target_link_libraries(VGGalgame PUBLIC VGGalgameScriptSequence)
