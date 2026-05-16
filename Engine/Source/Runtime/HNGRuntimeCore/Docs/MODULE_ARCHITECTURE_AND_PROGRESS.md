# HNGRuntimeCore — 节点图运行时核心

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供节点图运行时核心数据结构：`RuntimeGraph`、`RuntimeContext`、槽/边模型、**Value** 与类型枚举、节点执行函数签名（`NodeExecuteFn`）等；命名空间 **`Horizon::NodeGraphRuntime`**。 |
| **不负责** | 不负责具体节点类型的 `Execute` 实现（见 **HNGRuntimeNodes**）；不负责编辑器 UI（见 Editor 模块）。 |
| **CMake 目标** | `HNGRuntimeCore`（`SHARED`） |
| **宏** | `H_NODE_GRAPH_API_EXPORT` |
| **依赖** | `SDL3::SDL3`（PRIVATE）、`HCore`（PRIVATE）。 |
| **典型消费者** | **HNGRuntimeNodes**（**PUBLIC** 链接 `HNGRuntimeCore`）、编辑器/工具链。 |

---

## 2. 构建与选项

- 包含目录：**PRIVATE** `Engine/Source/Runtime`、`Include`、`Interface`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/HNGRuntimeCore/
├── CMakeLists.txt
├── HNodeGraphConfig.h
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
│   ├── RuntimeCore.h          ← RuntimeGraph / RuntimeSlot / RuntimeEdge 等
│   ├── Types.h                ← ExecResult、NODE_ID、SlotType 等
│   └── Value.h                ← 运行时值模型
├── Include/
│   └── RuntimeContext.h       ← 执行上下文
└── Source/
```

---

## 4. 使用说明

### 4.1 包含路径

```cpp
#include <HNGRuntimeCore/Interface/RuntimeCore.h>
#include <HNGRuntimeCore/Include/RuntimeContext.h>
```

**HNGRuntimeNodes** 通过相对路径包含 `../../HNGRuntimeCore/...`（见该模块头文件）；建议新代码统一经 Runtime 根包含目录。

### 4.2 执行模型

- **`NodeExecuteFn`**：`ExecResult (*)(RuntimeContext& ctx, NODE_ID nodeIndex)`。
- **`ExecResult`**：表示节点是否仍在运行（详见 [`Types.h`](../Interface/Types.h)）。
- **槽与边**：`RuntimeSlot`、`RuntimeEdge` 描述数据流与控制流（见 [`RuntimeCore.h`](../Interface/RuntimeCore.h) 注释）。

---

## 5. 接口与 API 文档（关键类型）

| 符号 / 文件 | 说明 |
|-------------|------|
| `RuntimeGraph` | 图结构：节点、槽集合、邻接关系（见 `RuntimeCore.h`）。 |
| `RuntimeContext` | 每次 tick/步进时的执行上下文（`RuntimeContext.h`）。 |
| `Value` | 动态类型值盒子（`Value.h`）。 |
| `NODE_ID` / `SLOT_ID` | 节点与槽的稳定索引类型（`Types.h`）。 |

完整字段与不变量以头文件内注释为准。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版：执行模型与目录索引。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [HNGRuntimeNodes](../HNGRuntimeNodes/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
