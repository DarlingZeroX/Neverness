# Neverness.Runtime.Core — NNNativeAPI 托管镜像

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Core` |
| **命名空间** | `Neverness.Managed.Core` |
| **职责** | `NNNativeApi`、`NNNativeApiConstants`（镜像 `NativeAPI.h`） |
| **不负责** | Bootstrap 安装（**Neverness.Runtime.Interop**）、Native 表实现（**NNRuntimeManaged**） |

## 2. 主要类型

- `NNNativeApi` — 与 Native `NNNativeAPI` 逐字段对齐
- `NNNativeApiConstants.ApiVersion` — 当前 **2**

## 3. 边界

- Native 经 `NNNativeApi_GetDefaultTable()` 提供表指针
- `Entry.Bootstrap` → `RuntimeInitializer` → `NativeApiBootstrap.Install`（Interop 程序集）

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-14** | Phase 2 ABI 镜像 |
| **2026-05-19** | Bootstrap 迁至 Interop；Core 仅保留镜像 |
