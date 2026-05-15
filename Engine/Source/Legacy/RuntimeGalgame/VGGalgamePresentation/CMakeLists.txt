# VGGalgamePresentation — Gal 表现层（渲染管线、后续转场/动画/对白 UI）

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

add_library(VGGalgamePresentation SHARED
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/RenderPipeline.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/RenderPipeline.cpp"
)

target_compile_definitions(VGGalgamePresentation PRIVATE VG_GALGAME_PRESENTATION_EXPORT)

target_include_directories(VGGalgamePresentation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(VGGalgamePresentation PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGGalgamePresentation PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/RuntimeGalgame)

target_link_libraries(VGGalgamePresentation PUBLIC VGGalgameCore)
target_link_libraries(VGGalgamePresentation PUBLIC VGEngine)
