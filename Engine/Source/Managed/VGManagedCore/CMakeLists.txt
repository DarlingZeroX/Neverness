# VGManagedCore — VisionGal Managed Runtime 基础层（Phase 2：Native/Managed ABI 合约）
# 静态库：由 VGManagedHost 等目标 PRIVATE 链接；不在此链接 nethost / hostfxr。

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

set(VGMANAGEDCORE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/NativeAPI.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedRuntimeServices.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedExports.cpp"
)

add_library(VGManagedCore STATIC ${VGMANAGEDCORE_SOURCES})

target_include_directories(VGManagedCore
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_link_libraries(VGManagedCore PUBLIC VGNativeEngineAPI)
