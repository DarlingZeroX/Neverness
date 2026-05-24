#pragma once

/**
 * @file SceneSubsystem.h
 * @brief 场景子系统：管理 NNRuntimeScene 实例，桥接 C 平面 API。
 *
 * 方法签名对齐 NNSceneAPI ABI（layoutVersion = 4）：
 * - 返回值统一为 NNSceneResult
 * - Handle 统一为 uint64_t
 * - 组件操作基于 FNV-1a name hash（componentTypeId）
 *
 * 本类不再持有自有 Entity 存储，所有实体操作委托 NNRuntimeScene（entt::registry）。
 */

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "NNNativeEngineAPI/Include/EditorSceneAPI.h"
#include "NNNativeEngineAPI/Include/EngineHandles.h"
#include "NNNativeEngineAPI/Include/SceneAPI.h"
#include "NNNativeEngineAPI/Include/VfsAPI.h"

namespace NN::Runtime::Scene
{
	class NNRuntimeScene;  /* 前向声明，避免引入完整头 */
}

namespace NN::Runtime::engine
{
/** @brief 场景子系统：管理 NNRuntimeScene 实例的生命周期与 ECS 操作。 */
class SceneSubsystem final
{
public:
	// ── 场景管理 ──
	NNSceneResult CreateScene(NNSceneHandle* outScene) noexcept;
	NNSceneResult DestroyScene(NNSceneHandle scene) noexcept;
	NNSceneResult TickSystems(NNSceneHandle scene, float deltaTime) noexcept;

	// ── 实体 CRUD ──
	NNSceneResult CreateEntity(NNSceneHandle scene, NNEntityHandle* outEntity) noexcept;
	NNSceneResult DestroyEntity(NNSceneHandle scene, NNEntityHandle entity) noexcept;

	// ── 组件操作（基于 FNV-1a name hash）──
	NNSceneResult AddComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId) noexcept;
	NNSceneResult RemoveComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId) noexcept;
	NNSceneResult HasComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, int32_t* outHas) noexcept;
	NNSceneResult GetComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, void* outData, uint32_t dataSize) noexcept;
	NNSceneResult SetComponent(NNSceneHandle scene, NNEntityHandle entity, uint64_t componentTypeId, const void* data, uint32_t dataSize) noexcept;

	// ── 层级 ──
	NNSceneResult SetParent(NNSceneHandle scene, NNEntityHandle child, NNEntityHandle parent) noexcept;
	NNSceneResult GetParent(NNSceneHandle scene, NNEntityHandle entity, NNEntityHandle* outParent) noexcept;

	// ── 序列化（经 VFS 路径）──
	NNSceneResult SerializeScene(NNSceneHandle scene, const char* vfsPath) noexcept;
	NNSceneResult DeserializeScene(NNSceneHandle* outScene, const char* vfsPath) noexcept;

	// ── 批量查询（layoutVersion = 6）──
	NNSceneResult QueryEntities(NNSceneHandle scene, uint64_t componentTypeId,
		NNEntityHandle* outEntities, uint32_t maxCount, uint32_t* outCount) noexcept;
	NNSceneResult QueryComponents(NNSceneHandle scene, uint64_t componentTypeId,
		const NNEntityHandle* entities, uint32_t entityCount,
		void* outData, uint32_t componentSize) noexcept;
	NNSceneResult QueryCount2(NNSceneHandle scene, uint64_t typeId1, uint64_t typeId2,
		uint32_t* outCount) noexcept;

	// ── Editor 快照查询（NNEditorSceneAPI，layoutVersion = 2）──
	uint64_t GetHierarchyVersion(NNSceneHandle scene) noexcept;
	uint32_t GetSnapshotSize(NNSceneHandle scene) noexcept;
	uint32_t GetHierarchySnapshot(NNSceneHandle scene, void* outBuffer, uint32_t capacity) noexcept;
	uint64_t GetTransformVersion(NNSceneHandle scene) noexcept;
	uint32_t GetTransformSnapshot(NNSceneHandle scene, const uint64_t* entities,
		uint32_t entityCount, NNEditorTransformData* outArray) noexcept;
	uint32_t GetIncrementalSnapshot(NNSceneHandle scene, void* outBuffer, uint32_t capacity) noexcept;

	/** @brief 设置 VFS API 函数指针（由 EngineServices 在构建 API 表后注入）。 */
	void SetVfsApi(const NNVfsAPI* vfs) noexcept;

private:
	/** @brief 根据句柄查找 NNRuntimeScene 实例。 */
	NN::Runtime::Scene::NNRuntimeScene* FindScene(NNSceneHandle handle) noexcept;

	mutable std::mutex mutex_{};
	std::unordered_map<NNSceneHandle, std::unique_ptr<NN::Runtime::Scene::NNRuntimeScene>> scenes_{};
	NNSceneHandle nextHandle_{1};  /* 从 1 开始，0 = Invalid */
	const NNVfsAPI* vfs_{nullptr}; /* VFS 函数指针（可选；序列化时需要） */
};
} // namespace NN::Runtime::engine
