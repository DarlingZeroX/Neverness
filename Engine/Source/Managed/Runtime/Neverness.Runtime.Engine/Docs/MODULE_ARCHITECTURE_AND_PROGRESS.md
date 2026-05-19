# Neverness.Runtime.Engine — NNNativeEngineAPI 托管镜像

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Engine` |
| **命名空间** | `Neverness.Managed.Engine` |
| **职责** | `NNNativeEngineApi` 及子表结构体镜像、`NNNativeEngineApiConstants` |
| **不负责** | Bootstrap（**Neverness.Runtime.Interop**）、Gameplay |

## 2. 版本

- `NNNativeEngineApiConstants.LayoutVersion` — 当前 **6**（含 `NNEntityApi`、`NNApplicationApi`）

## 3. 边界

- 从 `NNNativeAPI.engineServices` 解析；安装由 `EngineNativeApiBootstrap`（Interop）完成
- 与 Native `EntitySubsystem` **无**自动数据同步

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | Phase 3 Engine Service ABI |
| **2026-05-19** | Bootstrap 迁至 Interop |
