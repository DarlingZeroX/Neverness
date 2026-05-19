# Neverness.Runtime.Scene — Native 场景 ABI 门面

## 1. 定位

| 项目 | 说明 |
|------|------|
| **程序集** | `Neverness.Runtime.Scene` |
| **命名空间** | `Neverness.Managed.Scene` |
| **职责** | `Scene`、`SceneEntity`、`SceneNativeBridge`；经 `NNSceneAPI` 访问 C++ 场景图 |
| **不负责** | 场景存储与 ECS 迭代（**NNRuntimeScene** / Kernel）；托管不复制实体数据 |

## 2. 与 Native 边界

- **运行时**：`SceneNativeBridge` → `EngineNativeApiBootstrap.EngineApi.Scene`（`loadScene` / `spawn` / `destroy` / `find` 等）
- **句柄**：`SceneEntity` 持有 `NNEntityHandle`（与 `NNObjectHandle` / `VGObject` 分离）
- **JSON**：`SceneSerializer` 为工具/存档 DTO；再水合经 Native `spawn` 创建新句柄，不复活旧控制码
- **EntityAPI**：`NNEntityAPI`（Kernel 子系统 Tick）与场景图 API 语义分离，见 MANAGED 总览 §2.7.1

## 3. 依赖

- `Neverness.Runtime.Interop`
- `Neverness.Runtime.Engine`
- `Neverness.Runtime.Reflection`
- `Neverness.Runtime.Serialization`

## 4. 开发进展

| 日期 | 进展 |
|------|------|
| **2026-05-15** | Phase 5.3 JSON 再水合（Object 路径） |
| **2026-05-19** | 删除 `Neverness.Runtime.Entity`；Scene 改为 Native ABI 薄门面；Stub `spawn` 返回非零句柄供测试 |
