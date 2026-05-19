# NevernessRuntime-Application — 统一 Runtime Application 层（SDL3 窗口与事件泵）。

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

find_package(SDL3 CONFIG REQUIRED)

add_library(NevernessRuntime-Application STATIC)

target_sources(NevernessRuntime-Application
	PRIVATE
		Private/RuntimeApplication.cpp
		Private/BuildApplicationApi.cpp
)

target_include_directories(NevernessRuntime-Application
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${PROJECT_SOURCE_DIR}/Engine/Source/Runtime"
)

target_link_libraries(NevernessRuntime-Application
	PUBLIC
		NevernessRuntime-NativeEngineAPI
	PRIVATE
		SDL3::SDL3
)

add_library(NNRuntimeApplication ALIAS NevernessRuntime-Application)
