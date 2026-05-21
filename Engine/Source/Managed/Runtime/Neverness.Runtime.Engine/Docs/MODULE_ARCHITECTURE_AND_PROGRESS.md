# Neverness.Runtime.Engine

## 职责

单一引擎 ABI 镜像程序集（已合并原 `Neverness.Runtime.Core` 与 `Neverness.Runtime.EngineRuntime`）：

- `Neverness.Managed.Core` — `NNNativeApi`、`NNNativeApiConstants`
- `Neverness.Managed.Engine` — `NNNativeEngineApiTypes`、句柄、`EngineNativeApiCache`
- `Neverness.Managed.Engine.Runtime` — `EngineTime`

## 依赖

- 无其他 Runtime 项目引用；`Neverness.Runtime.Interop` 引用本程序集并调用 `EngineNativeApiCache.Install`。

## 进度

- Layout version **10**（`NNNativeEngineApiConstants.LayoutVersion`；含 `NNVfsApi` 等）
- `NNSceneAPI` 已追加 `serializeScene` / `deserializeScene` 字段（Stub 默认 no-op）
