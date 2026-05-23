#pragma once

/**
 * @file SceneAPI.h
 * @brief 场景 Engine Service 函数表（对齐 NNRuntimeScene ECS，C# SceneManager 驱动）。
 *
 * 纯 C 头文件：所有类型为 uint32_t / uint64_t / float，C# P/Invoke 可直接 blittable 对齐。
 * 字符串参数：须为 NUL 结尾 UTF-8；nullptr 视为 no-op / 失败回传。
 * 调用约定：全部 NN_ENGINE_ABI_STDCALL（Windows 上即 __stdcall）。
 *
 * layoutVersion（函数表首字段）：
 * - 当前 = 6
 * - 破坏性变更（重排 / 删除函数指针）时递增；仅允许尾部追加。
 *
 * 设计：C# SceneManager 负责生命周期策略，Native 端负责 ECS 数据与执行。
 * componentTypeId 使用 FNV-1a 64-bit name hash（Sprint 4-B 稳定标识）。
 */

#include <stddef.h>
#include <stdint.h>

#include "NativeInterop.h"

#ifdef __cplusplus
extern "C" {
#endif

// ── Handle ──────────────────────────────────────────────
/** @brief 场景实体 / Prefab 实例之不透明控制代码（低 32 = Index，高 32 = Generation，0 = Invalid）。 */
typedef uint64_t NNEntityHandle;
/** @brief 不透明场景 ID，0 = Invalid（与 EngineHandles.h 统一为 uint64）。 */
typedef uint64_t NNSceneHandle;

// ── Transform（blittable，C# 端 [StructLayout(Sequential)] 对齐）──
/** @brief 三维向量（位置 / 缩放）。 */
typedef struct NNVec3 { float x, y, z; } NNVec3;
/** @brief 四元数旋转。 */
typedef struct NNQuat { float x, y, z, w; } NNQuat;
/** @brief 完整变换数据（位置 + 旋转 + 缩放）。 */
typedef struct NNTransformData
{
	NNVec3 position;
	NNQuat rotation;
	NNVec3 scale;
} NNTransformData;

// ── 错误码 ───────────────────────────────────────────────
/** @brief 场景操作结果码。 */
typedef enum NNSceneResult
{
	NN_SCENE_OK               = 0, ///< 成功
	NN_SCENE_ERR_NOT_FOUND    = 1, ///< 实体 / 场景未找到
	NN_SCENE_ERR_INVALID      = 2, ///< 句柄无效（0 或世代不匹配）
	NN_SCENE_ERR_BUFFER_SMALL = 3, ///< 输出缓冲区容量不足（outRequired 含所需大小）
	NN_SCENE_ERR_IO           = 4, ///< 序列化 / 反序列化 I/O 错误
} NNSceneResult;

// ── 函数指针 ─────────────────────────────────────────────

// ── 场景管理（C# SceneManager 驱动）──

/** @brief 创建空场景；成功时 *outScene 填入场景句柄。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneCreateSceneFn)(NNSceneHandle* outScene);

/** @brief 销毁场景及其所有实体。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneDestroySceneFn)(NNSceneHandle scene);

/** @brief 驱动场景内 ECS System 调度器 Tick。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneTickSystemsFn)(NNSceneHandle scene, float deltaTime);

// ── 实体 CRUD ──

/** @brief 在场景中创建实体（含默认 Transform + Relationship + Tag 组件）；成功时 *outEntity 填入句柄。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneCreateEntityFn)(NNSceneHandle scene, NNEntityHandle* outEntity);

/** @brief 销毁场景实体。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneDestroyEntityFn)(NNSceneHandle scene, NNEntityHandle entity);

// ── 组件操作（通用，基于 FNV-1a name hash）──

/** @brief 给实体添加组件（通过 componentTypeId 标识）。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneAddComponentFn)(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId);

/** @brief 移除实体组件。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneRemoveComponentFn)(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId);

/** @brief 查询实体是否拥有指定组件；*outHas = 1 表示有。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneHasComponentFn)(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, int32_t* outHas);

/** @brief 读取组件数据到 outData（memcpy，dataSize 须匹配组件 sizeof）。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneGetComponentFn)(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, void* outData, uint32_t dataSize);

/** @brief 写入组件数据（memcpy）。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneSetComponentFn)(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, const void* data, uint32_t dataSize);

// ── 层级 ──

/** @brief 设置父子关系（child 的 Transform 相对于 parent）。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneSetParentFn)(NNSceneHandle scene, NNEntityHandle child, NNEntityHandle parent);

/** @brief 获取父实体；无父时 *outParent = 0。 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneGetParentFn)(NNSceneHandle scene, NNEntityHandle entity, NNEntityHandle* outParent);

// ── 序列化（经 VFS 路径读写 VGSC 二进制）──

/**
 * @brief 序列化场景并写入 VFS 路径（VGSC 二进制格式）。
 * @param scene   场景句柄。
 * @param vfsPath NUL 结尾 UTF-8 VFS 虚拟路径。
 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneSerializeFn)(
	NNSceneHandle scene, const char* vfsPath);

/**
 * @brief 自 VFS 路径读取 VGSC 二进制并反序列化，创建新场景。
 * @param outScene 成功时 *outScene 填入新场景句柄。
 * @param vfsPath  NUL 结尾 UTF-8 VFS 虚拟路径。
 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneDeserializeFn)(
	NNSceneHandle* outScene, const char* vfsPath);

// ── 批量查询（layoutVersion = 6 追加）──

/**
 * @brief 查询拥有指定组件的所有实体句柄。
 * @param outEntities 输出缓冲区（按匹配顺序填入句柄）。
 * @param maxCount    缓冲区容量（元素数）。
 * @param outCount    实际匹配数；若 maxCount < 匹配数则截断但 outCount 反映总数。
 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneQueryEntitiesFn)(
	NNSceneHandle scene,
	uint64_t componentTypeId,
	NNEntityHandle* outEntities,
	uint32_t maxCount,
	uint32_t* outCount);

/**
 * @brief 批量读取组件数据（减少 P/Invoke 次数）。
 * @param entities       实体句柄数组。
 * @param entityCount    实体数量。
 * @param outData        输出缓冲区（entityCount * componentSize 字节），逐实体 memcpy。
 * @param componentSize  单个组件 sizeof。
 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneQueryComponentsFn)(
	NNSceneHandle scene,
	uint64_t componentTypeId,
	const NNEntityHandle* entities,
	uint32_t entityCount,
	void* outData,
	uint32_t componentSize);

/**
 * @brief 查询同时拥有两个指定组件的实体数量。
 */
typedef NNSceneResult (NN_ENGINE_ABI_STDCALL* NNSceneQueryCount2Fn)(
	NNSceneHandle scene,
	uint64_t typeId1,
	uint64_t typeId2,
	uint32_t* outCount);

// ── 函数表 ───────────────────────────────────────────────

/** @brief 场景 Engine Service 子表（layoutVersion = 6，对齐 NNRuntimeScene ECS）。 */
typedef struct NNSceneAPI
{
	uint32_t layoutVersion; ///< 当前 = 6；破坏性变更时递增

	NNSceneCreateSceneFn     createScene;
	NNSceneDestroySceneFn    destroyScene;
	NNSceneTickSystemsFn     tickSystems;

	NNSceneCreateEntityFn    createEntity;
	NNSceneDestroyEntityFn   destroyEntity;

	NNSceneAddComponentFn    addComponent;
	NNSceneRemoveComponentFn removeComponent;
	NNSceneHasComponentFn    hasComponent;
	NNSceneGetComponentFn    getComponent;
	NNSceneSetComponentFn    setComponent;

	NNSceneSetParentFn       setParent;
	NNSceneGetParentFn       getParent;

	NNSceneSerializeFn       serializeScene;
	NNSceneDeserializeFn     deserializeScene;

	// ── 批量查询（layoutVersion = 6 追加）──
	NNSceneQueryEntitiesFn   queryEntities;
	NNSceneQueryComponentsFn queryComponents;
	NNSceneQueryCount2Fn     queryCount2;
} NNSceneAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
