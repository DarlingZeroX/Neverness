# NNAssetRegistry — Runtime 資產註冊表（GUID ↔ Path + 依賴圖）。

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

add_library(NevernessRuntime-AssetRegistry STATIC)

target_sources(NevernessRuntime-AssetRegistry
	PRIVATE
		Private/NNAssetRegistry.cpp
		Private/NNGuidTable.cpp
		Private/NNDependencyTable.cpp
		Private/NNAssetRegistryApi.cpp
)

target_include_directories(NevernessRuntime-AssetRegistry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-AssetRegistry PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-AssetRegistry PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)

target_link_libraries(NevernessRuntime-AssetRegistry PUBLIC NevernessRuntime-NativeEngineAPI)
