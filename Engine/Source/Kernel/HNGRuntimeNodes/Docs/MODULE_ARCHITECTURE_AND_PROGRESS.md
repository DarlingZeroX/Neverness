# HNGRuntimeNodes — 节点图运行时节点实现库

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 在 **HNGRuntimeCore** 提供的 `RuntimeContext` / 图模型之上，实现各类内置节点的 **`Execute`** 函数（如 Entry、Dialogue、Branch 等）；头文件 [`HNGRuntimeNodes.h`](../Include/HNGRuntimeNodes.h) 为对外入口。 |
| **不负责** | 不负责图编译与编辑器侧资产格式（见 Editor 与工具链文档）。 |
| **CMake 目标** | `HNGRuntimeNodes`（`SHARED`） |
| **宏** | `H_NG_RUNTIME_NODES_API_EXPORT`（`HNGRuntimeNodesConfig.h`） |
| **依赖** | **PUBLIC** `HNGRuntimeCore`；包含路径需能访问 **HNGRuntimeCore** 头（CMake 注释：`访问 HNGRuntimeCore 头`）。 |
| **典型消费者** | 运行时宿主、Legacy/编辑器桥接（依产品配置）。 |

---

## 2. 构建与选项

- `target_include_directories`：**PUBLIC** `Engine/Source/Kernel`（`VISIONGAL_KERNEL_ROOT`）、`Include`、**模块根目录**（用于 `HNGRuntimeNodesConfig.h` 等直包）。

---

## 3. 目录结构

```
Engine/Source/Kernel/HNGRuntimeNodes/
├── CMakeLists.txt
├── HNGRuntimeNodesConfig.h
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/
│   └── HNGRuntimeNodes.h      ← 对外 API：各节点 Execute 声明
└── Source/
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include <HNGRuntimeNodes.h>
// 或依据目标 include 路径：
#include "HNGRuntimeNodes/Include/HNGRuntimeNodes.h"
```

具体前缀以链接 **`HNGRuntimeNodes`** 的目标的 `target_include_directories` 为准。

### 4.2 节点 Execute 约定

头文件中典型模式：

```cpp
H_NG_RUNTIME_NODES_API ExecResult SomeNodeExecute(RuntimeContext& ctx, NODE_ID nodeIndex);
```

- 返回 **`ExecResult`** 以指示是否挂起等待、是否完成本步等（语义与 **HNGRuntimeCore** 一致）。
- **`DialogueState`** 等状态结构体与 **Dialogue** 节点行为绑定（见 [`HNGRuntimeNodes.h`](../Include/HNGRuntimeNodes.h)）。

---

## 5. 接口与 API 文档（摘录）

以下符号均在 [`Include/HNGRuntimeNodes.h`](../Include/HNGRuntimeNodes.h) 中声明（以源码为准）：

| 符号 | 说明 |
|------|------|
| `EntryNodeExecute` | 无状态入口，推进到下一节点。 |
| `DialogueNodeExecute` | 多行对白与 Flow 等待。 |
| `BranchNodeExecute` | 条件分支激活 True/False 路径。 |
| `DialogueState` | 对白节点私有状态载体。 |

（头文件中若还有其它 `*NodeExecute`，请同步维护本表。）

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Kernel 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [HNGRuntimeCore](../HNGRuntimeCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
