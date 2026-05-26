# NNRuntimeAsset — Runtime 資產系統（AssetManager / Loader / Streaming / Cache / Format）。

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

add_library(NevernessRuntime-Asset STATIC)

target_sources(NevernessRuntime-Asset
	PRIVATE
		Private/NNAssetManager.cpp
		Private/NNHandleTable.cpp
		Private/NNAssetCache.cpp
		Private/NNStreamingManager.cpp
		Private/NNAssetFormat.cpp
		Private/NNAssetManagerApi.cpp
		Private/NNPackManager.cpp
		Private/NNAssetMemoryPool.cpp
)

target_include_directories(NevernessRuntime-Asset PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Asset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Asset PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)

target_link_libraries(NevernessRuntime-Asset PUBLIC NevernessRuntime-NativeEngineAPI)
target_link_libraries(NevernessRuntime-Asset PUBLIC NevernessRuntime-VFS)
