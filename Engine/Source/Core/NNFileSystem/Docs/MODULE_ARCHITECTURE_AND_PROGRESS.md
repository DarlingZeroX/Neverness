# NNFileSystem — 文件系统动态库

> 曾用名：**HFileSystem**；CMake 目标 **`NevernessCore-FileSystem`**。

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供虚拟文件系统、打包头、生成器钩子等与 **文件 IO** 强相关的运行时实现。 |
| **不负责** | 不负责窗口与输入（**NNPlatformCore**）；不负责引擎资源业务类型（**NNRuntimeAsset**）。 |
| **CMake 目标** | `NevernessCore-FileSystem`（`SHARED`） |
| **依赖** | `NevernessCore-Core`（**PUBLIC**）。 |
| **宏** | `H_FILE_SYSTEM_EXPORT`（历史宏名） |
| **典型消费者** | **NevernessRuntime-Core**、**NevernessRuntime-RmlUI**、**NevernessRuntime-Asset**、**NevernessRuntime-EngineLegacy**。 |

---

## 2. 构建与选项

- 包含目录：**PUBLIC** `Engine/Source/Core`（`VISIONGAL_KERNEL_ROOT`）、`Include`、`Interface`。

---

## 3. 目录结构（摘要）

```
Engine/Source/Core/NNFileSystem/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/
└── Source/
```

---

## 4. 使用说明

### 4.1 链接与包含

```cpp
#include <NNFileSystem/Interface/HVirtualFileSystem.h>
```

链接 **`NevernessCore-FileSystem`** 时继承 **`NevernessCore-Core`** 的 **PUBLIC** 传递依赖。

---

## 5. 接口与 API 文档（`Interface/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`HFileSystem.h`](../Interface/HFileSystem.h) | 文件系统总入口/工厂。 |
| [`HVirtualFileSystem.h`](../Interface/HVirtualFileSystem.h) | 虚拟路径、挂载点、打开文件抽象。 |
| [`HFileHeaders.h`](../Interface/HFileHeaders.h) | 包头、魔数、版本常量。 |
| [`HFileSystemGenerator.h`](../Interface/HFileSystemGenerator.h) | 生成器/工具链钩子。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-17 | 文档与 **NN/Neverness** 命名对齐（无行为变更）。 |
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Core 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [NNCore](../NNCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
