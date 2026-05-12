# VGEditorComponentSchema — 编辑器侧组件 Authoring Schema（Phase 10，STATIC，无 ImGui / 无脚本序列 DLL）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(VGEditorComponentSchema STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequencePropertyType.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequencePropertyFlags.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequencePropertyRange.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequencePropertyValue.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequenceEnumSchema.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequenceGraphPortSchema.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequencePropertyAccessor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequencePropertySchema.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Schema/SequenceComponentSchema.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Runtime/SequenceRuntimePropertySnapshot.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Validation/GenericSchemaValidator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Validation/GenericSchemaValidator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Schema/SequencePropertyValue.cpp"
)

target_include_directories(VGEditorComponentSchema PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
