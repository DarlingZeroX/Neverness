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
	set(_managed_runtime_root "${CMAKE_CURRENT_SOURCE_DIR}/../../ManagedRuntime")
	# 定义托管运行时项目文件路径
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
		"${CMAKE_CURRENT_SOURCE_DIR}/../../ManagedEditor/VGManagedEditor/Managed/VisionGal.Managed.Editor/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../../ManagedEditor/VGManagedEditor/Managed/VisionGal.Managed.Editor/*.csproj"
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

	# 自定义构建命令：编译并发布 NevernessRuntimeManaged-Runtime 托管程序集，用于 VGManagedHost 测试
	add_custom_command(
		# 输出标记文件，用于标识发布操作已完成，避免重复执行
		OUTPUT "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
		# 创建托管程序发布目录，若目录已存在则无操作
		COMMAND ${CMAKE_COMMAND} -E make_directory "${VISIONGAL_MANAGED_PUBLISH_DIR}"
		# 调用.NET CLI发布指定C#项目，Debug配置，输出至目标目录，禁用独立可执行文件
		COMMAND "${VISIONGAL_DOTNET_EXECUTABLE}" publish "${_managed_csproj}" -c Debug -o "${VISIONGAL_MANAGED_PUBLISH_DIR}" /p:UseAppHost=false
		# 生成空的标记文件，作为本自定义命令的输出产物
		COMMAND ${CMAKE_COMMAND} -E touch "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
		# 依赖源文件：托管通用源码、核心源码、引擎源码，文件变更时触发重新发布
		DEPENDS ${_managed_sources} ${_managed_core_sources} ${_managed_engine_sources} ${_managed_engine_runtime_sources}
			${_managed_object_sources} ${_managed_reflection_sources} ${_managed_serialization_sources}
			${_managed_assets_sources} ${_managed_editor_sources} ${_managed_inspector_sources}
			${_managed_graph_sources} ${_managed_scene_sources} ${_managed_undoredo_sources}
			${_managed_scripting_sources} ${_managed_gameplay_sources} ${_managed_entity_sources}
			${_managed_runtimeloop_sources}
		# 构建时控制台输出提示信息
		COMMENT "Publishing NevernessRuntimeManaged-Runtime for VGManagedHost tests"
		# 严格按字面执行命令，不进行CMake自动转义处理
		VERBATIM
	)
	# 定义自定义构建目标 visiongal_managed_runtime_publish
	# 该目标依赖于时间戳文件，用于触发运行时托管组件的发布操作
	add_custom_target(visiongal_managed_runtime_publish
		DEPENDS "${VISIONGAL_MANAGED_PUBLISH_DIR}/.visiongal_managed_publish_stamp"
	)
else()
	# 定义自定义构建目标：visiongal_managed_runtime_publish
	# 用于在未找到dotnet工具时，跳过托管运行时发布流程，并给出提示
	add_custom_target(visiongal_managed_runtime_publish
		COMMAND ${CMAKE_COMMAND} -E echo "dotnet not found — skip managed publish (VGManagedHost tests will not be built)"
	)
endif()

if (ENABLE_TESTS AND VISIONGAL_DOTNET_EXECUTABLE)
	add_subdirectory("${CMAKE_SOURCE_DIR}/Engine/Source/Tests/VGManagedHostTest" VGManagedHostTest_bin)
endif()
