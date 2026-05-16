# VGNativeEngineAPI — Native Engine Service ABI

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 承载 **Engine Runtime 服务层** 的 C 可互操作 **函数表聚合**（`VGNativeEngineAPI`）：Render、UI、Audio、Asset、Input、Scene、Timing、AsyncWait、Object、AssetRegistry、**Entity（Kernel 首包：魔数 + `getRuntimeTick` 观测）**。提供 **Stub** 实现与测试计数；不包含 Gameplay、对白、变量、存档、Sequence、Editor 产品逻辑。 |
| **不负责** | 不链接 **VGEngine** / **VGRHI** / **VGUI**。真实能力由未来 Adapter 覆写函数指针；**Timing / Async / Scene / Asset（纹理与音频快捷项）/ Object / AssetRegistry** 的 Runtime 转发由 **VGEngineRuntimeServices** 在可选 CMake 路径下覆写。 |
| **CMake 目标** | `VGNativeEngineAPI`（`STATIC`） |
| **依赖** | 仅 C++ 标准库；`target_include_directories(... PUBLIC Include)`。 |
| **典型消费者** | **VGEngineRuntime**（PUBLIC 链接以使用 Handle 等头）、**VGEngineRuntimeServices**、**VGManagedCore**（经 `engineServices` 指针挂载表）。 |

---

## 2. 构建与选项

| 项目 | 说明 |
|------|------|
| 导出宏 | [`VGNativeEngineApiConfig.h`](../Include/VGNativeEngineAPI/VGNativeEngineApiConfig.h) 中 `VG_NATIVE_ENGINE_API`；当前为静态库时通常为空。 |
| 调用约定 | [`NativeInterop.h`](../Include/VGNativeEngineAPI/NativeInterop.h)：`VG_ENGINE_ABI_STDCALL`（Windows 上为 `__stdcall`，与托管 `UnmanagedCallersOnly` 对齐）。 |

根 [`CMakeLists.txt`](../../../../../CMakeLists.txt) 中 **`VISIONGAL_USE_ENGINE_RUNTIME_SERVICES`**（默认 ON）决定 **VGManagedCore** 使用 `VGNativeEngineApi_GetRuntimeTable()` 或纯 Stub 的 `VGNativeEngineApi_GetDefaultTable()`，详见 [VGEngineRuntimeServices 文档](../../VGEngineRuntimeServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)。

---

## 3. 目录结构

```
Engine/Source/Runtime/VGNativeEngineAPI/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/VGNativeEngineAPI/
│   ├── NativeInterop.h
│   ├── EngineHandles.h
│   ├── EngineTypes.h
│   ├── RenderAPI.h
│   ├── UIAPI.h
│   ├── AudioAPI.h
│   ├── AssetAPI.h
│   ├── AssetRegistryAPI.h
│   ├── InputAPI.h
│   ├── ObjectAPI.h
│   ├── SceneAPI.h
│   ├── TimingAPI.h
│   ├── AsyncWaitAPI.h
│   ├── EntityAPI.h
│   ├── EngineAPIRegistry.h
│   ├── NativeEngineAPI.h
│   └── VGNativeEngineApiConfig.h
└── Private/
    └── VGNativeEngineApiStubs.cpp
```

---

## 4. 使用说明

### 4.1 包含方式

在已链接 `VGNativeEngineAPI` 的目标中：

```cpp
#include "VGNativeEngineAPI/NativeEngineAPI.h"
#include "VGNativeEngineAPI/EngineAPIRegistry.h"
```

- 聚合根类型与建表 API 在 **`EngineAPIRegistry.h`** / **`NativeEngineAPI.h`**。
- 子域函数指针 typedef 见各 `*API.h`。

### 4.2 表构建与版本

1. 分配或栈上准备 `VGNativeEngineAPI table{}`。
2. 调用 **`VGNativeEngineApiTable_BuildDefault(&table)`** 填充 Stub 指针，并设置 **`table.layoutVersion = VG_NATIVE_ENGINE_API_LAYOUT_VERSION`**（由 `BuildDefault` 负责清零与填充，以实现为准）。
3. 将 **`const VGNativeEngineAPI*`** 挂到 **VGManagedCore** 的 `VGNativeAPI.engineServices`（或通过 **VGEngineRuntimeServices** 的 `VGNativeEngineApi_GetRuntimeTable()`）。

**布局演进规则**：仅允许在各子表尾部或聚合体尾部 **追加** 字段；禁止重排既有字段；破坏性变更时递增 **`VG_NATIVE_ENGINE_API_LAYOUT_VERSION`**（须与托管 `VGNativeEngineApiConstants.LayoutVersion` 对齐）。

### 4.3 字符串与线程

- 文档约定中的 **`const char*`** 参数须为 **NUL 结尾的 UTF-8**；`nullptr` 多为 no-op 或失败返回。
- **Render** 等子表：Stub 与默认 Adapter 假定为 **单线程或 RHI 单点** 调用；并发行为由具体实现定义。

### 4.4 与托管栈的关系

参见 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) 与程序集 **VisionGal.Managed.Engine**：托管侧镜像本 ABI 的函数表布局与 Handle 类型。

---

## 5. 接口与 API 文档

### 5.1 根符号（`extern "C"` 导出策略）

| 符号 | 说明 |
|------|------|
| `VG_NATIVE_ENGINE_API_LAYOUT_VERSION` | 当前聚合体内存布局版本（见 `EngineAPIRegistry.h`，**以源码为准**，**layout v4** 起含 **`VGEntityAPI entity`**；**layout v5** 起 **`VGEntityAPI`** 尾部追加 **`getRuntimeTick`**）。 |
| `VGNativeEngineAPI` | 根聚合体，含 `layoutVersion`、`reserved0` 与各子表。 |
| `VGNativeEngineApiTable_BuildDefault` | 将 `outTable` 清零后填入 Stub 函数指针。 |
| `VGNativeEngineApi_GetDefaultTable` | 进程内单例只读表指针（纯 Stub）。 |
| `VGNativeEngineApi_GetStubInvokeCount` | 测试：Stub 被调用累计次数。 |

**政策**：模块边界仅导出上述建表/取表/诊断符号；业务能力一律经 **函数表字段** 间接调用。

### 5.2 Handle 类型（`EngineHandles.h`）

| typedef | 说明 |
|---------|------|
| `VGTextureHandle` | 纹理不透明 ID，`0` 无效。 |
| `VGRenderTargetHandle` | 渲染目标。 |
| `VGElementHandle` | UI 元素。 |
| `VGAudioHandle` | 音频。 |
| `VGAssetHandle` | 通用资源。 |
| `VGAsyncWaitHandle` | 异步等待。 |
| `VGEntityHandle` | 场景实体 / Prefab 实例。 |
| `VGObjectHandle` | 托管 `VGObject` 与 Native 桥接。 |

### 5.3 POD 类型（`EngineTypes.h`）

| 类型 | 字段摘要 |
|------|----------|
| `VGGuid` | `high`、`low`（128-bit GUID）。 |
| `VGTransform3` | `position[3]`、`rotation[3]`、`scale[3]`。 |

### 5.4 `VGNativeEngineAPI` 聚合体字段顺序

见 [`EngineAPIRegistry.h`](../Include/VGNativeEngineAPI/EngineAPIRegistry.h)：

- `layoutVersion`、`reserved0`
- `render`、`ui`、`audio`、`asset`、`input`、`scene`、`timing`、`asyncWait`
- `object`、`assetRegistry`
- **`entity`**（`EntityAPI.h`：**`VGEntityAPI`** — **`getServiceAbiToken`** 魔数校验；**`getRuntimeTick`**（layout v5+）观测 **VGEngineRuntime** 是否已驱动 **EntitySubsystem**；与 **`VGSceneAPI`** 所用 **`VGEntityHandle`** 语义分离，见头文件注释）

### 5.5 子表字段摘要

**Timing（`TimingAPI.h`）**

| 字段 | 签名意图 |
|------|----------|
| `getDeltaTime` | 上一帧 Δt（秒）。 |
| `getTotalTime` | 自 Runtime `Initialize` 起累计时间。 |
| `getFrameIndex` | 已执行 Tick 次数（单调）。 |

**AsyncWait（`AsyncWaitAPI.h`）**

| 字段 | 说明 |
|------|------|
| `createWait` | 创建等待对象；Stub 可为“已完成”语义。 |
| `tryComplete` | 轮询：1 完成；0 未完成或已消费（以 Stub 文档为准）。 |
| `releaseWait` | 释放；对 `0` 或未知 handle 须 no-op。 |

**Scene（`SceneAPI.h`）**

| 字段 | 说明 |
|------|------|
| `loadScene` / `unloadScene` / `getActiveSceneName` | 场景生命周期与名称查询。 |
| `spawn` / `destroy` / `find` / `activate` | 实体 CRUD 与激活。 |
| `setParent` / `getParent` / `getChildCount` / `getChildAt` | 层级。 |
| `getTransform` / `setTransform` | 变换读写。 |
| `setEntityName` / `getEntityName` | 命名。 |

**Asset（`AssetAPI.h`）**

| 字段 | 说明 |
|------|------|
| `loadAsset` / `unloadAsset` | 通用资源。 |
| `loadTexture` / `loadAudio` | 快捷加载；未接线时可返回 `0`。 |

**Object（`ObjectAPI.h`）**

| 字段 | 说明 |
|------|------|
| `createObject` / `destroyObject` | 生命周期。 |
| `retainObject` / `releaseObject` / `getRefCount` | 引用计数。 |
| `isAlive` | 是否仍有效。 |
| `getTypeName` | 写入 `outUtf8`，返回字节数或 `-1`。 |

**AssetRegistry（`AssetRegistryAPI.h`）**

| 字段 | 说明 |
|------|------|
| `registerAsset` / `unregisterByGuid` / `unregisterByPath` | 登记与注销。 |
| `resolvePathByGuid` / `resolveGuidByPath` | GUID ↔ 路径。 |
| `getDependencyCount` / `getDependencyAt` | 依赖列表。 |
| `importAsset` | 导入并返回 `VGGuid`。 |

**Entity（`EntityAPI.h`，layout v4+）**

| 字段 | 说明 |
|------|------|
| `getServiceAbiToken` | 返回 **`VG_ENTITY_SERVICE_ABI_TOKEN`**（与托管 **`VGNativeEngineApiConstants.EntityServiceAbiToken`** 对齐）；Stub 累加 **`GetStubInvokeCount`**；**非** ECS 实体分配 API。 |
| `getRuntimeTick` | **layout v5** 尾部追加；返回单调 **`runtimeTick`**（**`VGEngineRuntime::Tick`** 驱动 **EntitySubsystem**）；Stub 返回 `0`；**非**托管 **EntityWorld** 镜像语义。 |

**Render / UI / Audio / Input**：见对应 `*API.h`（Stub 阶段以计数与 no-op 为主）。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-14 | Phase 3：模块落地、子 API 头拆分、Stub、`GetStubInvokeCount`；托管路径验证 Stub 计数。 |
| 2026-05-14 起 | Phase 4+：`layoutVersion` 与 Scene/Asset/Timing 等子表扩展；Runtime 转发见 **VGEngineRuntimeServices**。 |
| 2026-05-15 | 文档改为简体中文；对齐当前头文件中的 **`VG_NATIVE_ENGINE_API_LAYOUT_VERSION`** 与 **layout v3** 子表（Object、AssetRegistry、Scene 扩展字段等）。 |
| 2026-05-16 | 文档交叉：托管 **Phase 6 slice 5**（序列分支、**Advance** 可恢复等待）为纯 C#，**不**变更本模块 ABI；后续 **VGEntityAPI** 拟单独评审纳入（见 [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.2**、MANAGED **§2.7**）。 |
| 2026-05-18 | **规划索引**：**Phase 6+ Native**（Gameplay／存档）推进顺序见 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§5.1**；本模块负责子表与 **layoutVersion** 演进。 |
| 2026-05-19 | **SceneAPI 对照**：**SceneAPI.h** 已暴露 **VGEntityHandle**（场景实体），与托管 **EntityHandle**（纯 C#）语义独立；专用 **VGEntitySystem** 子表仍为 P0 下一跳（见 MANAGED **§2.7.1**）；本轮无 layout 变更。 |
| 2026-05-20 | **规划索引（续）**：托管 **EntityWorld.GetComponentCount** 为 C# 侧统计 API；本模块仍无 **VGEntityAPI** 子表变更。 |
| 2026-05-21 | **托管索引**：**EntityWorld** 非泛型 **Type** 键查询见 MANAGED **§2.5.4**；仍为纯 C#，**VGEntityAPI** 未纳入本模块。 |
| 2026-05-15 | **layout v4**：新增 **`EntityAPI.h`**、聚合体尾部 **`VGEntityAPI entity`**、**`getServiceAbiToken`** Stub；**`VG_ENTITY_SERVICE_ABI_TOKEN`**；**SceneAPI.h** / **EngineHandles.h** 与 **`EntityAPI.h`** 交叉说明 **VGEntityHandle** vs 托管 **EntityHandle**；见 MANAGED **§2.7.1**。 |
| 2026-05-15 | **layout v5**：**`getRuntimeTick`**；**`VG_NATIVE_ENGINE_API_LAYOUT_VERSION` → 5**；Stub 与 **VGEngineRuntimeServices::BuildRuntime`** 覆写 **`entity.*`** 对齐 MANAGED **§2.7.1 Kernel 首包**。 |
| 2026-05-15 | **P0 對齊審計**：與 MANAGED **§2.7.1** 敘述一致；**MERGED** 由 **merge_docs.py** 再生同步；無新增 ABI 欄位。 |
| 2026-05-15 | **§0 索引**：全檔 [MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md](../../../Managed/MANAGED_RUNTIME_ARCHITECTURE_AND_PROGRESS.md) **§0**（2026 主線、Kernel 化 **P0-1～P0-5**、Legacy Lua）；本模組處「後 ABI」階段，重點為子表與 **layoutVersion** 演進；**Graph.Runtime** 為 **100% Managed**（**§0.3** **P0-5**），**不**由本模組承擔。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGEngineRuntime](../VGEngineRuntime/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
- [VGEngineRuntimeServices](../VGEngineRuntimeServices/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
