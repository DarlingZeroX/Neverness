# VGEditorReactive — 无业务响应式失效图内核（Phase 8）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(VGEditorReactive STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ReactiveCore/DerivedStateGraph.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ReactiveCore/DerivedStateGraph.cpp"
)

target_include_directories(VGEditorReactive PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
