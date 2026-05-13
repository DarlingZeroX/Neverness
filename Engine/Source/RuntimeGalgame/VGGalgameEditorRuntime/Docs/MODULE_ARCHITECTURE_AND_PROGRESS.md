# VGGalgameEditorRuntime — 编辑器运行时隔离（Phase 8.7）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 为编辑器模块提供 **仅依赖 Contract** 的桥接头，减少对 `VGGalgame` 内部类型的直接耦合。 |
| **CMake** | `INTERFACE`；`PUBLIC` 链接 **`VGGalgameContract`**；暴露本目录与 `Engine/Source/Runtime` 包含根。 |
| **入口** | `Interface/IEditorGalgameRuntimeBridge.h`。 |

## 2. 集成

- **`VGEditorGalgame`**：`target_link_libraries(... PUBLIC VGGalgameEditorRuntime)`。

## 3. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-13 | Phase 8.7：首版 `IEditorGalgameRuntimeBridge` 骨架。 |
