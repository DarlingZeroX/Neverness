# NNCore — 通用原生基础库

> 曾用名：**HCore**；CMake 目标 **`NevernessCore-Core`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 数学（含 vendored **GLM**）、元数据、JSON/XML/INI、事件总线、场景基础类型、VFS 抽象、工具函数等 **无平台 UI** 的通用基建。 |
| **不负责** | SDL 窗口/输入（见 **NNPlatformCore**）；FFmpeg（见 **NNMediaCore**）；节点图执行（见 **NNNodeGraphCore**，默认未构建）。 |
| **CMake 目标** | `NevernessCore-Core`（`STATIC`） |
| **依赖** | `SDL3::SDL3`（**PUBLIC**） |
| **典型消费者** | 全部 **Core** 模块与 **NevernessRuntime-\*** 栈。 |

---

## 2. 构建与选项

- `target_include_directories(NevernessCore-Core PUBLIC ${VISIONGAL_KERNEL_ROOT})`，以及 **PRIVATE** `Include`、`Interface`。
- **`VISIONGAL_KERNEL_ROOT`** 物理路径为 `Engine/Source/Core`。
- 生成代码目录：`.Generated/`（事件、系统等，见 `CMakeLists.txt` `GLOB`）。

---

## 3. 目录结构（摘要）

```
Engine/Source/Core/NNCore/
├── CMakeLists.txt
├── CoreModuleDefinitions.h
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/          ← Event、File、Math/GLM、Scene、VFS、Utils 等
├── Source/
└── .Generated/       ← 生成源（若已运行代码生成）
```

---

## 4. 使用说明

### 4.1 包含路径

```cpp
#include <NNCore/Include/...>
```

链接 **`NevernessCore-Core`** 即可通过 **PUBLIC** `VISIONGAL_KERNEL_ROOT` 解析 `<NNCore/...>` 前缀。

---

## 5. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-16 | 迁入 **Core**；文档首版。 |

---

## 6. 相关链接

- [Core 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNPlatformCore](../NNPlatformCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
