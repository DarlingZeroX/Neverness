#pragma once

/**
 * @file EditorSceneAPI.h
 * @brief Editor 专用场景查询函数表——独立于 NNSceneAPI（Runtime 操作）。
 *
 * 设计要点：
 * - ABI 独立：layoutVersion = 2，与 NNSceneAPI 互不影响。
 * - 名字池分离：NNSceneNodeSnapshot 通过 nameOffset + nameLen 引用 namePool，
 *   支持任意长度 UTF-8（CJK、自动生成长名）。
 * - 快照输出格式：[Header 32B][Nodes * nodeCount * 40B][NamePool * namePoolBytes]
 * - 全部 POD，无 std::string / vector，可直接 memcpy 到 C#。
 *
 * C# 对应：NNNativeEngineApiTypes.cs 中的 NNSceneSnapshotHeader / NNSceneNodeSnapshot 等。
 */

#include <stddef.h>
#include <stdint.h>

#include "NativeInterop.h"
#include "SceneAPI.h"

// ── 兼容 C / C++ 静态断言 ──
#ifdef __cplusplus
#define NN_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
extern "C" {
#else
#define NN_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

// ── Snapshot Header ──────────────────────────────────────────────────

/** @brief 快照头部——位于 buffer 开头，描述后续 Node 数组和名字池的布局。32 字节。 */
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

NN_STATIC_ASSERT(sizeof(NNSceneSnapshotHeader) == 32,
    "NNSceneSnapshotHeader must be 32 bytes");

NN_STATIC_ASSERT(offsetof(NNSceneSnapshotHeader, magic)            ==  0, "magic offset");
NN_STATIC_ASSERT(offsetof(NNSceneSnapshotHeader, layoutVersion)    ==  4, "layoutVersion offset");
NN_STATIC_ASSERT(offsetof(NNSceneSnapshotHeader, hierarchyVersion) ==  8, "hierarchyVersion offset");
NN_STATIC_ASSERT(offsetof(NNSceneSnapshotHeader, nodeCount)        == 16, "nodeCount offset");
NN_STATIC_ASSERT(offsetof(NNSceneSnapshotHeader, namePoolBytes)    == 20, "namePoolBytes offset");
NN_STATIC_ASSERT(offsetof(NNSceneSnapshotHeader, rootCount)        == 24, "rootCount offset");

// ── Node Snapshot ────────────────────────────────────────────────────

/**
 * @brief 场景层级节点快照——单个 Entity 的 Hierarchy 信息。40 字节。
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

NN_STATIC_ASSERT(sizeof(NNSceneNodeSnapshot) == 40,
    "NNSceneNodeSnapshot must be 40 bytes");

NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, entity)     ==  0, "entity offset");
NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, parent)     ==  8, "parent offset");
NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, depth)      == 16, "depth offset");
NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, childCount) == 20, "childCount offset");
NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, nameOffset) == 24, "nameOffset offset");
NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, nameLen)    == 28, "nameLen offset");
NN_STATIC_ASSERT(offsetof(NNSceneNodeSnapshot, flags)      == 32, "flags offset");

// ── Transform Snapshot ───────────────────────────────────────────────

/** @brief Transform 快照节点——批量读取 Transform 组件。48 字节。 */
typedef struct NNEditorTransformData
{
    uint64_t entity;       ///< NNEntityHandle
    float    posX, posY, posZ;
    float    rotX, rotY, rotZ, rotW;
    float    sclX, sclY, sclZ;
} NNEditorTransformData;

NN_STATIC_ASSERT(sizeof(NNEditorTransformData) == 48,
    "NNEditorTransformData must be 48 bytes");

// ── Incremental Dirty Entry（Phase 2 预留）───────────────────────────

/**
 * @brief 脏节点条目——增量快照使用。16 字节。
 * changeFlags 标记哪些属性发生了变化，C# 端按 flag 局部更新缓存。
 */
typedef struct NNDirtyNodeEntry
{
    uint64_t entity;       ///< 脏实体句柄
    uint32_t changeFlags;  ///< bit0=name bit1=parent bit2=childCount bit3=active bit4=flags
    uint32_t _pad;         ///< 对齐填充
} NNDirtyNodeEntry;

NN_STATIC_ASSERT(sizeof(NNDirtyNodeEntry) == 16,
    "NNDirtyNodeEntry must be 16 bytes");

// ── ChangeFlags 常量 ──
#define NN_DIRTY_NAME_CHANGED      (1u << 0)
#define NN_DIRTY_PARENT_CHANGED    (1u << 1)
#define NN_DIRTY_CHILDREN_CHANGED  (1u << 2)
#define NN_DIRTY_ACTIVE_CHANGED    (1u << 3)
#define NN_DIRTY_FLAGS_CHANGED     (1u << 4)

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
 * @param scene     场景句柄
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

/**
 * @brief 拷贝增量脏条目到调用方缓冲区。
 *
 * Native 内部的 dirty 向量在调用后自动清空（一次性消费）。
 * 返回写入的字节数（entryCount * sizeof(NNDirtyNodeEntry)）。
 *
 * C# 端在版本变化时，可先尝试增量拉取：若 entry 数量少于总节点数的阈值（如 10%），
 * 则逐条修补缓存；否则回退为全量快照拉取。
 *
 * @param scene       场景句柄
 * @param outBuffer   调用方预分配的缓冲区（应能容纳所有脏条目）
 * @param capacity    缓冲区容量（字节）
 */
typedef uint32_t (NN_ENGINE_ABI_STDCALL* NNEditorGetIncrementalSnapshotFn)(
    NNSceneHandle scene,
    void*         outBuffer,
    uint32_t      capacity);

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

NN_STATIC_ASSERT(sizeof(NNEditorSceneAPI) == 4 + 4 + 6 * 8,
    "NNEditorSceneAPI size mismatch: layoutVersion(4) + _pad(4) + 6 function pointers(48) = 56");

#ifdef __cplusplus
} /* extern "C" */
#endif
