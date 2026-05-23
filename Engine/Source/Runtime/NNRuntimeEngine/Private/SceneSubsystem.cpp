/**
 * @file SceneSubsystem.cpp
 * @brief 场景子系统实现：管理 NNRuntimeScene 实例，桥接 C 平面 API（layoutVersion = 4）。
 */

#include "SceneSubsystem.h"

#include <cstring>

#include "NNRuntimeScene/Include/Components/NNRelationshipComponent.h"
#include "NNRuntimeScene/Include/Components/NNTagComponent.h"
#include "NNRuntimeScene/Include/Components/NNTransformComponent.h"
#include "NNRuntimeScene/Include/Reflection/NNComponentRegistry.h"
#include "NNRuntimeScene/Include/Scene/NNRuntimeScene.h"
#include "NNRuntimeScene/Include/Serialization/NNSceneSerializer.h"

using NN::Runtime::Scene::NNRuntimeScene;
using NN::Runtime::Scene::NNSceneSerializer;
using NN::Runtime::Scene::NNComponentRegistry;
using NN::Runtime::Scene::NNComponentTypeDesc;
using NN::Runtime::Scene::NNEntity;
using NN::Runtime::Scene::NNEntityInvalid;
using NN::Runtime::Scene::NNTransformComponent;
using NN::Runtime::Scene::NNRelationshipComponent;
using NN::Runtime::Scene::NNTagComponent;

namespace NN::Runtime::engine
{
NNRuntimeScene* SceneSubsystem::FindScene(const NNSceneHandle handle) noexcept
{
	const auto it = scenes_.find(handle);
	return (it != scenes_.end()) ? it->second.get() : nullptr;
}

// ── 场景管理 ──

NNSceneResult SceneSubsystem::CreateScene(NNSceneHandle* outScene) noexcept
{
	if (outScene == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	const NNSceneHandle handle = nextHandle_++;
	auto scene = std::make_unique<NNRuntimeScene>();
	scenes_.emplace(handle, std::move(scene));
	*outScene = handle;
	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::DestroyScene(const NNSceneHandle scene) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	const auto it = scenes_.find(scene);
	if (it == scenes_.end())
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}
	scenes_.erase(it);
	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::TickSystems(const NNSceneHandle scene, const float deltaTime) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}
	s->TickSystems(deltaTime);
	return NN_SCENE_OK;
}

// ── 实体 CRUD ──

NNSceneResult SceneSubsystem::CreateEntity(const NNSceneHandle scene, NNEntityHandle* outEntity) noexcept
{
	if (outEntity == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	/* 创建实体（含默认 Transform + Relationship + Tag 组件） */
	const NNEntity entity = s->CreateEntityWithDefaults();
	*outEntity = static_cast<NNEntityHandle>(entity);
	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::DestroyEntity(const NNSceneHandle scene, const NNEntityHandle entity) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const NNEntity e = static_cast<NNEntity>(entity);
	if (!s->IsAlive(e))
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}
	s->DestroyEntity(e);
	return NN_SCENE_OK;
}

// ── 组件操作 ──

namespace
{
/** @brief 根据 componentTypeId（FNV-1a hash）获取组件数据指针和大小。 */
struct ComponentAccess
{
	const void* data = nullptr;
	std::size_t size = 0;
};

ComponentAccess GetComponentRead(
	NNRuntimeScene* scene,
	const NNEntity entity,
	const std::uint64_t componentTypeId)
{
	const NNComponentRegistry& registry = scene->GetComponentRegistry();
	const NNComponentTypeDesc* desc = registry.FindDescByNameHash(componentTypeId);
	if (desc == nullptr)
	{
		return {};
	}

	if (desc->TypeIndex == std::type_index(typeid(NNTransformComponent)))
	{
		auto* ptr = scene->TryGet<NNTransformComponent>(entity);
		return {ptr, sizeof(NNTransformComponent)};
	}
	if (desc->TypeIndex == std::type_index(typeid(NNRelationshipComponent)))
	{
		auto* ptr = scene->TryGet<NNRelationshipComponent>(entity);
		return {ptr, sizeof(NNRelationshipComponent)};
	}
	if (desc->TypeIndex == std::type_index(typeid(NNTagComponent)))
	{
		auto* ptr = scene->TryGet<NNTagComponent>(entity);
		return {ptr, sizeof(NNTagComponent)};
	}
	return {};
}

void* GetComponentWrite(
	NNRuntimeScene* scene,
	const NNEntity entity,
	const std::uint64_t componentTypeId)
{
	const NNComponentRegistry& registry = scene->GetComponentRegistry();
	const NNComponentTypeDesc* desc = registry.FindDescByNameHash(componentTypeId);
	if (desc == nullptr)
	{
		return nullptr;
	}

	if (desc->TypeIndex == std::type_index(typeid(NNTransformComponent)))
	{
		return scene->TryGet<NNTransformComponent>(entity);
	}
	if (desc->TypeIndex == std::type_index(typeid(NNRelationshipComponent)))
	{
		return scene->TryGet<NNRelationshipComponent>(entity);
	}
	if (desc->TypeIndex == std::type_index(typeid(NNTagComponent)))
	{
		return scene->TryGet<NNTagComponent>(entity);
	}
	return nullptr;
}
} // namespace

NNSceneResult SceneSubsystem::AddComponent(
	const NNSceneHandle scene,
	const NNEntityHandle entity,
	const std::uint64_t componentTypeId) noexcept
{
	/* 当前设计：CreateEntityWithDefaults 已添加所有内置组件，
	   AddComponent 用于未来扩展组件（需 entt registry 支持动态添加）。 */
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::RemoveComponent(
	const NNSceneHandle scene,
	const NNEntityHandle entity,
	const std::uint64_t componentTypeId) noexcept
{
	/* 当前设计：内置组件不可移除（Transform/Relationship/Tag 是必需的）。 */
	(void)scene;
	(void)entity;
	(void)componentTypeId;
	return NN_SCENE_ERR_INVALID;
}

NNSceneResult SceneSubsystem::HasComponent(
	const NNSceneHandle scene,
	const NNEntityHandle entity,
	const std::uint64_t componentTypeId,
	int32_t* outHas) noexcept
{
	if (outHas == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}
	*outHas = 0;

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const auto access = GetComponentRead(s, static_cast<NNEntity>(entity), componentTypeId);
	*outHas = (access.data != nullptr) ? 1 : 0;
	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::GetComponent(
	const NNSceneHandle scene,
	const NNEntityHandle entity,
	const std::uint64_t componentTypeId,
	void* outData,
	const uint32_t dataSize) noexcept
{
	if (outData == nullptr || dataSize == 0)
	{
		return NN_SCENE_ERR_INVALID;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const auto access = GetComponentRead(s, static_cast<NNEntity>(entity), componentTypeId);
	if (access.data == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}
	if (dataSize < access.size)
	{
		return NN_SCENE_ERR_BUFFER_SMALL;
	}

	std::memcpy(outData, access.data, access.size);
	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::SetComponent(
	const NNSceneHandle scene,
	const NNEntityHandle entity,
	const std::uint64_t componentTypeId,
	const void* data,
	const uint32_t dataSize) noexcept
{
	if (data == nullptr || dataSize == 0)
	{
		return NN_SCENE_ERR_INVALID;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	void* dst = GetComponentWrite(s, static_cast<NNEntity>(entity), componentTypeId);
	if (dst == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	std::memcpy(dst, data, dataSize);
	return NN_SCENE_OK;
}

// ── 层级 ──

NNSceneResult SceneSubsystem::SetParent(
	const NNSceneHandle scene,
	const NNEntityHandle child,
	const NNEntityHandle parent) noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const NNEntity c = static_cast<NNEntity>(child);
	const NNEntity p = (parent != 0) ? static_cast<NNEntity>(parent) : NNEntityInvalid;

	if (!s->IsAlive(c))
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	return s->SetParent(c, p) ? NN_SCENE_OK : NN_SCENE_ERR_INVALID;
}

NNSceneResult SceneSubsystem::GetParent(
	const NNSceneHandle scene,
	const NNEntityHandle entity,
	NNEntityHandle* outParent) noexcept
{
	if (outParent == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}
	*outParent = 0;

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const NNEntity e = static_cast<NNEntity>(entity);
	if (!s->IsAlive(e))
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const NNEntity parent = s->GetParent(e);
	*outParent = static_cast<NNEntityHandle>(parent);
	return NN_SCENE_OK;
}

// ── 序列化（经 VFS 路径）──

NNSceneResult SceneSubsystem::SerializeScene(
	const NNSceneHandle scene,
	const char* vfsPath) noexcept
{
	if (vfsPath == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}

	if (vfs_ == nullptr || vfs_->writeBufferToFile == nullptr)
	{
		return NN_SCENE_ERR_IO;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	/* 序列化为 VGSC 二进制 blob */
	const std::vector<std::uint8_t> blob = NNSceneSerializer::Serialize(*s);

	/* 经 VFS writeBufferToFile 直接写入二进制 */
	return vfs_->writeBufferToFile(
		vfsPath,
		blob.data(),
		static_cast<std::uint64_t>(blob.size())) != 0 ? NN_SCENE_OK : NN_SCENE_ERR_IO;
}

NNSceneResult SceneSubsystem::DeserializeScene(
	NNSceneHandle* outScene,
	const char* vfsPath) noexcept
{
	if (outScene == nullptr || vfsPath == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}

	if (vfs_ == nullptr || vfs_->readBytes == nullptr)
	{
		return NN_SCENE_ERR_IO;
	}

	/* 经 VFS readBytes 读取二进制 */
	std::uint8_t* rawData = nullptr;
	std::uint32_t rawSize = 0;
	if (vfs_->readBytes(vfsPath, &rawData, &rawSize) == 0 || rawData == nullptr)
	{
		return NN_SCENE_ERR_IO;
	}

	const std::vector<std::uint8_t> blob(rawData, rawData + rawSize);

	if (vfs_->freeBuffer != nullptr)
	{
		vfs_->freeBuffer(rawData);
	}

	/* 创建新场景 */
	std::lock_guard<std::mutex> lock(mutex_);
	const NNSceneHandle handle = nextHandle_++;
	auto scene = std::make_unique<NNRuntimeScene>();

	if (!NNSceneSerializer::Deserialize(*scene, blob))
	{
		return NN_SCENE_ERR_IO;
	}

	scenes_.emplace(handle, std::move(scene));
	*outScene = handle;
	return NN_SCENE_OK;
}

void SceneSubsystem::SetVfsApi(const NNVfsAPI* vfs) noexcept
{
	vfs_ = vfs;
}

// ── 批量查询（layoutVersion = 6）──

NNSceneResult SceneSubsystem::QueryEntities(
	const NNSceneHandle scene,
	const uint64_t componentTypeId,
	NNEntityHandle* outEntities,
	const uint32_t maxCount,
	uint32_t* outCount) noexcept
{
	if (outCount == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}
	*outCount = 0;

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	const NNComponentRegistry& registry = s->GetComponentRegistry();
	const NNComponentTypeDesc* desc = registry.FindDescByNameHash(componentTypeId);
	if (desc == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	/* 类型分派：遍历注册组件的 entt view，收集 NNEntity 句柄 */
	auto collect = [&](auto typeTag) -> NNSceneResult
	{
		using ComponentType = decltype(typeTag);
		auto view = s->GetRegistry().view<ComponentType>();
		uint32_t count = 0;
		for (const auto enttEntity : view)
		{
			const NNEntity handle = s->HandleFromEntt(enttEntity);
			if (!s->IsAlive(handle))
			{
				continue;
			}
			if (outEntities != nullptr && count < maxCount)
			{
				outEntities[count] = static_cast<NNEntityHandle>(handle);
			}
			++count;
		}
		*outCount = count;
		return NN_SCENE_OK;
	};

	if (desc->TypeIndex == std::type_index(typeid(NNTransformComponent)))
	{
		return collect(NNTransformComponent{});
	}
	if (desc->TypeIndex == std::type_index(typeid(NNRelationshipComponent)))
	{
		return collect(NNRelationshipComponent{});
	}
	if (desc->TypeIndex == std::type_index(typeid(NNTagComponent)))
	{
		return collect(NNTagComponent{});
	}

	return NN_SCENE_ERR_NOT_FOUND;
}

NNSceneResult SceneSubsystem::QueryComponents(
	const NNSceneHandle scene,
	const uint64_t componentTypeId,
	const NNEntityHandle* entities,
	const uint32_t entityCount,
	void* outData,
	const uint32_t componentSize) noexcept
{
	if (entities == nullptr || outData == nullptr || entityCount == 0)
	{
		return NN_SCENE_ERR_INVALID;
	}

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	auto* dst = static_cast<std::uint8_t*>(outData);
	for (uint32_t i = 0; i < entityCount; ++i)
	{
		const auto access = GetComponentRead(s, static_cast<NNEntity>(entities[i]), componentTypeId);
		if (access.data != nullptr && access.size <= componentSize)
		{
			std::memcpy(dst + static_cast<std::size_t>(i) * componentSize, access.data, access.size);
		}
		else
		{
			std::memset(dst + static_cast<std::size_t>(i) * componentSize, 0, componentSize);
		}
	}

	return NN_SCENE_OK;
}

NNSceneResult SceneSubsystem::QueryCount2(
	const NNSceneHandle scene,
	const uint64_t typeId1,
	const uint64_t typeId2,
	uint32_t* outCount) noexcept
{
	if (outCount == nullptr)
	{
		return NN_SCENE_ERR_INVALID;
	}
	*outCount = 0;

	std::lock_guard<std::mutex> lock(mutex_);
	NNRuntimeScene* s = FindScene(scene);
	if (s == nullptr)
	{
		return NN_SCENE_ERR_NOT_FOUND;
	}

	/* 简化实现：复用 QueryEntities 对 typeId1 的结果，逐个检查 typeId2 */
	uint32_t singleCount = 0;
	NNSceneResult result = QueryEntities(scene, typeId1, nullptr, 0, &singleCount);
	if (result != NN_SCENE_OK)
	{
		return result;
	}

	if (singleCount == 0)
	{
		return NN_SCENE_OK;
	}

	/* 使用栈缓冲区获取 typeId1 的实体列表 */
	std::vector<NNEntityHandle> handles(singleCount);
	result = QueryEntities(scene, typeId1, handles.data(), singleCount, &singleCount);
	if (result != NN_SCENE_OK)
	{
		return result;
	}

	uint32_t intersectionCount = 0;
	int32_t has = 0;
	for (uint32_t i = 0; i < singleCount; ++i)
	{
		has = 0;
		HasComponent(scene, handles[i], typeId2, &has);
		if (has)
		{
			++intersectionCount;
		}
	}

	*outCount = intersectionCount;
	return NN_SCENE_OK;
}
} // namespace NN::Runtime::engine
