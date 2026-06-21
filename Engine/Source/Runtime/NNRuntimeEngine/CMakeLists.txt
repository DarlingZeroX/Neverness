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

add_library(NevernessRuntime-Engine STATIC)

target_sources(NevernessRuntime-Engine
	PRIVATE
		"Private/NNEngineRuntime.cpp"
		Private/TimingSystem.cpp
		Private/AsyncSystem.cpp
		Private/ObjectSubsystem.cpp
		Private/RuntimeSubsystemCollection.cpp
		Private/RuntimeScheduler.cpp
)

target_include_directories(NevernessRuntime-Engine PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Engine PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)

target_link_libraries(NevernessRuntime-Engine PUBLIC NevernessRuntime-NativeEngineAPI)
