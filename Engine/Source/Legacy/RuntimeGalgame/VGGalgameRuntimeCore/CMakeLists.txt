# VGGalgameRuntimeCore — Phase 8 运行时状态、存档、上下文、工厂实现（SHARED）
#
# 中文：承接原 VGGalgameCore 中「非纯 ABI」部分；对外仍导出 VG_GALGAME_CORE_API 以保持二进制兼容习惯。

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

file(GLOB RUNTIME_INTERFACE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB RUNTIME_HEADER_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB RUNTIME_SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Interface/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/DialogueSystem/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/UISystem/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/SceneSystem/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ScriptSystem/*.cpp"
)

add_library(VGGalgameRuntimeCore SHARED ${RUNTIME_INTERFACE_FILES} ${RUNTIME_HEADER_FILES} ${RUNTIME_SRC_FILES})

# 中文：对外 DLL 文件名保持为 VGGalgameCore.dll，避免启动器/插件硬编码路径失效。
#set_target_properties(VGGalgameRuntimeCore PROPERTIES OUTPUT_NAME VGGalgameCore)

target_compile_definitions(VGGalgameRuntimeCore PRIVATE VG_GALGAME_RUNTIME_CORE_EXPORT)

target_include_directories(VGGalgameRuntimeCore PUBLIC
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
	${PROJECT_SOURCE_DIR}/Engine/Source/RuntimeGalgame
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/VGLua/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Include
	${CMAKE_CURRENT_SOURCE_DIR}/Interface
)

target_link_libraries(VGGalgameRuntimeCore PUBLIC VGGalgameContract)
target_link_libraries(VGGalgameRuntimeCore PUBLIC VGEngine)
