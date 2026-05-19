# Neverness.Runtime.Gameplay — Galgame 产品逻辑（100% Managed）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Gameplay` |
| **命名空间** | `Neverness.Managed.Gameplay` |
| **职责** | 变量表、序列、`SequenceRunner`、`GameplaySessionSnapshot`、`DialoguePresenter` |
| **不负责** | Native Gameplay/存盘 ABI（待定）；CoreCLR 宿主 |

> **C# 主导**：Gameplay 产品逻辑**仅**在 Managed；C++ Kernel 不提供 Galgame 序列/存盘实现。

## 2. 主要 API

- `GameplayVariableStore` — JSON 往返
- `SequenceRunner` / `Advance` — 序列与等待步
- `GameplaySessionSnapshot` — 会话快照
- `DialoguePresenter` — 经 Engine UI 子表（Stub）

## 3. 依赖

- `Neverness.Runtime.Scene`、`Serialization`、`Interop`

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-15** | Phase 6 slice 2～5 |
| **2026-05-19** | `Entry` 演练迁至 `GameplayBootstrapDrillTests` |
