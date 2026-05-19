# NNRuntimeManagedBridge — Native ↔ Managed Tick 桥接

## 1. 定位

| 项目 | 说明 |
|------|------|
| **CMake 目标** | `NevernessRuntime-ManagedBridge`（STATIC） |
| **职责** | `NNEngineRuntimeHost_SetManagedTickCallback` / `NNEngineRuntimeHost_TickManaged` |
| **不负责** | CoreCLR 启动 |

## 2. 用法

1. 宿主解析 `Entry.RuntimeTick` UCO
2. `NNEngineRuntimeHost_SetManagedTickCallback(fn)`
3. 每帧：`NNEngineRuntimeHost_Tick(dt)` → `NNEngineRuntimeHost_TickManaged(dt)`

## 3. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-19** | Phase 6a 落地；Legacy Host 提供 `VGManagedHost_BootstrapDefaultRuntime` |
