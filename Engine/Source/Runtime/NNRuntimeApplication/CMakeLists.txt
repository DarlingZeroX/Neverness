# NevernessRuntime-Application — SDL3 窗口、Diligent 宿主与 ImGui 层。
# Phase 5: 移除 OpenGL 依赖，改用 Diligent 设备+交换链

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

find_package(SDL3 CONFIG REQUIRED)
#find_package(SDL3_image CONFIG REQUIRED)

add_library(NevernessRuntime-Application STATIC)

target_sources(NevernessRuntime-Application
	PRIVATE
		ApplicationExport.h
		Include/ApplicationApiExport.h
		Include/WindowApiExport.h
		Include/EventApiExport.h
		Include/Core/EventQueue.h
		Include/Core/SDL3EventTranslator.h
		Include/Core/Window.h
		Include/Core/WindowRegistry.h
		Include/Engine/ImGuiLayer.h
		Include/RuntimeApplication.h
		Include/RuntimeApplicationInstance.h
		Private/Core/Window.cpp
		Private/Core/WindowRegistry.cpp
		Private/Editor/EditorCore.cpp
		Private/Editor/EditorInitializer.cpp
		Private/Engine/ImGuiLayer.cpp
		Private/RuntimeApplication.cpp
		Private/RuntimeApplicationInstance.cpp
		Private/BuildApplicationApi.cpp
		Private/BuildWindowApi.cpp
		Private/BuildEventApi.cpp
		"Include/Editor/EditorInitializer.h"
		"Private/Editor/EditorInitializer.cpp"
		"Include/Editor/EditorCore.h"
		"Private/Editor/EditorCore.cpp"
)

target_compile_definitions(NevernessRuntime-Application PRIVATE EDITOR_PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

target_include_directories(NevernessRuntime-Application
	PUBLIC
		"${PROJECT_SOURCE_DIR}/Engine/Source/Runtime"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Core"
		# Rendering include（Diligent 设备/交换链）
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRender"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeCore"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeDiligent/Include"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Rendering/NNRuntimeRenderBootstrap/Include"
		"${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/DiligentEngine/DiligentCore"
)

target_link_libraries(NevernessRuntime-Application
	PUBLIC
		NevernessRuntime-NativeEngineAPI
		SDL3::SDL3
	PRIVATE
		#$<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
		NevernessCore-Core
		NevernessCore-PlatformCore
		NevernessCore-FileSystem
		NevernessRuntime-Core
		NevernessRuntime-VFS
		NevernessRuntime-ImGui
		NevernessRuntime-Pak
		# Diligent 后端（替代 NevernessRuntime-RHI + opengl32）
		NNRuntimeRender
		NNRuntimeDiligent
		NNRuntimeRenderBootstrap
		# RenderAssets（IRenderResourceFactory + NNRenderAssetManager）
		NevernessRuntime-RenderAssets
)

