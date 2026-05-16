# VGPackage — 资源包格式与 IO

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 读写 **Pak** 包体：`PakReader`/`PakWriter`、CRC、虚拟路径 **`PackageFileSystem`** 等；头文件位于 **`Include/`**（无独立 `Interface/` 目录）。 |
| **不负责** | 不负责通用 OS 文件对话框（**HCorePlatform**）；不负责游戏资产类型系统（**VGAsset**）。 |
| **CMake 目标** | `VGPackage`（`SHARED`） |
| **依赖** | `HCore`（PRIVATE）。 |
| **典型消费者** | **VGCore**、**VGEngine**。 |

---

## 2. 构建与选项

- 包含目录：**PRIVATE** `Engine/Source/Runtime`、`Engine/Source/Kernel`、`Include`（见 [`CMakeLists.txt`](../CMakeLists.txt)）。

---

## 3. 目录结构

```
Engine/Source/Runtime/VGPackage/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/
│   ├── PakFormat.h
│   ├── PakReader.h
│   ├── PakWriter.h
│   ├── PakCrc32.h
│   ├── PackageFile.h
│   ├── PackageFileSystem.h
│   └── Config.h
└── Source/
```

---

## 4. 使用说明

### 4.1 包含方式

```cpp
#include "VGPackage/Include/PackageFileSystem.h"
#include "VGPackage/Include/PakReader.h"
```

（示例取自 **VGCore** `VFS.cpp`；须已包含 **`Engine/Source/Runtime`** 作为基路径。）

### 4.2 与虚拟文件系统

`PackageFileSystem` 将包内路径映射到读取接口，常与 **HFileSystem** / **HCore** VFS 组合使用。

---

## 5. 接口与 API 文档（`Include/`）

| 头文件 | 主要职责 |
|--------|----------|
| [`PakFormat.h`](../Include/PakFormat.h) | 魔数、版本、目录表布局常量。 |
| [`PakReader.h`](../Include/PakReader.h) | 只读打开、查找、解压/映射（以实现为准）。 |
| [`PakWriter.h`](../Include/PakWriter.h) | 构建包、追加条目。 |
| [`PakCrc32.h`](../Include/PakCrc32.h) | 校验和工具。 |
| [`PackageFile.h`](../Include/PackageFile.h) | 单文件条目描述。 |
| [`PackageFileSystem.h`](../Include/PackageFileSystem.h) | 挂载包并提供类文件 API。 |
| [`Config.h`](../Include/Config.h) | 模块配置宏。 |

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGCore](../VGCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
