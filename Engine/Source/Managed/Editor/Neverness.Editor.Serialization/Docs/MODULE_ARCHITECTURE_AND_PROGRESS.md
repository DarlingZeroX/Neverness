# Neverness.Editor.Serialization

## 职责

Editor/工具链专用 JSON 序列化（自 Runtime 迁出）：

- `SceneSerializer` — 场景 DTO + `ReflectionRegistry` 属性捕获
- `SceneRehydrator` — DTO → Native spawn 再水合
- `AssetSerializer`、`VersionTolerance`

## 依赖

- `Neverness.Editor.Reflection`
- `Neverness.Runtime.Scene`、`Neverness.Runtime.Interop`

## 边界

Runtime `Neverness.Runtime.Serialization` 仅保留 `NNSceneSerializeBridge` 等 ABI 薄封装，不含 JSON 世界模型。
