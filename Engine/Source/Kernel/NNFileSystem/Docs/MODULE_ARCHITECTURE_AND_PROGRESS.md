# HFileSystem — 文件系统动态库

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 提供虚拟文件系统、打包头、生成器钩子等与 **文件 IO** 强相关的运行时实现；与 **HCorePlatform** 共享相似的 **Interface/Include 布局**（见 CMake `file(GLOB)`）。 |
| **不负责** | 不负责窗口与输入（**HCorePlatform**）；不负责引擎资源业务类型（**VGAsset**）。 |
| **CMake 目标** | `HFileSystem`（`SHARED`） |
| **依赖** | `HCore`（**PUBLIC**）。 |
| **宏** | `H_FILE_SYSTEM_EXPORT`（私有编译定义，用于 DLL 导出装饰）。 |
| **典型消费者** | **VGCore**、**VGUI**、**VGAsset**、**VGEngine**。 |

---

## 2. 构建与选项

- 包含目录：**PUBLIC** `Engine/Source/Kernel`（`VISIONGAL_KERNEL_ROOT`）、`Include`、`Interface`。
- 与 **HCorePlatform** 的源码清单模式同源（CMake 中复用 `HCORE_INTERFACE_FILES` 等变量命名）；以 [`CMakeLists.txt`](../CMakeLists.txt) 为准。

---

## 3. 目录结构（摘要）

```
Engine/Source/Kernel/HFileSystem/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Interface/
├── Include/                   ← FileWatcher、NativeFileDialog、Input 等与平台树同源布局
└── Source/
```

---

## 4. 使用说明

### 4.1 链接与包含

```cpp
#include <HFileSystem/Interface/HVirtualFileSystem.h>
```

链接 **`HFileSystem`** 时继承 **`HCore`** 的 **PUBLIC** 传递依赖。

### 4.2 与 HCorePlatform 的差异

二者 CMake 列表结构相近，但 **目标名与导出宏不同**；应用层应 **只链接所需目标**，避免符号与 DLL 边界混淆。

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
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Kernel 总览](../../KERNEL_ARCHITECTURE_AND_PROGRESS.md)
- [Runtime 总览](../../../Runtime/RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [HCore](../HCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
