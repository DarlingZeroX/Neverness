# NNRuntimeManagedHost ? CoreCLR hosting (nethost / hostfxr), Phase 1: native -> managed only.
# ? Engine/Source/Runtime/CMakeLists.txt ? VISIONGAL_ENABLE_MANAGED_HOST=ON ??????

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

set(NNRUNTIMEMANAGEDHOST_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/CoreCLRLoader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedHost.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedRuntime.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedAssembly.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedInterop.cpp"
)

add_library(NevernessRuntime-ManagedHost SHARED ${NNRUNTIMEMANAGEDHOST_SOURCES})

target_compile_definitions(NevernessRuntime-ManagedHost
	PRIVATE
		NN_RUNTIME_MANAGED_HOST_EXPORT
		NN_RUNTIME_MANAGED_HOST_BUILD_SHARED
	INTERFACE
		NN_RUNTIME_MANAGED_HOST_BUILD_SHARED
)

target_include_directories(NevernessRuntime-ManagedHost
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(NevernessRuntime-ManagedHost
	PRIVATE
		NevernessRuntime-Managed
		unofficial::nethost::nethost
)

if (UNIX AND NOT APPLE)
	target_link_libraries(NevernessRuntime-ManagedHost PRIVATE dl)
endif()

add_library(NNRuntimeManagedHost ALIAS NevernessRuntime-ManagedHost)
add_library(VGManagedHost ALIAS NevernessRuntime-ManagedHost)
