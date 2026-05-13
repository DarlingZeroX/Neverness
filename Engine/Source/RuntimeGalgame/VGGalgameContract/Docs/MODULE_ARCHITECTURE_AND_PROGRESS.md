# VGGalgameContract — Phase 8 纯 ABI（INTERFACE）

## 1. 定位

| 项目 | 说明 |
|------|------|
| **职责** | 仅承载 **对外稳定虚接口** 与 **窄数据类型**（如 `SubsystemBusSnapshot`）；不包含 `SaveArchive` / `GalGameContext` / Lua 工厂实现。 |
| **CMake** | `INTERFACE` 库；`target_include_directories` 暴露本目录与 `Engine/Source/Runtime` 根，以支持 `VGGalgameContract/Interface/...` 与 `Interface/...` 两种包含风格。 |
| **依赖** | `INTERFACE` → `VGEngine`、`VGCore`。 |

## 2. 主要头文件

| 路径 | 说明 |
|------|------|
| `VGGalCoreConfig.h` | `VG_GALGAME_CORE_API`（由 **VGGalgameRuntimeCore** DLL 导出符号时定义 `VG_GALGAME_CORE_EXPORT`）。 |
| `Interface/IGameEngine.h` | `IGalGameEngine`；`SaveArchive` / `ArchiveDataContainer` **仅前向声明**。 |
| `Interface/IGalRuntimeSession.h` | Phase 8.2 运行时会话边界。 |
| `Interface/IExecutionScheduler.h` | Phase 8.3 执行调度骨架。 |
| `Interface/IRuntimeEventPipeline.h` | 事件管线占位。 |
| `Interface/ILuaRuntimeBridge.h` | Lua 受控入口占位。 |
| `Interface/IRuntimeLayerGraph.h` | 图层图占位。 |
| `Interface/IRuntimeSnapshotProvider.h` | 存档快照树占位。 |
| `Include/SubsystemBusSnapshot.h` | 总线快照窄数据。 |

## 3. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-13 | Phase 8.1：自 **`VGGalgameCore`** 拆出独立 **Contract** 目标。 |
