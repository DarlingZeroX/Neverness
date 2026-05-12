# VGEditorAuthoringGraph 模块架构与进展

## 1. 定位

承载 **Authoring 层** 数据，与 **线性 `SequenceDocument` / 运行时** 解耦：

- 节点位置、注释、可视化边（`SequenceAuthoringEdge`）
- 通过 `entryIndex` 引用序列条目，**不**引入执行 Pin、分支 VM 或 Graph Compiler

线性执行语义仍由 `VGGalgameScriptSequence` 侧保证；本模块仅服务编辑器可视化与布局持久化预留。

## 2. 类型一览

| 类型 | 说明 |
|------|------|
| `SequenceAuthoringNode` | `nodeId` + `entryIndex` + `posX/Y` |
| `SequenceAuthoringEdge` | 作者图有向边（展示用） |
| `SequenceAuthoringComment` | 画布注释 |
| `SequenceAuthoringGraph` | 容器与 `EnsureNodeForEntry` 等 API |

## 3. 依赖

- **STATIC** 库，**PUBLIC** 链接 `VGCore`（预留与引擎 UUID/资源路径协同；当前实现以 `uint64_t` 为主键）。

## 4. 与 VGEditorGalgameSequence 的接缝

`SequenceGraphProjection` / `SequenceGraphWidget` 在合并读模型时：对每个文档条目 `EnsureNodeForEntry`；拖拽节点调用 `SetNodePositionForEntry` 或 `SetNodePosition`；边/注释由后续命令与序列化接入（Phase 8 持续演进）。

## 5. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-12 | Phase 8 首批：`SequenceAuthoringGraph` 实现与本文档。 |
