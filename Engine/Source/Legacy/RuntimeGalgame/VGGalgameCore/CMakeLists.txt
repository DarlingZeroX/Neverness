# VGGalgameCore — Phase 8 聚合目标（INTERFACE）
#
# 中文：物理实现已拆至 **VGGalgameContract**（纯 ABI 头）与 **VGGalgameRuntimeCore**（SHARED，状态/存档/工厂）。
# 本目标保留 CMake 名 `VGGalgameCore`，便于存量 `target_link_libraries(... VGGalgameCore)` 不变；
# 并通过本目录下 **薄转发头** 保留 `#include "VGGalgameCore/..."` 路径。

add_library(VGGalgameCore INTERFACE)

target_link_libraries(VGGalgameCore INTERFACE VGGalgameContract VGGalgameRuntimeCore)

target_include_directories(VGGalgameCore INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
	${PROJECT_SOURCE_DIR}/Engine/Source/RuntimeGalgame
)
