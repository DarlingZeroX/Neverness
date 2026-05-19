# NNRuntimeManagedHostLegacy — Legacy CoreCLR 宿主（nethost / hostfxr）；主路径请用 Entry.Bootstrap + 产品 hostfxr。
# 在 Engine/Source/Runtime/CMakeLists.txt 中 VISIONGAL_ENABLE_MANAGED_HOST_LEGACY=ON 时构建。

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

find_package(unofficial-nethost CONFIG REQUIRED)

set(NNRUNTIMEMANAGEDHOSTLEGACY_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/CoreCLRLoader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedHost.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedHostLegacyBootstrap.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedRuntime.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedAssembly.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedInterop.cpp"
)

add_library(NevernessRuntime-ManagedHostLegacy SHARED ${NNRUNTIMEMANAGEDHOSTLEGACY_SOURCES})

target_compile_definitions(NevernessRuntime-ManagedHostLegacy
	PRIVATE
		NN_RUNTIME_MANAGED_HOST_EXPORT
		NN_RUNTIME_MANAGED_HOST_BUILD_SHARED
	INTERFACE
		NN_RUNTIME_MANAGED_HOST_BUILD_SHARED
)

target_include_directories(NevernessRuntime-ManagedHostLegacy
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(NevernessRuntime-ManagedHostLegacy
	PRIVATE
		NevernessRuntime-Managed
		NevernessRuntime-ManagedBridge
		unofficial::nethost::nethost
)

if (UNIX AND NOT APPLE)
	target_link_libraries(NevernessRuntime-ManagedHostLegacy PRIVATE dl)
endif()

add_library(NNRuntimeManagedHostLegacy ALIAS NevernessRuntime-ManagedHostLegacy)
# 兼容别名（已弃用，下一发行周期移除）
add_library(NevernessRuntime-ManagedHost ALIAS NevernessRuntime-ManagedHostLegacy)
add_library(NNRuntimeManagedHost ALIAS NevernessRuntime-ManagedHostLegacy)
add_library(VGManagedHost ALIAS NevernessRuntime-ManagedHostLegacy)
