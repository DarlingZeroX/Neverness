# NNRuntimeManaged — Native ↔ Managed ABI（NNNativeAPI 函数表，SHARED 导出）

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

set(NNRUNTIMEMANAGED_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/NativeAPI.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedRuntimeServices.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedExports.cpp"
)

add_library(NevernessRuntime-Managed SHARED ${NNRUNTIMEMANAGED_SOURCES})

target_compile_definitions(NevernessRuntime-Managed
	PRIVATE
		NN_RUNTIME_MANAGED_EXPORT
		NN_RUNTIME_MANAGED_BUILD_SHARED
	INTERFACE
		NN_RUNTIME_MANAGED_BUILD_SHARED
)

target_include_directories(NevernessRuntime-Managed
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(NevernessRuntime-Managed PUBLIC
	NevernessRuntime-NativeEngineAPI
	NevernessRuntime-NativeEngineAPIStub
)

target_link_libraries(NevernessRuntime-Managed PUBLIC NevernessRuntime-EngineServices)

add_library(NNRuntimeManaged ALIAS NevernessRuntime-Managed)
