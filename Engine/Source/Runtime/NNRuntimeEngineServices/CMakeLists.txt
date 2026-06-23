# NevernessRuntime-EngineServices — Engine Service ABI 运行时转发 + NNNativeAPI 导出表（SHARED）
#
# 导出：
#   - NNNativeApi_GetDefaultTable()  — 进程单例 NNNativeAPI* 指针
#   - NNNativeApiTable_BuildDefault() — 填充默认表（挂载 Runtime EngineServices）

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
set(ENGINE_SERVICES_SOURCES
	# Runtime Engine Service 转发
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Registry/NativeEngineRuntimeApiTable.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Async/AsyncWaitRuntimeApi.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ViewportRender/ViewportRenderRuntimeApi.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ViewportSurface/ViewportSurfaceRuntimeApi.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Diligent/DiligentRuntimeApi.cpp"
	# NNNativeAPI 导出表（从 NNAPI 迁入）
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Registry/ManagedExports.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/Registry/ManagedRuntimeServices.cpp"
)

# ── 目标（SHARED） ─────────────────────────────────────────────
add_library(NevernessRuntime-EngineServices SHARED ${ENGINE_SERVICES_SOURCES})

target_compile_definitions(NevernessRuntime-EngineServices
	PRIVATE
		NN_ENGINE_SERVICES_EXPORT
		NEVERNESS_USE_ENGINE_RUNTIME_SERVICES=1
)

target_include_directories(NevernessRuntime-EngineServices
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
		# NNAPI 头文件（Engine Service ABI + Stub 声明）
		"${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNAPI/Include"
	PRIVATE
		"${PROJECT_SOURCE_DIR}/Engine/Source/Runtime"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Core"
		"${CMAKE_CURRENT_SOURCE_DIR}/Interface"
		"${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeEngine/Include"
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
		"${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeApplication/Include"
		"${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeVFS/Include"
		# Rendering include（INNRenderDevice 等接口）
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRender"
		# NNRuntimeDiligent include（NNDiligentViewportSurface 等）
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent"
		# NNRenderBootstrap include（设备创建工厂）
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRenderBootstrap/Include"
)

target_link_libraries(NevernessRuntime-EngineServices
	PUBLIC
		NevernessRuntime-API
	PRIVATE
		NevernessRuntime-Engine
		NevernessRuntime-Application
		NevernessRuntime-VFS
		NevernessRuntime-RmlUI
		NNRuntimeDiligent
		NNRuntimeRenderBootstrap
		NevernessRuntime-Renderer2D
)

add_library(NNEngineRuntimeServices ALIAS NevernessRuntime-EngineServices)
