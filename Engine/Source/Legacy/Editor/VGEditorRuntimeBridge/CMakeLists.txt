# VGEditorRuntimeBridge — 运行时事件帧与时间线（无 Sequence 链接依赖）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(VGEditorRuntimeBridge STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/RuntimeBridge/SequenceRuntimeEventFrame.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/RuntimeBridge/SequenceRuntimeEventTimeline.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/RuntimeBridge/SequenceRuntimeEventTimeline.cpp"
)

target_include_directories(VGEditorRuntimeBridge PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
