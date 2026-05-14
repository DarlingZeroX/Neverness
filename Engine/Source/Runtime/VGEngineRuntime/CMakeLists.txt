# VGEngineRuntime — 行程級 Runtime Facade（Timing / Async 等；不鏈結 VGEngine）。

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

add_library(VGEngineRuntime STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/VGEngineRuntime.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/TimingSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/AsyncSystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/SceneSubsystem.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/AssetSubsystem.cpp"
)

target_include_directories(VGEngineRuntime
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(VGEngineRuntime PUBLIC VGNativeEngineAPI)
