# CMakeList.txt: VGEditorGalgameSequence — Galgame 可视化序列脚本编辑器模块
#
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

if (MSVC)
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

file(GLOB SEQUENCE_EDITOR_HEADERS
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/EditorSession/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ComponentRegistry/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Runtime/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Inspector/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Widgets/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Core/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Commands/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/ViewModels/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Validation/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Validation/Builtin/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Timeline/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Events/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Services/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Async/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Transactions/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Transactions/Patches/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Reactive/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Reactive/DerivedState/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Projection/ProjectionEvents/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/DirtyRegions/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Projection/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Properties/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Projection/Graph/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Workspace/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Diff/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Document/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Inspector/PropertyEditing/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Interface/*.h"
)

file(GLOB SEQUENCE_EDITOR_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorSession/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ComponentRegistry/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Inspector/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Widgets/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Core/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Commands/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/ViewModels/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Validation/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Validation/Builtin/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Timeline/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Events/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Services/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Async/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/AssetMonitoring/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Transactions/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Transactions/Patches/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Reactive/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Reactive/DerivedState/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Runtime/SequenceRuntimeBridgeRecorder.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/EditorSession/SequenceEditorSession.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/DirtyRegions/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Document/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Projection/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Workspace/*.cpp"
)

add_library(VGEditorGalgameSequence SHARED ${SEQUENCE_EDITOR_HEADERS} ${SEQUENCE_EDITOR_SOURCES})

target_compile_definitions(VGEditorGalgameSequence PRIVATE VG_EDITOR_GALGAME_SEQUENCE_EXPORT)

target_include_directories(VGEditorGalgameSequence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(VGEditorGalgameSequence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(VGEditorGalgameSequence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGEditorGalgameSequence PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGEditorGalgameSequence PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Editor)

target_link_libraries(VGEditorGalgameSequence PUBLIC VGEditorFramework)
target_link_libraries(VGEditorGalgameSequence PUBLIC HNGEditorCore)
target_link_libraries(VGEditorGalgameSequence PUBLIC VGGalgame)
target_link_libraries(VGEditorGalgameSequence PRIVATE HCorePlatform)
target_link_libraries(VGEditorGalgameSequence PRIVATE HFileSystem)
target_link_libraries(VGEditorGalgameSequence PRIVATE VGCore)
target_link_libraries(VGEditorGalgameSequence PUBLIC VGEditorReactive)
target_link_libraries(VGEditorGalgameSequence PUBLIC VGEditorAuthoringGraph)
target_link_libraries(VGEditorGalgameSequence PUBLIC VGEditorExtensions)
target_link_libraries(VGEditorGalgameSequence PUBLIC VGEditorRuntimeBridge)
