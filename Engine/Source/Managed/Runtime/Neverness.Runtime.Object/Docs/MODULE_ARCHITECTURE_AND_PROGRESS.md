# Neverness.Runtime.Object — 托管对象与 Native Handle

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Object` |
| **命名空间** | `Neverness.Managed.Object` |
| **职责** | `VGObject`、`LifetimeSystem`、`ObjectRegistry`；经 **Interop** 的 `NativeHandleBridge` 操作 Native 控制代码 |
| **不负责** | Interop 安装（**Neverness.Runtime.Interop**） |

## 2. 生命周期

- `LifetimeSystem.CreateAndRegister` → Native `createObject`（ref=1）
- `Dispose` → retain/release 至 0 后 destroy

## 3. 依赖

- `Neverness.Runtime.Engine`
- `Neverness.Runtime.Interop`

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-15** | Phase 5 地基 |
| **2026-05-19** | `NativeHandleBridge` 迁至 Interop |
