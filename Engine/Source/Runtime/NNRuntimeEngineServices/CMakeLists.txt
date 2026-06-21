# NevernessRuntime-EngineServices — 將 NNNativeEngineAPI 轉發至 NNEngineRuntime（不鏈結 VGEngine）。

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

add_library(NevernessRuntime-EngineServices STATIC)

target_sources(NevernessRuntime-EngineServices
	PRIVATE
		Private/Registry/NativeEngineRuntimeApiTable.cpp
		Private/Async/AsyncWaitRuntimeApi.cpp
		Private/ViewportRender/ViewportRenderRuntimeApi.cpp
		Private/ViewportSurface/ViewportSurfaceRuntimeApi.cpp
		Private/Diligent/DiligentRuntimeApi.cpp
)

target_include_directories(NevernessRuntime-EngineServices PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-EngineServices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeEngine/Include)

target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)

target_include_directories(NevernessRuntime-EngineServices PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeApplication/Include
	${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeVFS/Include
	# Rendering include（INNRenderDevice 等接口）
	${PROJECT_SOURCE_DIR}/Engine/Source/Rendering
	${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRender
	${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeCore
	# NNRuntimeDiligent include（NNDiligentViewportSurface 等）
	${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent
)

target_link_libraries(NevernessRuntime-EngineServices PUBLIC
	NevernessRuntime-NativeEngineAPI
	NevernessRuntime-NativeEngineAPIStub
	NevernessRuntime-Engine
	NevernessRuntime-Application
	NevernessRuntime-VFS
	NevernessRuntime-RmlUI
	# ViewportSurface 需要 NNRuntimeDiligent（NNDiligentViewportSurface、SwapChain 创建）
	NNRuntimeDiligent
	# ViewportSurfaceRuntimeApi 直接使用 Renderer2D（RenderCommands 路径）
	NevernessRuntime-Renderer2D
)

add_library(NNEngineRuntimeServices ALIAS NevernessRuntime-EngineServices)
