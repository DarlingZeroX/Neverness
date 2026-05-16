# VGEditorRuntimeBridge 模块架构与进展

## 1. 定位

承载 **与调试器/UI 解耦** 的运行时侧 **数据结构**：

- `SequenceRuntimeEventFrame`：单条结构化事件（种类、时间戳、条目索引、可选 payload）
- `SequenceRuntimeEventTimeline`：环形逻辑（按容量丢弃最旧帧）的内存时间线

**不**在本库直接 `#include` `SequenceDebuggerSession`，避免与 `VGEditorGalgameSequence` 形成 CMake 链接环；由 Sequence 模块在收到调试回调时 **填充** `SequenceRuntimeEventFrame` 并 `Push` 到时间线。

## 2. 构建形态

- **STATIC** 库，仅 STL；由 `VGEditorGalgameSequence` **PUBLIC** 链入并随 DLL/宿主分发。

## 3. 后续（P8-6）

- Variable inspect / dialogue state 等可在 Sequence 侧实现 `ISequenceRuntimeBridgePanel` 类（或等价），消费本模块时间线数据。

## 4. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-12 | Phase 8 首批：`SequenceRuntimeEventFrame`、`SequenceRuntimeEventTimeline` 与本文档。 |
