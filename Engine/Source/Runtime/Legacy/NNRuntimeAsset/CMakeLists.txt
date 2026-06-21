# NNRuntimeAsset — Runtime 资产系统（AssetManager / Registry / Loader / Streaming / Cache / Format）。
# SHARED 库：解决跨 DLL 边界的单例复制问题（NNAssetRegistry::Instance() 全局唯一）。

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

add_library(NevernessRuntime-Asset SHARED)

target_sources(NevernessRuntime-Asset
	PRIVATE
		# AssetManager / Loader / Streaming / Cache / Format
		Private/NNAssetManager.cpp
		Private/NNHandleTable.cpp
		Private/NNAssetCache.cpp
		Private/NNStreamingManager.cpp
		Private/NNAssetFormat.cpp
		Private/NNPackManager.cpp
		Private/NNAssetMemoryPool.cpp
		# AssetRegistry（原 NNAssetRegistry 模块，合并到此）
		Private/NNAssetRegistry.cpp
		Private/NNGuidTable.cpp
		Private/NNDependencyTable.cpp
)

target_include_directories(NevernessRuntime-Asset PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Asset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Asset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)

target_compile_definitions(NevernessRuntime-Asset PRIVATE NN_ASSET_DLL_EXPORT NN_ASSET_DYNAMIC)
target_compile_definitions(NevernessRuntime-Asset PUBLIC NN_ASSET_DYNAMIC)

target_link_libraries(NevernessRuntime-Asset PUBLIC NevernessCore-Core)
target_link_libraries(NevernessRuntime-Asset PUBLIC NevernessRuntime-NativeEngineAPI)
target_link_libraries(NevernessRuntime-Asset PUBLIC NevernessRuntime-VFS)
