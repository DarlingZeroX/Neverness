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
	# 定义托管运行时项目文件路径
	set(_managed_csproj "${CMAKE_CURRENT_SOURCE_DIR}/Managed/VisionGal.Managed.Runtime/VisionGal.Managed.Runtime.csproj")
	# 收集托管运行时模块的C#源码及项目文件，CONFIGURE_DEPENDS表示文件变更时自动重新配置
	file(GLOB _managed_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/Managed/VisionGal.Managed.Runtime/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/Managed/VisionGal.Managed.Runtime/*.csproj"
	)
	# 收集托管核心模块的C#源码及项目文件
	file(GLOB _managed_core_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedCore/Managed/VisionGal.Managed.Core/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedCore/Managed/VisionGal.Managed.Core/*.csproj"
	)
	# 收集托管引擎模块的C#源码及项目文件
	file(GLOB _managed_engine_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedEngine/Managed/VisionGal.Managed.Engine/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedEngine/Managed/VisionGal.Managed.Engine/*.csproj"
	)
	file(GLOB _managed_engine_runtime_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedEngine/Managed/VisionGal.Managed.Engine.Runtime/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedEngine/Managed/VisionGal.Managed.Engine.Runtime/*.csproj"
	)
	file(GLOB _managed_object_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedObject/Managed/VisionGal.Managed.Object/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedObject/Managed/VisionGal.Managed.Object/*.csproj"
	)
	file(GLOB _managed_reflection_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedReflection/Managed/VisionGal.Managed.Reflection/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedReflection/Managed/VisionGal.Managed.Reflection/*.csproj"
	)
	file(GLOB _managed_serialization_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedSerialization/Managed/VisionGal.Managed.Serialization/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedSerialization/Managed/VisionGal.Managed.Serialization/*.csproj"
	)
	file(GLOB _managed_assets_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedAssets/Managed/VisionGal.Managed.Assets/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedAssets/Managed/VisionGal.Managed.Assets/*.csproj"
	)
	file(GLOB _managed_editor_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedEditor/Managed/VisionGal.Managed.Editor/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedEditor/Managed/VisionGal.Managed.Editor/*.csproj"
	)
	file(GLOB _managed_inspector_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedInspector/Managed/VisionGal.Managed.Inspector/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedInspector/Managed/VisionGal.Managed.Inspector/*.csproj"
	)
	file(GLOB _managed_graph_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedGraph/Managed/VisionGal.Managed.Graph/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedGraph/Managed/VisionGal.Managed.Graph/*.csproj"
	)
	file(GLOB _managed_scene_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedScene/Managed/VisionGal.Managed.Scene/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedScene/Managed/VisionGal.Managed.Scene/*.csproj"
	)
	file(GLOB _managed_undoredo_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedUndoRedo/Managed/VisionGal.Managed.UndoRedo/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedUndoRedo/Managed/VisionGal.Managed.UndoRedo/*.csproj"
	)
	file(GLOB _managed_scripting_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedScripting/Managed/VisionGal.Managed.Scripting/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedScripting/Managed/VisionGal.Managed.Scripting/*.csproj"
	)
	file(GLOB _managed_gameplay_sources CONFIGURE_DEPENDS
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedGameplay/Managed/VisionGal.Managed.Gameplay/*.cs"
		"${CMAKE_CURRENT_SOURCE_DIR}/../VGManagedGameplay/Managed/VisionGal.Managed.Gameplay/*.csproj"
	)

	# 自定义构建命令：编译并发布 VisionGal.Managed.Runtime 托管程序集，用于VGManagedHost测试
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
			${_managed_scripting_sources} ${_managed_gameplay_sources}
		# 构建时控制台输出提示信息
		COMMENT "Publishing VisionGal.Managed.Runtime for VGManagedHost tests"
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
