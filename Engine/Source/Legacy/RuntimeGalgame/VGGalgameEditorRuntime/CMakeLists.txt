# VGGalgameEditorRuntime — 编辑器与 Gal 运行时之间的隔离层（Phase 8.7，INTERFACE）
#
# 中文：仅聚合契约头与薄桥接接口，禁止编辑器模块直接 `#include` VGGalgame 内部实现头。

add_library(VGGalgameEditorRuntime INTERFACE)

target_link_libraries(VGGalgameEditorRuntime INTERFACE VGGalgameContract)

target_include_directories(VGGalgameEditorRuntime INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
	${PROJECT_SOURCE_DIR}/Engine/Source/RuntimeGalgame
)
