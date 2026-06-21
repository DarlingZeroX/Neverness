# Runtime Reflection Debug API — 设计文档

## 1. 概述

### 1.1 目标

实现「结构化 Runtime Reflection Snapshot API」，使 C# Editor 能通过 Native 反射元数据自动构建 Inspector UI，包括：

- Scene Hierarchy
- Component Viewer
- Property Grid
- Remote Runtime Debugger
- Future SerializedProperty system

### 1.2 设计原则

| 原则 | 说明 |
|------|------|
| **POD only** | 所有公开结构体为 plain-old-data，无 C++ 对象 |
| **ABI stable** | layoutVersion 控制，仅尾部追加，破坏性变更递增版本号 |
| **memcpy-safe** | 结构体可直接 memcpy 到 C#（`[StructLayout(LayoutKind.Sequential)]`） |
| **UTF-8 name pool** | 名字不内嵌结构体，通过 nameOffset + nameLen 引用连续 namePool |
| **零硬编码** | 所有数据来自 `NNComponentRegistry`，无 `if (typeid(...))` 链 |
| **Native 只输出** | reflection metadata、component descriptors、raw component data |
| **C# 完成 UI** | 所有 UI 逻辑在 C# 端，Native 不含 printf / ImGui / 字符串拼接 |

---

## 2. 架构总览

```
┌──────────────────────────────────────────────────────────┐
│  C# Editor                                              │
│  ┌────────────────────┐  ┌─────────────────────────────┐ │
│  │ EditorSceneNative  │  │ Inspector / PropertyGrid    │ │
│  │ Bridge (unsafe)    │──│ (auto-generated UI)         │ │
│  └────────┬───────────┘  └─────────────────────────────┘ │
│           │ delegate* unmanaged                          │
├───────────┼──────────────────────────────────────────────┤
│  Native   │                                              │
│  ┌────────▼───────────┐  ┌─────────────────────────────┐ │
│  │ NNEditorSceneAPI   │  │ NNReflectionSnapshotBuilder │ │
│  │ (C function table) │──│ (EstimateSize + Build)      │ │
│  └────────┬───────────┘  └────────────┬────────────────┘ │
│           │                           │                  │
│  ┌────────▼───────────────────────────▼────────────────┐ │
│  │ NNComponentRegistry                                 │ │
│  │   NNComponentTypeDesc → NNComponentFieldDesc[]      │ │
│  │   ForEachDescriptor / FindDescByNameHash            │ │
│  └─────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────┘
```

---

## 3. 数据结构

### 3.1 NNEditorComponentInfo（24 字节）

描述单个组件类型的元数据。

```c
typedef struct NNEditorComponentInfo
{
    uint64_t typeId;        // FNV-1a name hash，与 NNSceneAPI 的 componentTypeId 一致
    uint32_t nameOffset;    // 在 namePool 中的字节偏移
    uint32_t nameLen;       // 名字字节长度（不含 NUL 终止符）
    uint32_t fieldCount;    // 该组件类型的字段数量
    uint32_t flags;         // 保留：[0]=serializable [1]=hasPostDeserialize ...
} NNEditorComponentInfo;
```

**内存布局验证：**
- `offsetof(typeId) == 0`
- `offsetof(nameOffset) == 8`
- `offsetof(nameLen) == 12`
- `offsetof(fieldCount) == 16`
- `offsetof(flags) == 20`
- `sizeof == 24`

### 3.2 NNEditorFieldInfo（24 字节）

描述单个字段的反射元数据。

```c
typedef struct NNEditorFieldInfo
{
    uint32_t nameOffset;    // 在 namePool 中的字节偏移
    uint32_t nameLen;       // 名字字节长度（不含 NUL 终止符）
    uint32_t fieldType;     // NNComponentFieldType 枚举值（cast to uint32_t）
    uint32_t dataOffset;    // 字段在组件原始数据中的字节偏移
    uint32_t dataSize;      // 字段占用字节数
    uint32_t _pad;          // 对齐到 8 字节边界
} NNEditorFieldInfo;
```

**内存布局验证：**
- `offsetof(nameOffset) == 0`
- `offsetof(nameLen) == 4`
- `offsetof(fieldType) == 8`
- `offsetof(dataOffset) == 12`
- `offsetof(dataSize) == 16`
- `sizeof == 24`

### 3.3 NNComponentFieldType 枚举

```cpp
enum class NNComponentFieldType : std::uint8_t
{
    Float = 0,
    Float3,
    Float4,
    Float4x4,
    Quaternion,
    UInt32,
    UInt64,
    Entity,
    CharArray,
    Guid,
};
```

---

## 4. API 函数表

### 4.1 NNEditorSceneAPI（layoutVersion = 3）

在 layoutVersion = 2 基础上追加 7 个函数指针：

```c
typedef struct NNEditorSceneAPI
{
    uint32_t layoutVersion; // = 3

    // Phase 1：Hierarchy / Transform 快照（v1）
    NNEditorGetHierarchyVersionFn  getHierarchyVersion;
    NNEditorGetSnapshotSizeFn      getSnapshotSize;
    NNEditorGetHierarchySnapshotFn getHierarchySnapshot;
    NNEditorGetTransformVersionFn  getTransformVersion;
    NNEditorGetTransformSnapshotFn getTransformSnapshot;

    // Phase 2：增量快照（v2）
    NNEditorGetIncrementalSnapshotFn getIncrementalSnapshot;

    // Phase 3：Runtime Reflection（v3）
    NNEditorGetReflectionVersionFn      getReflectionVersion;
    NNEditorGetTypeInfoSnapshotSizeFn   getTypeInfoSnapshotSize;
    NNEditorGetTypeInfoSnapshotFn       getTypeInfoSnapshot;
    NNEditorGetEntityComponentCountFn   getEntityComponentCount;
    NNEditorGetEntityComponentsFn       getEntityComponents;
    NNEditorGetComponentFieldInfosFn    getComponentFieldInfos;
    NNEditorGetComponentRawDataFn       getComponentRawData;
} NNEditorSceneAPI;
// sizeof == 4 + 4 + 13 * 8 = 112
```

### 4.2 函数签名

#### getReflectionVersion
```c
uint64_t getReflectionVersion(NNSceneHandle scene);
```
- **用途**：C# 每帧调用（~50ns），版本变化才重新拉取 entity 组件列表
- **递增时机**：`Emplace<T>` / `Remove<T>` 时

#### getTypeInfoSnapshotSize
```c
uint32_t getTypeInfoSnapshotSize(NNSceneHandle scene);
```
- **用途**：查询类型信息快照所需缓冲区大小
- **返回值**：`sizeof(Header) + typeCount * 24 + totalFieldCount * 24 + namePoolBytes`

#### getTypeInfoSnapshot
```c
uint32_t getTypeInfoSnapshot(NNSceneHandle scene, void* outBuffer, uint32_t capacity);
```
- **用途**：拷贝类型信息快照到调用方缓冲区
- **布局**：`[Header 32B][ComponentInfo[] 24B × N][FieldInfo[] 24B × M][NamePool]`
- **返回值**：实际写入字节数；capacity 不足时返回所需大小

#### getEntityComponentCount
```c
uint32_t getEntityComponentCount(NNSceneHandle scene, uint64_t entity);
```
- **用途**：查询实体拥有的组件数量
- **返回值**：实际组件数（即使 outInfos 为 nullptr）

#### getEntityComponents
```c
uint32_t getEntityComponents(NNSceneHandle scene, uint64_t entity,
    NNEditorComponentInfo* outInfos, uint32_t capacity);
```
- **用途**：拷贝实体的组件信息数组
- **注意**：只有 `typeId` 和 `flags` 有效，`nameOffset/nameLen/fieldCount` 通过类型快照获取

#### getComponentFieldInfos
```c
uint32_t getFieldInfos(NNSceneHandle scene, uint64_t componentTypeId,
    NNEditorFieldInfo* outFields, uint32_t capacity);
```
- **用途**：拷贝指定组件类型的字段信息
- **C# 侧缓存**：typeId → FieldInfo[] 映射

#### getComponentRawData
```c
uint32_t getComponentRawData(NNSceneHandle scene, uint64_t entity,
    uint64_t componentTypeId, void* outData, uint32_t capacity);
```
- **用途**：拷贝实体的指定组件原始数据
- **数据布局**：与 `NNComponentTypeDesc.SizeBytes` 一致，字段偏移由 `NNEditorFieldInfo.dataOffset` 描述

---

## 5. 类型信息快照格式

### 5.1 内存布局

```
┌──────────────────────────────────────────┐
│ NNSceneSnapshotHeader (32 bytes)         │
│   magic = 0x56475343 ('VGSC')           │
│   layoutVersion = 2                      │
│   hierarchyVersion = reflectionVersion   │
│   nodeCount = typeCount                  │
│   namePoolBytes                          │
│   rootCount = 0                          │
├──────────────────────────────────────────┤
│ NNEditorComponentInfo[0] (24 bytes)      │
│   typeId, nameOffset, nameLen,           │
│   fieldCount, flags                      │
├──────────────────────────────────────────┤
│ NNEditorComponentInfo[1] (24 bytes)      │
│ ...                                      │
├──────────────────────────────────────────┤
│ NNEditorFieldInfo[0] (24 bytes)          │
│   nameOffset, nameLen, fieldType,        │
│   dataOffset, dataSize, _pad             │
├──────────────────────────────────────────┤
│ NNEditorFieldInfo[1] (24 bytes)          │
│ ...                                      │
├──────────────────────────────────────────┤
│ char namePool[namePoolBytes]             │
│   "Transform\0Position\0Rotation\0..."   │
└──────────────────────────────────────────┘
```

### 5.2 示例：Transform 组件

```
ComponentInfo:
  typeId = 0xC1FFF4F356DFB2FB (fnv1a_64("Transform"))
  nameOffset = 0, nameLen = 9
  fieldCount = 4, flags = 0

FieldInfo[0]:
  nameOffset = 10, nameLen = 8 ("Position")
  fieldType = 2 (Float3), dataOffset = 0, dataSize = 12

FieldInfo[1]:
  nameOffset = 19, nameLen = 8 ("Rotation")
  fieldType = 4 (Quaternion), dataOffset = 12, dataSize = 16

FieldInfo[2]:
  nameOffset = 28, nameLen = 5 ("Scale")
  fieldType = 2 (Float3), dataOffset = 28, dataSize = 12

FieldInfo[3]:
  nameOffset = 34, nameLen = 11 ("WorldMatrix")
  fieldType = 3 (Float4x4), dataOffset = 48, dataSize = 64

NamePool: "Transform\0Position\0Rotation\0Scale\0WorldMatrix\0"
```

---

## 6. 版本追踪

### 6.1 ReflectionVersion

- 存储于 `NNRuntimeScene::m_ReflectionVersion`（atomic<uint64_t>）
- `Emplace<T>()` 时递增
- `Remove<T>()` 时递增
- C# 每帧通过 `getReflectionVersion()` poll（~50ns）
- 版本变化 → 重新拉取 entity 组件列表

### 6.2 版本号汇总

| 范围 | 版本 | 用途 |
|------|------|------|
| `NNNativeEngineAPI` | 18 | 根聚合表布局版本 |
| `NNSceneAPI` | 6 | Runtime 场景操作 |
| `NNEditorSceneAPI` | 3 | Editor 快照 + Reflection |
| `NNSceneSnapshotHeader.layoutVersion` | 1 | Hierarchy 快照格式 |
| `NNSceneSnapshotHeader.layoutVersion` | 2 | 类型信息快照格式 |

---

## 7. 实现文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `NNNativeEngineAPI/Include/EditorSceneAPI.h` | 修改 | 2 个 POD 结构体 + 7 个函数签名 + API struct 扩展 |
| `NNRuntimeScene/Include/Snapshot/NNReflectionSnapshotBuilder.h` | 新建 | 快照构建器头文件 |
| `NNRuntimeScene/Source/Snapshot/NNReflectionSnapshotBuilder.cpp` | 新建 | EstimateSize + Build 实现 |
| `NNRuntimeScene/Include/Scene/NNRuntimeScene.h` | 修改 | m_ReflectionVersion + 版本访问器 + Emplace/Remove 钩子 |
| `NNRuntimeEngine/Include/SceneSubsystem.h` | 修改 | 7 个方法声明 |
| `NNRuntimeEngine/Private/SceneSubsystem.cpp` | 修改 | 7 个方法实现 |
| `NNRuntimeEngineServices/.../SceneRuntimeApi.cpp` | 修改 | v2→v3, 7 个函数指针 |
| `NNRuntimeNativeEngineAPIStub/.../SceneApiStubs.cpp` | 修改 | v2→v3, 7 个 stub |
| `NNNativeEngineAPI/Include/EngineAPIRegistry.h` | 修改 | 17→18 |
| `Managed/.../NNNativeEngineApiTypes.cs` | 修改 | C# POD structs + API 扩展 |
| `Managed/.../NNNativeEngineApiConstants.cs` | 修改 | 17→18 |
| `Managed/.../EditorSceneNativeBridge.cs` | 修改 | 7 个 bridge 方法 |
| `NNRuntimeScene/CMakeLists.txt` | 修改 | 新文件注册 |

---

## 8. C# 使用流程

### 8.1 初始化（一次性）

```csharp
// 1. 获取类型信息快照（一次性，或 reflectionVersion 变化时重建）
uint size = EditorSceneNativeBridge.GetTypeInfoSnapshotSize(sceneHandle);
byte[] typeInfoBuffer = new byte[size];
fixed (byte* p = typeInfoBuffer)
{
    EditorSceneNativeBridge.GetTypeInfoSnapshot(sceneHandle, p, size);
}
// 2. 解析 Header → ComponentInfo[] → FieldInfo[] → NamePool
// 3. 构建 Dictionary<ulong, TypeInfo> 缓存
```

### 8.2 每帧 Inspector 刷新

```csharp
// 1. 检查 reflectionVersion
ulong version = EditorSceneNativeBridge.GetReflectionVersion(sceneHandle);
if (version != cachedVersion)
{
    // 2. 重新拉取选中实体的组件列表
    uint count = EditorSceneNativeBridge.GetEntityComponentCount(sceneHandle, selectedEntity);
    Span<NNEditorComponentInfo> components = stackalloc NNEditorComponentInfo[(int)count];
    EditorSceneNativeBridge.GetEntityComponents(sceneHandle, selectedEntity, components);

    // 3. 对每个组件，获取字段信息 + 原始数据
    foreach (var comp in components)
    {
        var typeInfo = typeInfoCache[comp.TypeId];

        // 拉取原始数据
        byte[] rawData = new byte[typeInfo.Size];
        fixed (byte* p = rawData)
        {
            EditorSceneNativeBridge.GetComponentRawData(
                sceneHandle, selectedEntity, comp.TypeId, p, (uint)rawData.Length);
        }

        // 4. 根据 FieldInfo 自动构建 UI
        foreach (var field in typeInfo.Fields)
        {
            // field.FieldType → 选择 PropertyDrawer
            // field.DataOffset → 读写 rawData[field.DataOffset .. +field.DataSize]
            RenderField(field, rawData);
        }
    }
    cachedVersion = version;
}
```

---

## 9. 未来扩展

| 功能 | 预留设计 |
|------|----------|
| **SerializedProperty** | `NNEditorFieldInfo.dataOffset + dataSize` 直接映射到属性路径 |
| **Property Drawer** | `fieldType` 枚举驱动 C# 端选择对应的 Drawer |
| **Prefab Override** | `flags` 字段标记 overridden fields |
| **Remote Runtime Editor** | POD snapshot 可通过网络传输，零 C++ 对象依赖 |
| **Network Inspector** | `getComponentRawData` 返回真实内存，可序列化为网络包 |
| **Live Hot Reload** | `reflectionVersion` 在热重载时递增，触发 C# 重建缓存 |
| **C# Gameplay Component** | `getEntityComponentCount` 自动发现新注册的 C# 组件 |
| **Editable Fields** | `SetComponent` API 已存在；field offset/desc 驱动 C# 写入路径 |

---

## 10. 验证方案

1. **编译验证**：C++ cmake build + dotnet build，0 错误
2. **ABI 验证**：static_assert 通过，sizeof 对齐
3. **Round-trip 测试**：创建场景 → 添加组件 → 验证快照数据正确
4. **版本递增测试**：AddComponent / RemoveComponent 后验证 reflectionVersion 递增
5. **零硬编码验证**：注册新组件类型 → 不修改 API 代码 → 验证自动包含新类型
