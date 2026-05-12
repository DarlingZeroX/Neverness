# VGEditorExtensions — 序列编辑器扩展 SDK（静态注册）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(VGEditorExtensions STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Extensions/ISequenceEditorExtension.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Extensions/SequenceExtensionRegistry.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Extensions/SequenceExtensionRegistry.cpp"
)

target_include_directories(VGEditorExtensions PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
