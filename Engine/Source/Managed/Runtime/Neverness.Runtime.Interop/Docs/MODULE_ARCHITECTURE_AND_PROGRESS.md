# Neverness.Runtime.Interop — Native ↔ Managed 互操作

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Interop` |
| **命名空间** | `Neverness.Managed.Interop` |
| **职责** | `NativeApiBootstrap`、`EngineNativeApiBootstrap`、`NativeHandleBridge`、`RuntimeVersionInfo` |
| **禁止** | `DllImport` 直调引擎；仅经函数表间接调用 |
| **不负责** | ABI 结构体定义（**Core** / **Engine**） |

## 2. 安装顺序（`RuntimeInitializer`）

1. `NativeApiBootstrap.Install(table)`
2. `EngineNativeApiBootstrap.InstallFromNativeApiTable(table)`
3. 生产路径**不**调用 `ExerciseStubInteropPath`（仅测试）

## 3. 依赖

- `Neverness.Runtime.Core`
- `Neverness.Runtime.Engine`

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-19** | M-2：自 Core/Engine/Object 迁出 |
