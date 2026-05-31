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
		Private/Timing/TimingRuntimeApi.cpp
		Private/Async/AsyncWaitRuntimeApi.cpp
		Private/Scene/SceneRuntimeApi.cpp
		Private/Asset/AssetRuntimeApi.cpp
		Private/Object/ObjectRuntimeApi.cpp
		Private/AssetRegistry/AssetRegistryRuntimeApi.cpp
		Private/RenderAsset/RenderAssetRuntimeApi.cpp
		Private/ViewportRender/ViewportRenderRuntimeApi.cpp
)

target_include_directories(NevernessRuntime-EngineServices PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-EngineServices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeEngine/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeScene/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeAsset/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNAssetCooker/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeRenderAssets/Include)
target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeRenderer2D/Include)

target_include_directories(NevernessRuntime-EngineServices PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)

target_include_directories(NevernessRuntime-EngineServices PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeApplication/Include
	${CMAKE_CURRENT_SOURCE_DIR}/../NNRuntimeVFS/Include
)

target_link_libraries(NevernessRuntime-EngineServices PUBLIC
	NevernessRuntime-NativeEngineAPI
	NevernessRuntime-NativeEngineAPIStub
	NevernessRuntime-Engine
	NevernessRuntime-Asset
	NevernessRuntime-AssetCooker
	NevernessRuntime-ManagedBridge
	NevernessRuntime-Application
	NevernessRuntime-VFS
	NevernessRuntime-RenderAssets
	NevernessRuntime-Renderer2D
	NevernessRuntime-RmlUI
	NevernessRuntime-Scene
)

add_library(NNEngineRuntimeServices ALIAS NevernessRuntime-EngineServices)
