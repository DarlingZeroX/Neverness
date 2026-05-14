# VGManagedHost — CoreCLR hosting (nethost / hostfxr), Phase 1: native -> managed only.
# Option VISIONGAL_ENABLE_MANAGED_HOST is defined in the root CMakeLists.txt.

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

set(VGMANAGEDHOST_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/CoreCLRLoader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedHost.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedRuntime.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedAssembly.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedInterop.cpp"
)

add_library(VGManagedHost SHARED ${VGMANAGEDHOST_SOURCES})

target_compile_definitions(VGManagedHost PRIVATE VG_MANAGED_HOST_EXPORT)

target_include_directories(VGManagedHost
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(VGManagedHost
	PRIVATE
		VGManagedCore
		unofficial::nethost::nethost
)

if (UNIX AND NOT APPLE)
	target_link_libraries(VGManagedHost PRIVATE dl)
endif()

set(VISIONGAL_MANAGED_PUBLISH_DIR "${CMAKE_BINARY_DIR}/ManagedRuntimePublish" CACHE INTERNAL "")

find_program(VISIONGAL_DOTNET_EXECUTABLE dotnet DOC ".NET SDK CLI")

if (VISIONGAL_DOTNET_EXECUTABLE)
	set(_managed_csproj "${CMAKE_CURRENT_SOURCE_DIR}/Managed/VisionGal.Managed.Runtime/VisionGal.Managed.Runtime.csproj")
	file(GLOB _managed_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/Managed/VisionGal.Managed.Runtime/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/Managed/VisionGal.Managed.Runtime/*.csproj"
	)
	file(GLOB _managed_core_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedCore/Managed/VisionGal.Managed.Core/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedCore/Managed/VisionGal.Managed.Core/*.csproj"
	)
	add_custom_command(
		OUTPUT "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${VISIONGAL_MANAGED_PUBLISH_DIR}"
		COMMAND "${VISIONGAL_DOTNET_EXECUTABLE}" publish "${_managed_csproj}" -c Debug -o "${VISIONGAL_MANAGED_PUBLISH_DIR}" /p:UseAppHost=false
		COMMAND ${CMAKE_COMMAND} -E touch "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
		DEPENDS ${_managed_sources} ${_managed_core_sources}
		COMMENT "Publishing VisionGal.Managed.Runtime for VGManagedHost tests"
		VERBATIM
	)
	add_custom_target(visiongal_managed_runtime_publish
		DEPENDS "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
	)
else()
	add_custom_target(visiongal_managed_runtime_publish
		COMMAND ${CMAKE_COMMAND} -E echo "dotnet not found — skip managed publish (VGManagedHost tests will not be built)"
	)
endif()

if (ENABLE_TESTS AND VISIONGAL_DOTNET_EXECUTABLE)
	add_subdirectory("${CMAKE_SOURCE_DIR}/Engine/Source/Tests/VGManagedHostTest" VGManagedHostTest_bin)
endif()
