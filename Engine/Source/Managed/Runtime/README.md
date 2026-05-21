# Neverness Managed Runtime

## 模块边界

- **Runtime** 程序集只做 C ABI 薄封装（`Engine` 结构体镜像 + `*NativeBridge`），不得在 C# 侧实现第二套对象模型、自定义反射注册表或 Gameplay 状态机。
- **Editor** 程序集承载 Inspector、Undo/Redo、场景 JSON 序列化与 `Neverness.Editor.Reflection` 工具反射。
- 新能力流程：C 头 `NN*API` → `Neverness.Runtime.Engine` 镜像 → `Interop` 安装 → 各模块 static bridge。

## 当前程序集

| 程序集 | 职责 |
|--------|------|
| `Neverness.Runtime.Engine` | `NNNativeApi`、`NNNativeEngineApi`、句柄、`EngineTime`、`EngineNativeApiCache` |
| `Neverness.Runtime.Interop` | Bootstrap、API 表安装 |
| `Neverness.Runtime.Bootstrap` / `Host` | 启动链 |
| `Neverness.Runtime.RuntimeLoop` | 托管帧循环 |
| `Neverness.Runtime.Assets` / `Scene` / `Serialization` / `Scripting` | ABI 薄壳 |
| `Neverness.Editor.*` | 编辑器与工具链（不得被 `Host` 传递依赖） |

## Editor 与 Native 表

- **Runtime 程序集**（`Bootstrap` / `Interop`）禁止 `DllImport`；仅通过已安装的 `NNNativeAPI*` 函数表调用 Native。
- **NervernessEditor** 可在进程启动时用 `NativeLibrary` 加载 `NevernessRuntime-Managed.dll` 并解析 `NNNativeApi_GetDefaultTable`（见 [NervernessEditor/README.md](../Editor/NervernessEditor/README.md)）。

## 已移除（勿回潮）

`Neverness.Runtime.Object`、`Reflection`、`Gameplay`、`Graph`、`Core`、`EngineRuntime` 独立程序集。
