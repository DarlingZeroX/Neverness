# Neverness.Runtime.Bootstrap — 托管 Runtime 启动与主循环

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Bootstrap` |
| **命名空间** | `Neverness.Managed.Bootstrap` |
| **职责** | `RuntimeBootstrap`、`RuntimeInitializer`、`RuntimeMainLoop`；对接 Native `Entry` 与可选 `Neverness.Runtime.App` |
| **不负责** | Interop 实现（**Neverness.Runtime.Interop**）、Native Kernel |

## 2. 关键类型

| 类型 | 说明 |
|------|------|
| `RuntimeBootstrap` | 进程级 `Start` / `Shutdown`；`GetPackedApiVersion` |
| `RuntimeInitializer` | Interop 安装、子系统注册、`InitializeRegistered` |
| `RuntimeMainLoop` | 单帧 `Tick`；包装 `Neverness.Runtime.RuntimeLoop.RuntimeLoop` |
| `NativeBootstrapContext` | `NativeApiTable`、运行模式（`NativeDriven` / `ManagedOuterLoop`） |

## 3. 依赖

- `Neverness.Runtime.Interop`
- `Neverness.Runtime.RuntimeLoop`

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-19** | M-1 落地；C# 主导启动路径确立 |
