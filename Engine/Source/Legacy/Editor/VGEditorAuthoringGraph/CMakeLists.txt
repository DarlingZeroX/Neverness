# VGEditorAuthoringGraph — 序列 Authoring 图数据层（与线性运行时解耦）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(VGEditorAuthoringGraph STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/AuthoringGraph/SequenceAuthoringGraph.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/AuthoringGraph/SequenceAuthoringGraph.cpp"
)

target_include_directories(VGEditorAuthoringGraph PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
target_link_libraries(VGEditorAuthoringGraph PUBLIC VGCore)
