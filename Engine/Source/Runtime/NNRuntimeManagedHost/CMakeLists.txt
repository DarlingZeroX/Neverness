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

add_custom_command(TARGET NevernessRuntime-ManagedHost POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"$<TARGET_FILE:NevernessRuntime-Managed>"
		"$<TARGET_FILE_DIR:NevernessRuntime-ManagedHost>"
	VERBATIM
)

if (UNIX AND NOT APPLE)
	target_link_libraries(NevernessRuntime-ManagedHost PRIVATE dl)
endif()

add_library(NNRuntimeManagedHost ALIAS NevernessRuntime-ManagedHost)
add_library(VGManagedHost ALIAS NevernessRuntime-ManagedHost)

if(NOT DEFINED VISIONGAL_MANAGED_PUBLISH_DIR)
	set(VISIONGAL_MANAGED_PUBLISH_DIR "${CMAKE_BINARY_DIR}/ManagedRuntimePublish" CACHE INTERNAL "")
endif()

find_program(VISIONGAL_DOTNET_EXECUTABLE dotnet DOC ".NET SDK CLI")

if (VISIONGAL_DOTNET_EXECUTABLE)
	set(_managed_runtime_root "${CMAKE_CURRENT_SOURCE_DIR}/../../Managed/Runtime")
	set(_managed_csproj "${_managed_runtime_root}/Host/NevernessRuntimeManaged-Runtime.csproj")
	file(GLOB _managed_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Host/*.cs"
		"${_managed_runtime_root}/Host/*.csproj"
	)
	file(GLOB _managed_core_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Core/*.cs"
		"${_managed_runtime_root}/Core/*.csproj"
	)
	file(GLOB _managed_engine_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Engine/*.cs"
		"${_managed_runtime_root}/Engine/*.csproj"
	)
	file(GLOB _managed_engine_runtime_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Engine.Runtime/*.cs"
		"${_managed_runtime_root}/Engine.Runtime/*.csproj"
	)
	file(GLOB _managed_object_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Object/*.cs"
		"${_managed_runtime_root}/Object/*.csproj"
	)
	file(GLOB _managed_reflection_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Reflection/*.cs"
		"${_managed_runtime_root}/Reflection/*.csproj"
	)
	file(GLOB _managed_serialization_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Serialization/*.cs"
		"${_managed_runtime_root}/Serialization/*.csproj"
	)
	file(GLOB _managed_assets_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Assets/*.cs"
		"${_managed_runtime_root}/Assets/*.csproj"
	)
	file(GLOB _managed_editor_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../../Managed/Editor/NNEditorFramwork/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../../Managed/Editor/NNEditorFramwork/*.csproj"
	)
	file(GLOB _managed_inspector_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Inspector/*.cs"
		"${_managed_runtime_root}/Inspector/*.csproj"
	)
	file(GLOB _managed_graph_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Graph/*.cs"
		"${_managed_runtime_root}/Graph/*.csproj"
	)
	file(GLOB _managed_scene_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Scene/*.cs"
		"${_managed_runtime_root}/Scene/*.csproj"
	)
	file(GLOB _managed_undoredo_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/UndoRedo/*.cs"
		"${_managed_runtime_root}/UndoRedo/*.csproj"
	)
	file(GLOB _managed_scripting_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Scripting/*.cs"
		"${_managed_runtime_root}/Scripting/*.csproj"
	)
	file(GLOB _managed_gameplay_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Gameplay/*.cs"
		"${_managed_runtime_root}/Gameplay/*.csproj"
	)
	file(GLOB _managed_entity_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/Entity/*.cs"
		"${_managed_runtime_root}/Entity/*.csproj"
	)
	file(GLOB _managed_runtimeloop_sources CONFIGURE_DEPENDS
		"${_managed_runtime_root}/RuntimeLoop/*.cs"
		"${_managed_runtime_root}/RuntimeLoop/*.csproj"
	)

	add_custom_command(
		OUTPUT "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${VISIONGAL_MANAGED_PUBLISH_DIR}"
		COMMAND "${VISIONGAL_DOTNET_EXECUTABLE}" publish "${_managed_csproj}" -c Debug -o "${VISIONGAL_MANAGED_PUBLISH_DIR}" /p:UseAppHost=false
		COMMAND ${CMAKE_COMMAND} -E touch "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
		DEPENDS ${_managed_sources} ${_managed_core_sources} ${_managed_engine_sources} ${_managed_engine_runtime_sources}
			${_managed_object_sources} ${_managed_reflection_sources} ${_managed_serialization_sources}
			${_managed_assets_sources} ${_managed_editor_sources} ${_managed_inspector_sources}
			${_managed_graph_sources} ${_managed_scene_sources} ${_managed_undoredo_sources}
			${_managed_scripting_sources} ${_managed_gameplay_sources} ${_managed_entity_sources}
			${_managed_runtimeloop_sources}
		COMMENT "Publishing NevernessRuntimeManaged-Runtime for managed host tests"
		VERBATIM
	)
	add_custom_target(visiongal_managed_runtime_publish
		DEPENDS "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
	)
else()
	add_custom_target(visiongal_managed_runtime_publish
		COMMAND ${CMAKE_COMMAND} -E echo "dotnet not found ? skip managed publish (managed host tests will not be built)"
	)
endif()

if (ENABLE_TESTS AND VISIONGAL_DOTNET_EXECUTABLE)
	add_subdirectory("${CMAKE_SOURCE_DIR}/Engine/Source/Tests/VGManagedHostTest" VGManagedHostTest_bin)
endif()
