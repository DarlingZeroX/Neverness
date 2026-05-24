# Neverness Engine — Editor Scene Snapshot System

> 版本：v2.0　|　日期：2026-05-24
> 目标：消除 Editor SceneBrowser 的逐 Entity P/Invoke 调用，实现 Snapshot-driven 架构

---

## 1. 问题分析

### 1.1 现状瓶颈

当前 SceneBrowser（ImGui 树）绘制一个 Entity 需要：

```
getParent(handle)        → P/Invoke 1
hasComponent(Tag)        → P/Invoke 2
getComponent(Tag)        → P/Invoke 3
hasComponent(Transform)  → P/Invoke 4
```

1000 entities × 4 calls = **4000 次 P/Invoke / 帧**。即使 SceneBrowser 目前是 stub，一旦实现就会暴露：

| 问题 | 原因 |
|------|------|
| P/Invoke 开销 | 每次 call = 用户态 ↔ 内核态切换 + ABI 序列化 |
| ECS cache miss | 逐 entity 查询打散 EnTT sparse-set 的连续访问模式 |
| ImGui Tree 开销 | `TreeNode` 递归 + `SetNextItemOpen` 无法预测展开状态 |
| 无法扩展 | 搜索、过滤、虚拟化都需要全局数据视图 |

### 1.2 解决思路

**Snapshot-driven**：Native 端一次性批量构建整个 Hierarchy 为连续 POD 数组 + 名字池，C# 端通过 version polling 按需 `memcpy` 到缓存，ImGui 全部从缓存绘制。

```
每帧：C# 调用 getHierarchyVersion()     → 1 次轻量 P/Invoke（纯整数返回）
  ├─ version 未变 → 跳过（0 工作量）
  └─ version 变化 → getSnapshotSize() → getHierarchySnapshot() → 2 次 P/Invoke
                                                                    ↓
                                                              ParseSnapshot()
                                                                    ↓
                                                              RebuildVisibleList()
                                                                    ↓
                                                              ImGui 绘制
```

---

## 2. Native 端设计

### 2.1 Editor Scene API（独立子表）

> **关键决策：独立于 NNSceneAPI。** NNSceneAPI 是 Runtime 场景操作（创建实体、组件读写），EditorSceneAPI 是 Editor 专用查询。两者 ABI 版本独立演进，互不干扰。

```
NNSceneAPI (layoutVersion = 6)     ← Runtime 操作（createEntity / getComponent / ...）
    ↓ 不追加 Editor 函数
NNEditorSceneAPI (layoutVersion = 2) ← Editor 查询（getHierarchyVersion / getSnapshot / getIncrementalSnapshot）
```

### 2.2 Memory Layout

```
┌──────────────────────────────────────────────────────────────────────┐
│  NNEditorSnapshotBuffer（Native 输出，C# 一次性拷贝）                │
│                                                                      │
│  Offset 0:    NNSceneSnapshotHeader (32 bytes)                       │
│  Offset 32:   NNSceneNodeSnapshot[nodeCount] (40 bytes each)         │
│  Offset 32+   char namePool[namePoolBytes] (UTF-8, NUL 分隔)        │
│               + N * 40:                                               │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### 2.3 POD 结构体

> 规则：完全 C POD，无 std::string，无 vector，无非 POD 类型。名字池分离（支持任意长度 UTF-8）。

```c
// EditorSceneAPI.h

#include <stddef.h>
#include <stdint.h>

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

// ── Snapshot Header ──────────────────────────────────────────────────

/** @brief 快照头部——位于 buffer 开头，描述后续 Node 数组和名字池的布局。 */
typedef struct NNSceneSnapshotHeader
{
    uint32_t magic;            ///< 0x56475343（'VGSC' 小端），用于校验完整性
    uint32_t layoutVersion;    ///< = 1
    uint64_t hierarchyVersion; ///< 单调递增，C# 比对后决定是否重建缓存
    uint32_t nodeCount;        ///< NNSceneNodeSnapshot 数量
    uint32_t namePoolBytes;    ///< 名字池总字节数（UTF-8，每个名字 NUL 分隔）
    uint32_t rootCount;        ///< 根节点数量（C# 快速定位根列表起点）
    uint32_t _pad;             ///< 对齐保留
} NNSceneSnapshotHeader;

_Static_assert(sizeof(NNSceneSnapshotHeader) == 32,
    "NNSceneSnapshotHeader must be 32 bytes");

_Static_assert(offsetof(NNSceneSnapshotHeader, magic)            ==  0, "");
_Static_assert(offsetof(NNSceneSnapshotHeader, layoutVersion)    ==  4, "");
_Static_assert(offsetof(NNSceneSnapshotHeader, hierarchyVersion) ==  8, "");
_Static_assert(offsetof(NNSceneSnapshotHeader, nodeCount)        == 16, "");
_Static_assert(offsetof(NNSceneSnapshotHeader, namePoolBytes)    == 20, "");
_Static_assert(offsetof(NNSceneSnapshotHeader, rootCount)        == 24, "");

// ── Node Snapshot ────────────────────────────────────────────────────

/**
 * @brief 场景层级节点快照——单个 Entity 的 Hierarchy 信息。
 *
 * 名字不内嵌，而是通过 nameOffset + nameLen 引用 Header 之后的 namePool。
 * 这样名字长度不受限制，支持本地化名、自动生成长名、Prefab 路径等。
 */
typedef struct NNSceneNodeSnapshot
{
    uint64_t entity;       ///< NNEntityHandle
    uint64_t parent;       ///< 父实体句柄（根节点 = 0）
    uint32_t depth;        ///< DFS 深度（根 = 0）
    uint32_t childCount;   ///< 直接子节点数
    uint32_t nameOffset;   ///< 在 namePool 中的字节偏移（相对于 namePool 起始）
    uint32_t nameLen;      ///< 字节长度（不含 NUL 终止符）
    uint32_t flags;        ///< bit0=active bit1=prefab_instance bit2=dirty bit3=selected
    uint32_t _pad;         ///< 对齐到 8 字节边界
} NNSceneNodeSnapshot;

_Static_assert(sizeof(NNSceneNodeSnapshot) == 40,
    "NNSceneNodeSnapshot must be 40 bytes");

_Static_assert(offsetof(NNSceneNodeSnapshot, entity)     ==  0, "");
_Static_assert(offsetof(NNSceneNodeSnapshot, parent)     ==  8, "");
_Static_assert(offsetof(NNSceneNodeSnapshot, depth)      == 16, "");
_Static_assert(offsetof(NNSceneNodeSnapshot, childCount) == 20, "");
_Static_assert(offsetof(NNSceneNodeSnapshot, nameOffset) == 24, "");
_Static_assert(offsetof(NNSceneNodeSnapshot, nameLen)    == 28, "");
_Static_assert(offsetof(NNSceneNodeSnapshot, flags)      == 32, "");

// ── Transform Snapshot ───────────────────────────────────────────────

/** @brief Transform 快照节点——批量读取 Transform 组件。 */
typedef struct NNEditorTransformData
{
    uint64_t entity;       ///< NNEntityHandle
    float    posX, posY, posZ;
    float    rotX, rotY, rotZ, rotW;
    float    sclX, sclY, sclZ;
} NNEditorTransformData;

_Static_assert(sizeof(NNEditorTransformData) == 48, "");

// ── Incremental Dirty Entry（Phase 2 预留）───────────────────────────

/**
 * @brief 脏节点条目——增量快照使用。
 * changeFlags 标记哪些属性发生了变化，C# 端按 flag 局部更新缓存。
 */
typedef struct NNDirtyNodeEntry
{
    uint64_t entity;       ///< 脏实体句柄
    uint32_t changeFlags;  ///< bit0=name bit1=parent bit2=childCount bit3=active bit4=flags
    uint32_t _pad;
} NNDirtyNodeEntry;

_Static_assert(sizeof(NNDirtyNodeEntry) == 16, "");

// ── ChangeFlags 常量 ──
#define NN_DIRTY_NAME_CHANGED      (1u << 0)
#define NN_DIRTY_PARENT_CHANGED    (1u << 1)
#define NN_DIRTY_CHILDREN_CHANGED  (1u << 2)
#define NN_DIRTY_ACTIVE_CHANGED    (1u << 3)
#define NN_DIRTY_FLAGS_CHANGED     (1u << 4)

#pragma pack(pop)

// ── 函数签名 ─────────────────────────────────────────────────────────

/**
 * @brief 查询当前 hierarchyVersion（不拷贝数据，C# 每帧调用）。
 *
 * 这是最热路径的函数——每帧调用 1 次，纯整数返回。
 * Native 实现：返回 NNRuntimeScene::m_HierarchyVersion（atomic load）。
 */
typedef uint64_t (NN_ENGINE_ABI_STDCALL* NNEditorGetHierarchyVersionFn)(
    NNSceneHandle scene);

/**
 * @brief 查询 snapshot 所需缓冲区大小（字节）。
 *
 * 返回 Header + Nodes + NamePool 的总字节数。
 * C# 端用此值确保 _snapshotBuffer 容量充足。
 */
typedef uint32_t (NN_ENGINE_ABI_STDCALL* NNEditorGetSnapshotSizeFn)(
    NNSceneHandle scene);

/**
 * @brief 拷贝完整 snapshot 到调用方缓冲区。
 *
 * 布局：[Header 32B][Nodes * nodeCount * 40B][NamePool * namePoolBytes]
 * 返回实际写入字节数；0 = 失败。
 *
 * @param scene   场景句柄
 * @param outBuffer 调用方预分配的缓冲区
 * @param capacity  缓冲区容量（字节），应 >= getSnapshotSize 返回值
 */
typedef uint32_t (NN_ENGINE_ABI_STDCALL* NNEditorGetHierarchySnapshotFn)(
    NNSceneHandle scene,
    void*         outBuffer,
    uint32_t      capacity);

/**
 * @brief 查询 Transform 版本号（仅 Transform 变化时递增）。
 *
 * 独立于 hierarchyVersion——名称/层级变化不触发 Transform 版本递增。
 */
typedef uint64_t (NN_ENGINE_ABI_STDCALL* NNEditorGetTransformVersionFn)(
    NNSceneHandle scene);

/**
 * @brief 按 Entity 列表批量获取 Transform 数据。
 *
 * @param entities     请求的 entity 句柄数组
 * @param entityCount  实体数量
 * @param outArray     调用方分配的输出数组（entityCount 个 NNEditorTransformData）
 */
typedef uint32_t (NN_ENGINE_ABI_STDCALL* NNEditorGetTransformSnapshotFn)(
    NNSceneHandle             scene,
    const uint64_t*           entities,
    uint32_t                  entityCount,
    NNEditorTransformData*    outArray);

// ── 函数表 ───────────────────────────────────────────────────────────

/**
 * @brief Editor 专用场景查询函数表（独立于 NNSceneAPI）。
 *
 * layoutVersion = 2；破坏性变更时递增，仅允许尾部追加。
 * 与 NNSceneAPI 独立演进——Runtime API 变更不影响 Editor，反之亦然。
 */
typedef struct NNEditorSceneAPI
{
    uint32_t layoutVersion; ///< = 2

    NNEditorGetHierarchyVersionFn  getHierarchyVersion;
    NNEditorGetSnapshotSizeFn      getSnapshotSize;
    NNEditorGetHierarchySnapshotFn getHierarchySnapshot;

    NNEditorGetTransformVersionFn  getTransformVersion;
    NNEditorGetTransformSnapshotFn getTransformSnapshot;

    // ── Phase 2：增量快照 ──
    NNEditorGetIncrementalSnapshotFn getIncrementalSnapshot;

} NNEditorSceneAPI;

#ifdef __cplusplus
}
#endif
```

### 2.4 NNRuntimeScene 版本号字段

```cpp
// NNRuntimeScene.h — 新增字段
class NNRuntimeScene {
    // ... existing fields ...
    std::atomic<uint64_t> m_HierarchyVersion{0};   ///< EntityCreated/Destroyed/ParentChanged 时递增
    std::atomic<uint64_t> m_TransformVersion{0};    ///< NNTransformSystem 计算 WorldMatrix 时递增

public:
    uint64_t HierarchyVersion() const { return m_HierarchyVersion.load(std::memory_order_relaxed); }
    void IncrementHierarchyVersion() { m_HierarchyVersion.fetch_add(1, std::memory_order_relaxed); }

    uint64_t TransformVersion() const { return m_TransformVersion.load(std::memory_order_relaxed); }
    void IncrementTransformVersion() { m_TransformVersion.fetch_add(1, std::memory_order_relaxed); }
};
```

### 2.5 HierarchySnapshotBuilder

```
Runtime/NNRuntimeScene/
├── Include/
│   └── Snapshot/
│       └── NNHierarchySnapshotBuilder.h
└── Source/
    └── Snapshot/
        └── NNHierarchySnapshotBuilder.cpp
```

```cpp
// NNHierarchySnapshotBuilder.h
#pragma once
#include "Scene/NNRuntimeScene.h"
#include "Components/NNRelationshipComponent.h"
#include "Components/NNTagComponent.h"

namespace NN {

/// 层级快照构建器——从 ECS Registry 批量构建 DFS 有序的连续 buffer。
/// 输出格式：[Header][Nodes[]][NamePool]，完全 POD，可直接 memcpy 到 C#。
class NNHierarchySnapshotBuilder
{
public:
    /// 构建完整快照到 outBuffer。
    /// @return 实际写入字节数；0 = capacity 不足（此时内部仍计算所需大小供 getSnapshotSize 使用）。
    static uint32_t Build(
        const NNRuntimeScene& scene,
        void*                 outBuffer,
        uint32_t              capacity);

    /// 获取构建当前快照所需的总字节数（Header + Nodes + NamePool）。
    static uint32_t EstimateSize(const NNRuntimeScene& scene);
};

} // namespace NN
```

**构建算法（两遍扫描 + DFS 输出）：**

```cpp
// NNHierarchySnapshotBuilder.cpp
#include "Snapshot/NNHierarchySnapshotBuilder.h"
#include "Systems/NNHierarchySystem.h"
#include <vector>
#include <cstring>
#include <algorithm>
#include <functional>

namespace NN {

uint32_t NNHierarchySnapshotBuilder::EstimateSize(const NNRuntimeScene& scene)
{
    auto& registry = scene.Registry();
    uint32_t nodeCount = 0;
    uint32_t namePoolBytes = 0;

    registry.view<NNRelationshipComponent>().each(
        [&](const NNTagComponent* tag) {
            nodeCount++;
            if (tag) {
                namePoolBytes += (uint32_t)strnlen(tag->Name, 63) + 1; // +1 for NUL
            } else {
                namePoolBytes += 1; // empty string = single NUL
            }
        });

    return sizeof(NNSceneSnapshotHeader)
         + nodeCount * sizeof(NNSceneNodeSnapshot)
         + namePoolBytes;
}

uint32_t NNHierarchySnapshotBuilder::Build(
    const NNRuntimeScene& scene,
    void*                 outBuffer,
    uint32_t              capacity)
{
    auto& registry     = scene.Registry();
    auto& handleTable  = scene.EntityHandleTable();
    auto* hierarchySys = scene.GetSystem<NNHierarchySystem>();

    // ── 第一遍：统计节点数 + 名字池大小 ──
    uint32_t nodeCount = 0;
    uint32_t namePoolBytes = 0;

    // 收集根节点 + 统计
    std::vector<NNEntity> roots;
    registry.view<NNRelationshipComponent>().each(
        [&](entt::entity enttE, const NNRelationshipComponent& rel) {
            nodeCount++;
            if (rel.Parent == NNEntityInvalid) {
                roots.push_back(handleTable.HandleFromEntt(enttE));
            }
            // 统计名字长度
            auto* tag = registry.try_get<NNTagComponent>(enttE);
            namePoolBytes += tag ? (uint32_t)strnlen(tag->Name, 63) + 1 : 1;
        });

    // 根节点排序（稳定 DFS 序）
    std::sort(roots.begin(), roots.end());

    // ── 验证容量 ──
    uint32_t needed = sizeof(NNSceneSnapshotHeader)
                    + nodeCount * sizeof(NNSceneNodeSnapshot)
                    + namePoolBytes;

    if (capacity < needed || outBuffer == nullptr) {
        return needed; // 返回所需大小，调用方可用于 getSnapshotSize
    }

    // ── 写 Header ──
    auto* header = reinterpret_cast<NNSceneSnapshotHeader*>(outBuffer);
    header->magic            = 0x56475343; // 'VGSC'
    header->layoutVersion    = 1;
    header->hierarchyVersion = scene.HierarchyVersion();
    header->nodeCount        = nodeCount;
    header->namePoolBytes    = namePoolBytes;
    header->rootCount        = (uint32_t)roots.size();
    header->_pad             = 0;

    // ── DFS 遍历：写 Nodes[] + NamePool ──
    auto* nodes    = reinterpret_cast<NNSceneNodeSnapshot*>(header + 1);
    char* namePool = reinterpret_cast<char*>(nodes + nodeCount);

    // 构建 Parent → Children 映射
    auto childrenMap = hierarchySys->GetAllChildrenMap();

    uint32_t nodeIdx    = 0;
    uint32_t nameOffset = 0;

    std::function<void(NNEntity, uint32_t)> dfs = [&](NNEntity entity, uint32_t depth)
    {
        auto& n = nodes[nodeIdx++];

        n.entity     = entity;
        n.parent     = 0;
        n.depth      = depth;
        n.childCount = 0;
        n.nameOffset = nameOffset;
        n.flags      = 0;

        // 读 Relationship
        entt::entity enttE = handleTable.Resolve(entity);
        if (auto* rel = registry.try_get<NNRelationshipComponent>(enttE)) {
            n.parent     = rel->Parent;
            n.childCount = rel->ChildCount;
        }

        // 读 Tag → 写入 namePool
        if (auto* tag = registry.try_get<NNTagComponent>(enttE)) {
            n.flags = tag->Flags;
            uint32_t len = (uint32_t)strnlen(tag->Name, 63);
            memcpy(namePool + nameOffset, tag->Name, len);
            namePool[nameOffset + len] = '\0';
            n.nameLen = len;
            nameOffset += len + 1;
        } else {
            namePool[nameOffset] = '\0';
            n.nameLen = 0;
            nameOffset += 1;
        }

        // 子节点逆序入栈（保持 DFS 有序）
        auto it = childrenMap.find(entity);
        if (it != childrenMap.end()) {
            const auto& children = it->second;
            for (int i = (int)children.size() - 1; i >= 0; --i) {
                dfs(children[i], depth + 1);
            }
        }
    };

    for (int i = (int)roots.size() - 1; i >= 0; --i) {
        dfs(roots[i], 0);
    }

    return needed;
}

} // namespace NN
```

### 2.6 SceneSubsystem 新增方法 + 函数表填充

```cpp
// SceneSubsystem.h — 新增
uint64_t GetHierarchyVersion(NNSceneHandle scene);
uint32_t GetSnapshotSize(NNSceneHandle scene);
uint32_t GetHierarchySnapshot(NNSceneHandle scene, void* outBuffer, uint32_t capacity);
uint64_t GetTransformVersion(NNSceneHandle scene);
uint32_t GetTransformSnapshot(NNSceneHandle scene, const uint64_t* entities,
                               uint32_t count, NNEditorTransformData* out);

// SceneSubsystem.cpp — 实现
uint64_t SceneSubsystem::GetHierarchyVersion(NNSceneHandle scene) {
    std::lock_guard lock(m_Mutex);
    auto* s = ResolveScene(scene);
    return s ? s->HierarchyVersion() : 0;
}

uint32_t SceneSubsystem::GetSnapshotSize(NNSceneHandle scene) {
    std::lock_guard lock(m_Mutex);
    auto* s = ResolveScene(scene);
    return s ? NNHierarchySnapshotBuilder::EstimateSize(*s) : 0;
}

uint32_t SceneSubsystem::GetHierarchySnapshot(NNSceneHandle scene, void* buf, uint32_t cap) {
    std::lock_guard lock(m_Mutex);
    auto* s = ResolveScene(scene);
    return s ? NNHierarchySnapshotBuilder::Build(*s, buf, cap) : 0;
}
```

```cpp
// SceneRuntimeApi.cpp — 函数表填充
// 注意：这是独立的 NNEditorSceneAPI，不是追加到 NNSceneAPI
api->layoutVersion         = 1;
api->getHierarchyVersion   = &rt_scene_getHierarchyVersion;
api->getSnapshotSize       = &rt_scene_getSnapshotSize;
api->getHierarchySnapshot  = &rt_scene_getHierarchySnapshot;
api->getTransformVersion   = &rt_scene_getTransformVersion;
api->getTransformSnapshot  = &rt_scene_getTransformSnapshot;
```

### 2.7 NNNativeEngineApi 顶层结构体追加

```c
// EngineAPIRegistry.h — NNNativeEngineApi 聚合体
typedef struct NNNativeEngineApi {
    uint32_t layoutVersion;      // = 14（递增）
    // ... existing sub-tables ...
    NNSceneAPI        scene;
    NNEditorSceneAPI  editorScene;  // ← 新增：独立的 Editor 查询子表
    // ...
} NNNativeEngineApi;
```

---

## 3. C# 端设计

### 3.1 POD 结构体映射

```csharp
// Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs

/// <summary>快照头部——与 Native NNSceneSnapshotHeader 逐字段对齐。32 字节。</summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNSceneSnapshotHeader
{
    public uint  Magic;
    public uint  LayoutVersion;
    public ulong HierarchyVersion;
    public uint  NodeCount;
    public uint  NamePoolBytes;
    public uint  RootCount;
    public uint  Pad;
}

/// <summary>场景层级节点快照——与 Native NNSceneNodeSnapshot 逐字段对齐。40 字节。</summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNSceneNodeSnapshot
{
    public ulong Entity;
    public ulong Parent;
    public uint  Depth;
    public uint  ChildCount;
    public uint  NameOffset;
    public uint  NameLen;
    public uint  Flags;
    public uint  Pad;
}

/// <summary>脏节点条目——增量快照使用。16 字节。</summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNDirtyNodeEntry
{
    public ulong Entity;
    public uint  ChangeFlags;
    public uint  Pad;
}

/// <summary>Transform 快照数据——与 Native NNEditorTransformData 对齐。48 字节。</summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNEditorTransformData
{
    public ulong Entity;
    public float PosX, PosY, PosZ;
    public float RotX, RotY, RotZ, RotW;
    public float SclX, SclY, SclZ;
}

/// <summary>ChangeFlags 常量。</summary>
public static class SnapshotChangeFlags
{
    public const uint NameChanged     = 1u << 0;
    public const uint ParentChanged   = 1u << 1;
    public const uint ChildrenChanged = 1u << 2;
    public const uint ActiveChanged   = 1u << 3;
    public const uint FlagsChanged    = 1u << 4;
}
```

### 3.2 NNEditorSceneApi 函数表（独立子表）

```csharp
/// <summary>
/// Editor 专用场景查询函数表——独立于 NNSceneApi，layoutVersion = 2。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNEditorSceneApi
{
    public uint LayoutVersion;  // = 1

    public delegate* unmanaged<ulong, ulong> GetHierarchyVersion;
    public delegate* unmanaged<ulong, uint> GetSnapshotSize;
    public delegate* unmanaged<ulong, void*, uint, uint> GetHierarchySnapshot;

    public delegate* unmanaged<ulong, ulong> GetTransformVersion;
    public delegate* unmanaged<ulong, ulong*, uint, NNEditorTransformData*, uint> GetTransformSnapshot;
}
```

### 3.3 NNNativeEngineApi 聚合体追加

```csharp
// NNNativeEngineApi struct — LayoutVersion 13→14，新增 EditorScene 子表

[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNNativeEngineApi
{
    public uint LayoutVersion;  // = 14
    public uint Reserved0;
    public NNRenderApi Render;
    public NNUIApi UI;
    public NNAudioApi Audio;
    public NNAssetApi Asset;
    public NNInputApi Input;
    public NNSceneApi Scene;
    public NNEditorSceneApi EditorScene;  // ← 新增
    public NNTimingApi Timing;
    // ... rest
}
```

### 3.4 EditorSceneNativeBridge

```csharp
// Managed/Editor/Neverness.Editor.Scene/Private/EditorSceneNativeBridge.cs

using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Editor.Scene.Private;

/// <summary>
/// Editor 专用场景查询桥接——通过 NNEditorSceneApi 函数表调用。
/// 独立于 SceneNativeBridge（Runtime 用），仅 Editor 模块消费。
/// </summary>
internal static unsafe class EditorSceneNativeBridge
{
    public static bool IsAvailable =>
        EngineNativeApiBootstrap.IsInstalled &&
        EngineNativeApiBootstrap.EngineApi.EditorScene.GetHierarchyVersion != null;

    /// <summary>查询 hierarchyVersion（每帧调用，纯整数返回，最轻量）。</summary>
    public static ulong GetHierarchyVersion(ulong sceneHandle)
    {
        if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetHierarchyVersion == null)
            return 0;
        return api.GetHierarchyVersion(sceneHandle);
    }

    /// <summary>查询 snapshot 所需缓冲区大小（字节）。</summary>
    public static uint GetSnapshotSize(ulong sceneHandle)
    {
        if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetSnapshotSize == null)
            return 0;
        return api.GetSnapshotSize(sceneHandle);
    }

    /// <summary>拷贝完整 snapshot 到调用方缓冲区。返回实际写入字节数。</summary>
    public static uint GetHierarchySnapshot(ulong sceneHandle, void* buffer, uint capacity)
    {
        if (sceneHandle == 0 || buffer == null || !TryGetApi(out var api) || api.GetHierarchySnapshot == null)
            return 0;
        return api.GetHierarchySnapshot(sceneHandle, buffer, capacity);
    }

    /// <summary>查询 transformVersion。</summary>
    public static ulong GetTransformVersion(ulong sceneHandle)
    {
        if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetTransformVersion == null)
            return 0;
        return api.GetTransformVersion(sceneHandle);
    }

    /// <summary>按 Entity 列表批量获取 Transform 数据。</summary>
    public static uint GetTransformSnapshot(
        ulong sceneHandle,
        ReadOnlySpan<ulong> entities,
        Span<NNEditorTransformData> outData)
    {
        if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetTransformSnapshot == null)
            return 0;

        fixed (ulong* pEntities = entities)
        fixed (NNEditorTransformData* pOut = outData)
        {
            return api.GetTransformSnapshot(sceneHandle, pEntities, (uint)entities.Length, pOut);
        }
    }

    private static bool TryGetApi(out NNEditorSceneApi api)
    {
        if (!EngineNativeApiBootstrap.IsInstalled) { api = default; return false; }
        api = EngineNativeApiBootstrap.EngineApi.EditorScene;
        return true;
    }
}
```

---

## 4. C# Editor — SceneHierarchyCache

```
Managed/Editor/Neverness.Editor.Scene/
├── Private/
│   ├── Panel/
│   │   ├── SceneBrowser.cs           ← 重写：基于 Snapshot 绘制
│   │   └── EditorViewport.cs
│   ├── SceneModuleImp.cs
│   ├── EditorSceneNativeBridge.cs    ← Editor 专用 ABI 桥接
│   └── Cache/
│       ├── SceneHierarchyCache.cs    ← 核心缓存
│       └── HierarchyNode.cs          ← 缓存节点
```

### 4.1 HierarchyNode

```csharp
namespace Neverness.Editor.Scene.Private.Cache;

/// <summary>
/// 层级缓存节点——从 NNSceneNodeSnapshot 解析而来。
/// </summary>
public sealed class HierarchyNode
{
    public ulong  Entity;
    public ulong  Parent;
    public uint   Depth;
    public uint   ChildCount;
    public string Name  = "";
    public uint   Flags;
    public int    FlatIndex;  ///< 在 _allNodes 中的索引
}
```

### 4.2 SceneHierarchyCache（完整实现）

```csharp
using System.Runtime.InteropServices;

namespace Neverness.Editor.Scene.Private.Cache;

/// <summary>
/// 场景层级缓存——Native Snapshot 的 Managed 端缓存层。
///
/// 触发机制：
///   每帧调用 TryRefresh() → getHierarchyVersion() 比较版本 → 变化才拉取全量 Snapshot。
///   无变化帧：1 次轻量 P/Invoke（纯整数返回），0 数据拷贝。
///
/// 数据流：
///   _snapshotBuffer (byte[])  ← Native 一次性写入 Header + Nodes + NamePool
///       ↓ ParseSnapshot
///   _allNodes (HierarchyNode[]) ← 解析后，名字已转为 string
///   _entityToIndex (Dictionary)  ← O(1) Entity → Node 查找
///       ↓ RebuildVisibleList
///   _visibleList (List<int>)     ← ImGui 直接遍历（DFS depth 跳跃跳过折叠子树）
/// </summary>
public sealed class SceneHierarchyCache
{
    // ── Snapshot 缓冲区（复用，避免每帧 GC）──

    /// <summary>原始字节缓冲区，Native 直接写入。容量不足时翻倍重新分配。</summary>
    private byte[] _snapshotBuffer = Array.Empty<byte>();

    /// <summary>当前缓存的 hierarchyVersion（与 Native 同步）。</summary>
    private ulong  _cachedVersion = ulong.MaxValue;

    // ── 解析后的数据 ──

    /// <summary>所有节点（DFS 顺序，与 Native 1:1 对应）。</summary>
    private HierarchyNode[] _allNodes = Array.Empty<HierarchyNode>();

    /// <summary>Entity → _allNodes 索引，O(1) 查找。</summary>
    private readonly Dictionary<ulong, int> _entityToIndex = new(4096);

    /// <summary>根节点在 _allNodes 中的索引列表。</summary>
    private readonly List<int> _rootIndices = new(256);

    // ── UI 状态（独立于快照，快照刷新不清空）──

    /// <summary>展开的 Entity 集合。</summary>
    private readonly HashSet<ulong> _expandedSet = new(256);

    /// <summary>选中的 Entity 集合。</summary>
    private readonly HashSet<ulong> _selectedSet = new(64);

    /// <summary>搜索过滤字符串（小写）。</summary>
    private string _searchText = "";

    /// <summary>是否需要重建可见列表。</summary>
    private bool _visibleDirty = true;

    /// <summary>可见节点索引列表（ImGui 直接遍历，DFS depth 跳跃跳过折叠子树）。</summary>
    private readonly List<int> _visibleList = new(4096);

    // ── 公共 API ──

    public int NodeCount => _allNodes.Length;
    public ulong HierarchyVersion => _cachedVersion;
    public string SearchText => _searchText;
    public HierarchyNode[] AllNodes => _allNodes;
    public IReadOnlyList<int> VisibleList
    {
        get { if (_visibleDirty) RebuildVisibleList(); return _visibleList; }
    }
    public IReadOnlyCollection<ulong> SelectedEntities => _selectedSet;

    // ── 版本轮询 + 按需拉取 ──

    /// <summary>
    /// 每帧调用。比较 hierarchyVersion，变化时拉取全量 Snapshot 并重建缓存。
    /// 无变化帧：仅 1 次 getHierarchyVersion()（纯整数 P/Invoke），无数据拷贝。
    /// </summary>
    /// <returns>true = 缓存已更新；false = 版本未变，跳过</returns>
    public unsafe bool TryRefresh(ulong sceneHandle)
    {
        // 1. 版本轮询（最热路径，纯整数返回）
        ulong nativeVersion = EditorSceneNativeBridge.GetHierarchyVersion(sceneHandle);
        if (nativeVersion == _cachedVersion)
            return false; // 无变化，0 数据工作量

        // 2. 查询所需大小
        uint needed = EditorSceneNativeBridge.GetSnapshotSize(sceneHandle);
        if (needed == 0)
            return false;

        // 3. 确保缓冲区容量（翻倍预留，减少下次重分配概率）
        if (_snapshotBuffer.Length < needed)
            _snapshotBuffer = new byte[needed * 2];

        // 4. 一次 P/Invoke 拷贝全部数据
        uint written;
        fixed (byte* buf = _snapshotBuffer)
        {
            written = EditorSceneNativeBridge.GetHierarchySnapshot(sceneHandle, buf, needed * 2);
        }
        if (written == 0)
            return false;

        // 5. 解析 Snapshot → _allNodes
        fixed (byte* buf = _snapshotBuffer)
        {
            ParseSnapshot(buf, written);
        }

        _cachedVersion = nativeVersion;
        _visibleDirty  = true;
        return true;
    }

    // ── Snapshot 解析 ──

    private unsafe void ParseSnapshot(byte* buf, uint size)
    {
        // 校验 Header
        if (size < sizeof(NNSceneSnapshotHeader)) return;
        var header = *(NNSceneSnapshotHeader*)buf;
        if (header.Magic != 0x56475343) return; // 'VGSC'

        // 指针定位
        var nodePtr  = (NNSceneNodeSnapshot*)(buf + sizeof(NNSceneSnapshotHeader));
        var namePool = (byte*)(nodePtr + header.NodeCount);

        // 复用或重建节点数组（避免旧节点对象逃逸）
        if (_allNodes.Length != header.NodeCount)
            _allNodes = new HierarchyNode[header.NodeCount];

        _entityToIndex.Clear();
        _rootIndices.Clear();

        for (uint i = 0; i < header.NodeCount; i++)
        {
            ref readonly var raw = ref nodePtr[i];
            var node = _allNodes[i] ??= new HierarchyNode();

            node.Entity     = raw.Entity;
            node.Parent     = raw.Parent;
            node.Depth      = raw.Depth;
            node.ChildCount = raw.ChildCount;
            node.Flags      = raw.Flags;
            node.FlatIndex  = (int)i;

            // 从 namePool 读取 UTF-8 字符串（nameLen 可为 0）
            node.Name = raw.NameLen > 0
                ? System.Text.Encoding.UTF8.GetString(namePool + raw.NameOffset, (int)raw.NameLen)
                : "";

            _entityToIndex[raw.Entity] = (int)i;

            if (raw.Parent == 0)
                _rootIndices.Add((int)i);
        }
    }

    // ── 可见列表重建（DFS depth 跳跃）──

    /// <summary>
    /// 重建可见节点列表。
    ///
    /// 核心优化：利用 Snapshot 的 DFS 顺序 + depth 字段，折叠时直接跳过整个子树。
    /// 不需要递归——DFS 连续输出保证：所有后代在当前节点之后，且 depth 更大。
    /// 遇到 depth <= 当前 depth 的节点时，子树结束。
    ///
    /// 搜索模式：强制展开所有命中节点的祖先路径，确保命中节点可见。
    /// </summary>
    public void RebuildVisibleList()
    {
        if (!_visibleDirty) return;
        _visibleList.Clear();

        bool hasSearch = !string.IsNullOrEmpty(_searchText);

        // 搜索模式：先展开所有命中节点的祖先路径
        if (hasSearch)
            ExpandSearchPaths();

        int i = 0;
        while (i < _allNodes.Length)
        {
            var node = _allNodes[i];

            // 搜索模式下：过滤不匹配节点
            if (hasSearch && !node.Name.Contains(_searchText, StringComparison.OrdinalIgnoreCase))
            {
                i++;
                continue;
            }

            // 可见 → 加入列表
            _visibleList.Add(i);

            // 未展开 + 有子节点 + 非搜索模式 → 跳过整个子树
            bool collapsed = !_expandedSet.Contains(node.Entity);
            if (collapsed && !hasSearch && node.ChildCount > 0)
            {
                uint skipDepth = node.Depth;
                i++;
                // DFS 顺序保证：depth > skipDepth 的连续节点都是后代
                while (i < _allNodes.Length && _allNodes[i].Depth > skipDepth)
                    i++;
                continue;
            }

            i++;
        }

        _visibleDirty = false;
    }

    /// <summary>展开搜索命中节点的所有祖先（沿 Parent 链向上）。</summary>
    private void ExpandSearchPaths()
    {
        foreach (var node in _allNodes)
        {
            if (!node.Name.Contains(_searchText, StringComparison.OrdinalIgnoreCase))
                continue;

            // 向上展开祖先链
            ulong cur = node.Parent;
            while (cur != 0 && _entityToIndex.TryGetValue(cur, out int idx))
            {
                _expandedSet.Add(cur);
                cur = _allNodes[idx].Parent;
            }
        }
    }

    // ── 展开/折叠 ──

    public bool IsExpanded(ulong entity) => _expandedSet.Contains(entity);

    public void SetExpanded(ulong entity, bool expanded)
    {
        if (expanded) _expandedSet.Add(entity);
        else _expandedSet.Remove(entity);
        _visibleDirty = true;
    }

    public void ToggleExpanded(ulong entity)
    {
        if (!_expandedSet.Remove(entity))
            _expandedSet.Add(entity);
        _visibleDirty = true;
    }

    public void ExpandAll()
    {
        for (int i = 0; i < _allNodes.Length; i++)
            if (_allNodes[i].ChildCount > 0)
                _expandedSet.Add(_allNodes[i].Entity);
        _visibleDirty = true;
    }

    public void CollapseAll()
    {
        _expandedSet.Clear();
        _visibleDirty = true;
    }

    // ── 选中 ──

    public bool IsSelected(ulong entity) => _selectedSet.Contains(entity);

    public void Select(ulong entity, bool additive = false)
    {
        if (!additive) _selectedSet.Clear();
        _selectedSet.Add(entity);
    }

    public void Deselect(ulong entity) => _selectedSet.Remove(entity);

    // ── 搜索 ──

    public void SetSearch(string text)
    {
        if (_searchText == text) return;
        _searchText   = text ?? "";
        _visibleDirty = true;
        // ExpandSearchPaths 在 RebuildVisibleList 中调用
    }

    // ── 查找 ──

    public HierarchyNode? GetNode(ulong entity)
        => _entityToIndex.TryGetValue(entity, out int i) ? _allNodes[i] : null;
}
```

---

## 5. ImGui SceneBrowser（虚拟化渲染）

```csharp
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Scene.Private.Cache;
using System.Numerics;

namespace Neverness.Editor.Scene.Private.Panel;

public class SceneBrowser : IEditorPanel
{
    private bool _isOpen = true;
    private readonly SceneHierarchyCache _cache = new();
    private ulong _sceneHandle;
    private float _itemHeight = 22f;

    public SceneBrowser(ulong sceneHandle)
    {
        _sceneHandle = sceneHandle;
    }

    public void OnGUI()
    {
        if (!_isOpen) return;
        if (!ImGui.Begin(GetWindowFullName(), ImGuiWindowFlags.None))
        { ImGui.End(); return; }

        DrawToolbar();
        DrawHierarchyTree();

        ImGui.End();
    }

    private void DrawToolbar()
    {
        string search = _cache.SearchText;
        ImGui.SetNextItemWidth(ImGui.GetContentRegionAvail().X - 180);
        if (ImGui.InputTextWithHint("##search", "Search entities...", ref search, 256))
            _cache.SetSearch(search);

        ImGui.SameLine();
        if (ImGui.Button("+")) _cache.ExpandAll();
        ImGui.SameLine();
        if (ImGui.Button("-")) _cache.CollapseAll();
        ImGui.SameLine();
        ImGui.TextDisabled($"{_cache.NodeCount} entities");
    }

    /// <summary>手动虚拟化——基于 DFS depth 跳跃的可见列表 + 滚动裁剪。</summary>
    private void DrawHierarchyTree()
    {
        _cache.RebuildVisibleList();
        var visible = _cache.VisibleList;
        if (visible.Count == 0)
        {
            ImGui.TextDisabled("No entities.");
            return;
        }

        float totalHeight = visible.Count * _itemHeight;

        ImGui.BeginChild("##hierarchy", Vector2.Zero, ImGuiChildFlags.None,
            ImGuiWindowFlags.HorizontalScrollbar);

        // 占位 Dummy 控制滚动条范围
        ImGui.Dummy(new Vector2(1, totalHeight));

        // 计算可见区域的 first/last 行
        float scrollY    = ImGui.GetScrollY();
        float viewHeight = ImGui.GetWindowHeight();
        int firstVisible = Math.Max(0, (int)(scrollY / _itemHeight) - 1);
        int lastVisible  = Math.Min(visible.Count - 1,
            firstVisible + (int)(viewHeight / _itemHeight) + 2);

        // 移动游标到第一个可见行
        ImGui.SetCursorPosY(firstVisible * _itemHeight);

        for (int vi = firstVisible; vi <= lastVisible; vi++)
        {
            int nodeIdx = visible[vi];
            if (nodeIdx < 0 || nodeIdx >= _cache.AllNodes.Length) continue;
            DrawNode(_cache.AllNodes[nodeIdx]);
        }

        ImGui.EndChild();
    }

    private void DrawNode(HierarchyNode node)
    {
        bool hasChildren = node.ChildCount > 0;
        bool expanded    = _cache.IsExpanded(node.Entity);
        bool selected    = _cache.IsSelected(node.Entity);

        // 缩进（按 depth）
        float indent = node.Depth * 16f;
        ImGui.SetCursorPosX(ImGui.GetCursorPosX() + indent);

        var flags = ImGuiTreeNodeFlags.SpanAvailWidth
                  | ImGuiTreeNodeFlags.FramePadding
                  | ImGuiTreeNodeFlags.OpenOnArrow
                  | ImGuiTreeNodeFlags.OpenOnDoubleClick;

        if (!hasChildren) flags |= ImGuiTreeNodeFlags.Leaf | ImGuiTreeNodeFlags.NoTreePushOnOpen;
        if (expanded)     flags |= ImGuiTreeNodeFlags.DefaultOpen;
        if (selected)     flags |= ImGuiTreeNodeFlags.Selected;

        ImGui.SetNextItemOpen(expanded, ImGuiCond.Always);

        string icon  = GetEntityIcon(node);
        string label = $"{icon} {node.Name}##{node.Entity}";
        bool nowOpen = ImGui.TreeNodeEx(label, flags);

        // 展开状态变化 → 同步缓存
        if (hasChildren && nowOpen != expanded)
            _cache.SetExpanded(node.Entity, nowOpen);

        // 点击选中
        if (ImGui.IsItemClicked(ImGuiMouseButton.Left) && !ImGui.IsItemToggledOpen())
            _cache.Select(node.Entity, ImGui.GetIO().KeyCtrl);

        // 拖放源
        if (ImGui.BeginDragDropSource())
        {
            ImGui.SetDragDropPayload("ENTITY", ref node.Entity, sizeof(ulong));
            ImGui.Text(node.Name);
            ImGui.EndDragDropSource();
        }

        // 拖放目标（重设 Parent）
        if (hasChildren && ImGui.BeginDragDropTarget())
        {
            unsafe
            {
                var payload = ImGui.AcceptDragDropPayload("ENTITY");
                if (payload.Data != null)
                {
                    ulong dragged = *(ulong*)payload.Data;
                    if (dragged != node.Entity)
                    {
                        // SceneNativeBridge.SetParent(_sceneHandle, dragged, node.Entity);
                        // EditorSceneNativeBridge → next frame version check → auto refresh
                    }
                }
            }
            ImGui.EndDragDropTarget();
        }

        // 右键菜单
        if (ImGui.BeginPopupContextItem($"##ctx_{node.Entity}"))
        {
            ImGui.Text(node.Name);
            ImGui.Separator();
            if (ImGui.MenuItem("Rename")) { }
            if (ImGui.MenuItem("Duplicate")) { }
            if (ImGui.MenuItem("Delete")) { }
            ImGui.EndPopup();
        }

        if (nowOpen && hasChildren)
            ImGui.TreePop();
    }

    private static string GetEntityIcon(HierarchyNode node) => FontAwesome5Pro.Cube;

    // ── IEditorPanel ──
    public void OnUpdate(float delta)
    {
        // 每帧轮询版本号（最热路径，1 次轻量 P/Invoke）
        _cache.TryRefresh(_sceneHandle);
    }
    public void OnFixedUpdate() { }
    public bool IsAsync() => false;
    public string GetWindowFullName() => FontAwesome5Pro.ProjectDiagram + " " + GetWindowName();
    public string GetWindowName() => "SceneBrowser";
    public void OpenWindow(bool open) => _isOpen = open;
    public bool IsWindowOpened() => _isOpen;
}
```

---

## 6. 增量 Snapshot（Phase 2 设计）

### 6.1 Native 端

```c
// Phase 2 新增函数指针（追加到 NNEditorSceneAPI 尾部）

/**
 * @brief 获取增量脏节点列表。
 *
 * 调用方传入上次拉取的 hierarchyVersion，Native 返回自该版本以来变化的节点。
 * 如果增量条目数 > capacity，返回 NN_SCENE_ERR_BUFFER_SMALL。
 *
 * @param lastKnownVersion  C# 端上次缓存的 hierarchyVersion
 * @param outEntries        输出缓冲区
 * @param maxEntries        缓冲区容量（元素数）
 * @param outCurrentVersion 输出当前 hierarchyVersion（C# 更新缓存版本）
 * @return 实际写入的脏节点数；0 = 无变化或版本差距过大（需全量重建）
 */
typedef uint32_t (NN_ENGINE_ABI_STDCALL* NNEditorGetIncrementalSnapshotFn)(
    NNSceneHandle       scene,
    uint64_t            lastKnownVersion,
    NNDirtyNodeEntry*   outEntries,
    uint32_t            maxEntries,
    uint64_t*           outCurrentVersion);
```

### 6.2 Native 实现思路

```cpp
// NNRuntimeScene 新增
class NNRuntimeScene {
    // 每次层级变化时，记录脏条目到 m_DirtyEntries
    struct DirtyEntry {
        NNEntity entity;
        uint32_t changeFlags;
    };
    std::vector<DirtyEntry> m_DirtyEntries;  // 递增变化时追加
    // getHierarchySnapshot() 调用后清空 m_DirtyEntries
    // getIncrementalSnapshot() 直接拷贝 m_DirtyEntries 到 outBuffer
};
```

### 6.3 C# 端增量合并

```csharp
// SceneHierarchyCache — Phase 2 增量方法

/// <summary>尝试增量更新（Phase 2）。返回 false 时需回退全量拉取。</summary>
public unsafe bool TryIncrementalRefresh(ulong sceneHandle)
{
    ulong nativeVersion = EditorSceneNativeBridge.GetHierarchyVersion(sceneHandle);
    if (nativeVersion == _cachedVersion) return true; // 无变化

    // 分配增量缓冲区
    const int maxDirty = 4096;
    var entries = new NNDirtyNodeEntry[maxDirty];
    ulong currentVersion = 0;

    uint count = EditorSceneNativeBridge.GetIncrementalSnapshot(
        sceneHandle, _cachedVersion, entries, maxDirty, ref currentVersion);

    if (count == 0)
        return false; // 版本差距过大，需全量重建

    // 按 changeFlags 局部更新 _allNodes
    for (int i = 0; i < count; i++)
    {
        ref readonly var entry = ref entries[i];
        if (!_entityToIndex.TryGetValue(entry.Entity, out int nodeIdx))
            continue; // 新实体，增量无法处理，需要全量

        var node = _allNodes[nodeIdx];

        if ((entry.ChangeFlags & SnapshotChangeFlags.NameChanged) != 0)
            node.Name = FetchEntityName(sceneHandle, entry.Entity);

        if ((entry.ChangeFlags & SnapshotChangeFlags.ParentChanged) != 0)
            node.Parent = FetchEntityParent(sceneHandle, entry.Entity);

        // Parent 变化 → 需要重建 DFS 序 → 回退全量
        if ((entry.ChangeFlags & SnapshotChangeFlags.ParentChanged) != 0)
            return false;
    }

    _cachedVersion = currentVersion;
    _visibleDirty  = true;
    return true;
}
```

---

## 7. 性能分析

### 7.1 帧级别操作成本

| 操作 | 旧方案 | Snapshot 方案 |
|------|--------|--------------|
| 帧轮询（无变化） | 4000 P/Invoke | **1 P/Invoke**（`getHierarchyVersion`，纯整数） |
| 首次加载 1k entities | 4000 P/Invoke | **2 P/Invoke**（`getSnapshotSize` + `getHierarchySnapshot`） |
| 帧刷新（有变化） | 4000 P/Invoke | **3 P/Invoke**（version check + size + snapshot） |
| ImGui Tree 渲染 1k visible | 递归 TreeNodeEx 1000 次 | **depth 跳跃遍历 ~1000 次**（扁平循环，无递归） |
| ImGui Tree 渲染 10k 折叠 | TreeNodeEx 10000 次 | **~1000 次**（折叠子树直接跳过，depth 跳跃） |
| 搜索过滤 10k entities | 逐 Entity 查询 | **线性扫描 _allNodes[].Name**（已缓存，0 P/Invoke） |

### 7.2 版本轮询的开销

```
getHierarchyVersion(scene)
  → SceneSubsystem::lock_guard (mutex)
  → return scene->m_HierarchyVersion.load(relaxed)
  → unlock

总开销：~50ns（atomic load + mutex lock/unlock）
对比 4000 次 P/Invoke：~4000 × 100ns = 400μs
节省：8000x
```

### 7.3 内存占用

```
10,000 entities:
  _snapshotBuffer: ~400KB（Header 32 + Nodes 400KB + NamePool ~100KB）
  _allNodes:       ~400KB（HierarchyNode 对象 + string 引用）
  _entityToIndex:  ~320KB（Dictionary<ulong, int>，预设 4096）
  _visibleList:    ~40KB （List<int>，展开状态下 ~10k 条目）
  ──────────────────
  总计：~1.2MB（10k entities）
```

---

## 8. 模块依赖图

```
┌─────────────────────────────────────────────────────────────────┐
│  Native Side                                                    │
│                                                                 │
│  NNRuntimeScene (entt::registry)                                │
│    ├── NNEntityHandleTable                                      │
│    ├── NNHierarchySystem → GetAllChildrenMap()                  │
│    ├── NNDirtyTracker → m_HierarchyVersion (atomic)             │
│    │                      m_TransformVersion (atomic)           │
│    └── NNHierarchySnapshotBuilder                               │
│         └── Build() → [Header][Nodes[]][NamePool]               │
│                                                                 │
│  NNEngineRuntime                                                │
│    ├── NNSceneAPI (layoutVersion = 6)  ← Runtime 操作           │
│    └── NNEditorSceneAPI (layoutVersion = 2) ← Editor 查询        │
│         ├── getHierarchyVersion()                               │
│         ├── getSnapshotSize()                                   │
│         ├── getHierarchySnapshot()                              │
│         ├── getTransformVersion()                               │
│         └── getTransformSnapshot()                              │
└─────────────────────────┬───────────────────────────────────────┘
                          │ ABI (delegate* unmanaged<...>)
┌─────────────────────────┴───────────────────────────────────────┐
│  C# Runtime                                                     │
│                                                                 │
│  NNNativeEngineApi                                              │
│    ├── NNSceneApi (existing)                                    │
│    └── NNEditorSceneApi (NEW)                                   │
│                                                                 │
│  SceneNativeBridge (Runtime 操作，现有)                          │
│  EditorSceneNativeBridge (Editor 查询，NEW)                      │
└─────────────────────────┬───────────────────────────────────────┘
                          │
┌─────────────────────────┴───────────────────────────────────────┐
│  C# Editor (Neverness.Editor.Scene)                             │
│                                                                 │
│  SceneHierarchyCache                                            │
│    ├── _snapshotBuffer (byte[], 复用, 翻倍扩容)                  │
│    ├── _allNodes (HierarchyNode[], DFS 顺序)                    │
│    ├── _entityToIndex (Dictionary<Entity, int>)                 │
│    ├── _expandedSet / _selectedSet (HashSet)                    │
│    ├── _visibleList (List<int>, DFS depth 跳跃)                 │
│    └── TryRefresh() → version 比较 → 按需拉取                   │
│                                                                 │
│  SceneBrowser.OnUpdate() → _cache.TryRefresh() 每帧轮询         │
│  SceneBrowser.OnGUI()    → DrawHierarchyTree() 虚拟化渲染        │
└─────────────────────────────────────────────────────────────────┘
```

---

## 9. 关键设计决策

| 决策 | 选择 | 原因 |
|------|------|------|
| **名字存储** | 名字池分离（`nameOffset` + `nameLen`） | `char[64]` 会被中日韩 UTF-8 / 自动生成长名打破；名字池无长度限制 |
| **独立函数表** | `NNEditorSceneAPI` 独立于 `NNSceneAPI` | Editor 查询与 Runtime 操作 ABI 版本独立演进；NNSceneAPI 不被 Editor 污染 |
| **触发机制** | version polling（每帧 `getHierarchyVersion`） | 比每帧 poll 全量高效（~50ns vs ~400μs），比 Native push 简单（无需回调注册） |
| **虚拟化** | 手动 clipper（scrollY / itemHeight 计算） | ImGuiListClipper 不支持动态行高和 depth 缩进；手动方案更灵活 |
| **折叠跳过** | DFS depth 跳跃 | Snapshot 保证 DFS 连续输出 + depth 字段，`while (depth > current) i++` 跳过整个子树，O(1) 判断 |
| **搜索展开** | ExpandSearchPaths 向上展开祖先链 | 确保搜索命中节点可见，且不改变其他节点的展开状态 |
| **缓冲区复用** | `_snapshotBuffer` 翻倍预留 | 避免每帧 GC；翻倍策略保证摊还 O(1) 重分配 |
| **静态断言** | 所有 POD struct 覆盖 `sizeof` + `offsetof` | 编译时捕获 ABI 不匹配，运行时不炸 |
| **增量预留** | `NNDirtyNodeEntry` + `changeFlags` 位图 | Phase 2 可局部更新缓存（name/active/flags 变化无需全量重建）；Parent 变化需回退全量 |

---

## 10. 文件变更清单

### 10.1 Native 新增

| 文件 | 说明 |
|------|------|
| `Runtime/NNNativeEngineAPI/Include/EditorSceneAPI.h` | POD 结构体 + 函数指针 + NNEditorSceneAPI 函数表 |
| `Runtime/NNRuntimeScene/Include/Snapshot/NNHierarchySnapshotBuilder.h` | 快照构建器声明 |
| `Runtime/NNRuntimeScene/Source/Snapshot/NNHierarchySnapshotBuilder.cpp` | 两遍扫描 + DFS 输出实现 |

### 10.2 Native 修改

| 文件 | 变更 |
|------|------|
| `Runtime/NNNativeEngineAPI/Include/EngineAPIRegistry.h` | NNNativeEngineApi 追加 `NNEditorSceneAPI editorScene` 字段 |
| `Runtime/NNRuntimeEngine/Include/SceneSubsystem.h` | 新增 GetHierarchyVersion / GetSnapshotSize / GetHierarchySnapshot 等方法 |
| `Runtime/NNRuntimeEngine/Private/SceneSubsystem.cpp` | 实现上述方法 |
| `Runtime/NNRuntimeEngineServices/.../SceneRuntimeApi.cpp` | 填充 NNEditorSceneAPI 函数指针 |
| `Runtime/NNRuntimeNativeEngineAPIStub/.../SceneApiStubs.cpp` | 新增 stub 实现 |
| `Runtime/NNRuntimeScene/Include/Scene/NNRuntimeScene.h` | 新增 `m_HierarchyVersion` / `m_TransformVersion` atomic 字段 |
| `Runtime/NNRuntimeScene/Include/Systems/NNHierarchySystem.h` | 新增 `GetAllChildrenMap()` 方法 |
| `Runtime/NNRuntimeScene/Include/Runtime/NNDirtyTracker.h` | 集成 hierarchyVersion 递增 |

### 10.3 C# Runtime 修改

| 文件 | 变更 |
|------|------|
| `Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs` | 新增 `NNSceneSnapshotHeader` / `NNSceneNodeSnapshot` / `NNDirtyNodeEntry` / `NNEditorTransformData` / `NNEditorSceneApi` 结构体 |
| `Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiConstants.cs` | LayoutVersion 13→14 |

### 10.4 C# Editor 新增

| 文件 | 说明 |
|------|------|
| `Managed/Editor/Neverness.Editor.Scene/Private/EditorSceneNativeBridge.cs` | Editor 专用 ABI 桥接（getHierarchyVersion / getSnapshot） |
| `Managed/Editor/Neverness.Editor.Scene/Private/Cache/SceneHierarchyCache.cs` | 核心缓存（version polling + 解析 + 可见列表） |
| `Managed/Editor/Neverness.Editor.Scene/Private/Cache/HierarchyNode.cs` | 缓存节点类 |

### 10.5 C# Editor 修改

| 文件 | 变更 |
|------|------|
| `Managed/Editor/Neverness.Editor.Scene/Private/Panel/SceneBrowser.cs` | 完全重写为 Snapshot-driven + 手动虚拟化 |
| `Managed/Editor/Neverness.Editor.Scene/Private/SceneModuleImp.cs` | 传入 sceneHandle 创建 SceneBrowser |

---

## 11. 实施路线图

| Phase | 内容 | 验证 |
|-------|------|------|
| **P1** | Native：EditorSceneAPI.h（POD + 静态断言）+ NNHierarchySnapshotBuilder | C++ 单元测试 DFS 顺序 + namePool 正确性 |
| **P2** | Native：NNEditorSceneAPI 函数表 + SceneSubsystem 实现 + stubs | stub 返回 layoutVersion=1，getHierarchyVersion 返回 0 |
| **P3** | C# Runtime：POD 结构体映射 + NNEditorSceneApi + EditorSceneNativeBridge | 编译通过，stub 调用链打通 |
| **P4** | C# Editor：SceneHierarchyCache（TryRefresh + ParseSnapshot + RebuildVisibleList） | 单元测试 version polling + depth 跳跃 |
| **P5** | C# Editor：SceneBrowser 重写（手动虚拟化 + 拖放 + 右键） | 100 entities 渲染正确 |
| **P6** | 10k entities 压力测试 + 性能调优 | < 1ms 渲染，< 100μs version check |
| **P7** | 增量 Snapshot（NNDirtyNodeEntry + changeFlags）+ Transform Snapshot | dirty tracking 生效 |

---

## 12. 架构预留扩展

| 扩展 | 预留机制 | 实现时机 |
|------|----------|----------|
| **增量 Snapshot** | `NNDirtyNodeEntry` + `changeFlags` 位图 + 函数表槽位 | Phase 7 |
| **Transform Versioning** | `m_TransformVersion` atomic + `getTransformVersion` | Phase 7 |
| **World Partition** | Header 的 `_pad` 字段可扩展为 `regionId` / `cellCount` | 未来 |
| **Multi-Scene** | `getHierarchySnapshot(scene, regionFilter, ...)` 增加过滤参数 | 未来 |
| **Prefab Stage** | `flags` bit1 标记 prefab_instance，切换 sceneHandle 实现 Stage | 未来 |
| **Selection Sync** | `getSelectionSnapshot` 预留槽位 | 未来 |
