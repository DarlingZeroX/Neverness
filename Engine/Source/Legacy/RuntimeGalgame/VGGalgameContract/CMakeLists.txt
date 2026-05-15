# VGGalgameContract — Phase 8 纯 ABI / 接口头（INTERFACE 库，无独立 .cpp）
#
# 中文：与存档、Lua 绑定、GalGameContext 实现解耦；导出宏仍使用 VG_GALGAME_CORE_API 名，
# 由 VGGalgameRuntimeCore 在编译 SHARED 时定义 VG_GALGAME_CORE_EXPORT 并导出符号。

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(VGGalgameContract INTERFACE)

target_include_directories(VGGalgameContract INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
)

# 契约头依赖引擎与核心类型（float4、ISubGameEngine、VGEngineResource 等）
target_link_libraries(VGGalgameContract INTERFACE VGEngine VGCore)
