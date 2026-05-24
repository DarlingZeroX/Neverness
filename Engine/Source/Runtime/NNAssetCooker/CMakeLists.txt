# NNAssetCooker — 资产编译/打包编排器（.nnpack 构建）。

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

add_library(NevernessRuntime-AssetCooker STATIC)

target_sources(NevernessRuntime-AssetCooker
	PRIVATE
		Private/NNAssetCooker.cpp
		Private/NNCookManifest.cpp
		Private/NNPackBuilder.cpp
		Private/NNAssetCookerApi.cpp
)

target_include_directories(NevernessRuntime-AssetCooker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-AssetCooker PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-AssetCooker PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)

target_link_libraries(NevernessRuntime-AssetCooker PUBLIC NevernessRuntime-NativeEngineAPI)
target_link_libraries(NevernessRuntime-AssetCooker PUBLIC NevernessRuntime-Asset)
