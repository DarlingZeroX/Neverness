# NNAPI — Engine Service ABI 头文件 + Stub 实现（STATIC 库）
#
# 合并自：
#   - NNNativeEngineAPI（Engine Service ABI 头文件契约）
#   - NNRuntimeNativeEngineAPIStub（Stub 实现）
#   - NNRuntimeManaged（NNNativeAPI 函数表定义）
#
# 导出函数（NNNativeApi_GetDefaultTable 等）已移至 NNRuntimeEngineServices。

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# ── 源文件 ──────────────────────────────────────────────────────
set(NNAPI_STUB_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Common/StubInvokeCounter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Registry/NNNativeEngineApiTable.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Render/RenderApiStubs.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Audio/AudioApiStubs.cpp"
	# Application/Window 已移除：C# Neverness.Runtime.Application 接管 (2026-06-24)
	# Input/RenderAsset 已移除：迁移至 C# (2026-06-24)
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Vfs/VfsApiStubs.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Async/AsyncWaitApiStubs.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/Event/EventApiStubs.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub/ViewportRender/ViewportRenderApiStubs.cpp"
)

# ── 目标（STATIC） ─────────────────────────────────────────────
add_library(NevernessRuntime-API STATIC ${NNAPI_STUB_SOURCES})

target_include_directories(NevernessRuntime-API
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private/Stub"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Runtime"
)

# NNAPI 是纯 ABI 头文件 + Stub，不链接任何运行时模块

# ── 别名 ──────────────────────────────────────────────────────
add_library(NNAPI ALIAS NevernessRuntime-API)

# ── 兼容别名（下游模块仍在使用） ────────────────────────────
add_library(NevernessRuntime-NativeEngineAPI ALIAS NevernessRuntime-API)  # VFS/Engine/Application 链接
