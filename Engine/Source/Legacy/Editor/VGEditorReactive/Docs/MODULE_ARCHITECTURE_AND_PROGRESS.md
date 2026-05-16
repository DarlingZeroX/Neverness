# VGEditorReactive 模块架构与进展

## 1. 定位

**VGEditorReactive** 是 Phase 8 抽离的 **无 Galgame/无 ImGui** 的响应式小内核，提供：

- 有向 **依赖边** 与 **拓扑 flush**
- **`Invalidate(id)` + `FlushDirty()`** 驱动的增量重算顺序

供 `VGEditorGalgameSequence` 内的 `SequenceDerivedState*` 装配使用，避免把图算法散落在 `SequencePresentationScheduler` 中。

## 2. 目录与 API

| 路径 | 说明 |
|------|------|
| `Include/ReactiveCore/DerivedStateGraph.h` | `DerivedStateId`、`DerivedStateGraph` |
| `Source/ReactiveCore/DerivedStateGraph.cpp` | Kahn 拓扑、脏传播、按拓扑调用 `ComputeFn` |

### 2.1 使用约定

- 所有节点需先 **`RegisterNode`** 再 `FlushDirty`；依赖 ID 若尚未注册则拓扑时忽略该边。
- **单线程**：与编辑器主线程一致，不做原子同步。
- 失效传播：`Invalidate` 某节点后，`FlushDirty` 会将 **依赖链上的下游** 一并标记为重算（闭包后再按拓扑执行）。

## 3. 与 VGEditorGalgameSequence 的接缝

Sequence 侧在 `Include/Reactive/DerivedState/` 定义业务 `SequenceDerivedStateId` 枚举，与 `ReactiveCore::DerivedStateId` 数值对齐；`SequenceDerivedStateGraph` 持有 `ReactiveCore::DerivedStateGraph` 并注册各 `ComputeFn`（内部调用 `ApplyIfStale`、`ApplyRuntimeOverlay` 等）。

## 4. 已知限制

- 无循环依赖检测（若成环则拓扑退化到节点注册顺序）。
- 无跨帧合并调度器（由 Sequence 的 presentation tick 决定何时 `FlushDirty`）。

## 5. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-12 | Phase 8 首批：新增 STATIC 库 `DerivedStateGraph` 与本文档。 |
