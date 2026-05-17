# VGNativeEngineAPI — Native Engine Service ABI（函數表 + Phase 3 Stub），不連結 VGEngine。

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

add_library(NevernessRuntime-NativeEngineAPI STATIC)

target_sources(NevernessRuntime-NativeEngineAPI
	PRIVATE
		Private/VGNativeEngineApiStubs.cpp
)

target_include_directories(NevernessRuntime-NativeEngineAPI
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
)
