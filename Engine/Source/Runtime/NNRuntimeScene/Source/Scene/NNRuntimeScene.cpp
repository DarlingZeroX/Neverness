/**
 * @file NNRuntimeScene.cpp
 * @brief NNRuntimeScene 实现：实体生命周期、System、事件与脏跟踪。
 */

#include "Scene/NNRuntimeScene.h"

#include "NNNativeEngineAPI/Include/EditorSceneAPI.h"
#include "Runtime/NNSceneEventBus.h"

namespace NN::Runtime::Scene
{
NNRuntimeScene::NNRuntimeScene()
{
	NNComponentRegistryGlobal::Instance().MergeInto(m_ComponentRegistry);
	RegisterBuiltinComponents();
	RegisterDefaultSystems();
}

NNRuntimeScene::~NNRuntimeScene() = default;

void NNRuntimeScene::RegisterBuiltinComponents()
{
	// 字段元数据由 BuiltinComponentRegistration.cpp 静态注册并 MergeInto；
	// 若静态注册未执行（单测静态链接），回退为无字段的类型登记。
	if (m_ComponentRegistry.FindDescByNameHash(fnv1a_64("Transform")) == nullptr)
	{
		(void)m_ComponentRegistry.Register<NNTransformComponent>("Transform");
		(void)m_ComponentRegistry.Register<NNRelationshipComponent>("Relationship");
		(void)m_ComponentRegistry.Register<NNTagComponent>("Tag");
		(void)m_ComponentRegistry.Register<NNCameraComponent>("Camera");
	}

	// Phase 5：绑定 Runtime Type-Erased ECS Access 函数指针
	// 已有字段元数据（来自 NN_REGISTER_COMPONENT 宏）会被合并，函数指针补充到描述符中
	BindComponentType<NNTransformComponent>("Transform");
	BindComponentType<NNRelationshipComponent>("Relationship");
	// Relationship 反序列化后需要调用 SetParent 恢复层级关系
	{
		auto* desc = m_ComponentRegistry.FindDescByNameHash(fnv1a_64("Relationship"));
		if (desc != nullptr)
		{
			desc->PostDeserializeFn = [](NNRuntimeScene& scene, NNEntity entity)
			{
				auto* rel = scene.TryGet<NNRelationshipComponent>(entity);
				if (rel != nullptr && rel->Parent != NNEntityInvalid)
				{
					scene.SetParent(entity, rel->Parent);
				}
			};
		}
	}
	BindComponentType<NNTagComponent>("Tag");
	BindComponentType<NNCameraComponent>("Camera");
	BindComponentType<NNSpriteRendererComponent>("SpriteRenderer");
}

void NNRuntimeScene::RegisterDefaultSystems()
{
	RegisterSystem(&m_HierarchySystem);
	RegisterSystem(&m_SceneUpdateSystem);
	RegisterSystem(&m_TransformSystem);
	RegisterSystem(&m_CameraSystem);
}

void NNRuntimeScene::RegisterSystem(ISceneSystem* const system) noexcept
{
	m_SystemScheduler.Register(system);
}

void NNRuntimeScene::TickSystems(const float deltaTimeSeconds) noexcept
{
	m_SystemScheduler.Tick(*this, deltaTimeSeconds);
}

NNEntity NNRuntimeScene::CreateEntity() noexcept
{
	entt::entity enttEntity = entt::null;
	const NNEntity handle = m_EntityTable.Create(m_Registry, enttEntity);
	if (handle == NNEntityInvalid)
	{
		return NNEntityInvalid;
	}

	NNSceneEntityEvent evt{};
	evt.Type = NNSceneEventType::EntityCreated;
	evt.Entity = handle;
	m_EventBus.Emit(evt);
	m_DirtyTracker.MarkEntityDirty(handle);
	MarkHierarchyDirty(handle, NN_DIRTY_NAME_CHANGED | NN_DIRTY_PARENT_CHANGED | NN_DIRTY_CHILDREN_CHANGED);
	IncrementHierarchyVersion();
	return handle;
}

NNEntity NNRuntimeScene::CreateEntityWithDefaults() noexcept
{
	const NNEntity handle = CreateEntity();
	if (handle == NNEntityInvalid)
	{
		return NNEntityInvalid;
	}
	Emplace<NNTransformComponent>(handle);
	Emplace<NNRelationshipComponent>(handle);
	Emplace<NNTagComponent>(handle);
	return handle;
}

bool NNRuntimeScene::DestroyEntity(const NNEntity handle) noexcept
{
	if (!IsAlive(handle))
	{
		return false;
	}

	m_HierarchySystem.OnEntityDestroyed(handle);

	NNSceneEntityEvent evt{};
	evt.Type = NNSceneEventType::EntityDestroyed;
	evt.Entity = handle;
	m_EventBus.Emit(evt);
	m_DirtyTracker.MarkEntityDirty(handle);
	MarkHierarchyDirty(handle, NN_DIRTY_NAME_CHANGED | NN_DIRTY_PARENT_CHANGED | NN_DIRTY_CHILDREN_CHANGED);

	const bool destroyed = m_EntityTable.Destroy(m_Registry, handle);
	if (destroyed)
	{
		m_DirtyTracker.MarkEntityDirty(handle);
		IncrementHierarchyVersion();
	}
	return destroyed;
}

bool NNRuntimeScene::IsAlive(const NNEntity handle) const noexcept
{
	return m_EntityTable.IsAlive(handle);
}

bool NNRuntimeScene::SetParent(const NNEntity child, const NNEntity parent) noexcept
{
	const bool ok = m_HierarchySystem.SetParent(*this, child, parent);
	if (ok)
	{
		// child：parent + children 均变化
		MarkHierarchyDirty(child, NN_DIRTY_PARENT_CHANGED | NN_DIRTY_CHILDREN_CHANGED);
		// parent：children 列表变化
		if (parent != NNEntityInvalid)
		{
			MarkHierarchyDirty(parent, NN_DIRTY_CHILDREN_CHANGED);
		}
		IncrementHierarchyVersion();
	}
	return ok;
}

NNEntity NNRuntimeScene::GetParent(const NNEntity entity) const noexcept
{
	return m_HierarchySystem.GetParent(*this, entity);
}

std::vector<NNEntity> NNRuntimeScene::GetChildren(const NNEntity entity) const
{
	return m_HierarchySystem.GetChildren(*this, entity);
}

NNEntity NNRuntimeScene::HandleFromEntt(const entt::entity entity) const noexcept
{
	return m_EntityTable.HandleFromEntt(entity);
}

void NNRuntimeScene::MarkHierarchyDirty(const NNEntity entity, const std::uint32_t changeFlags) noexcept
{
	// 查找已有条目，合并 changeFlags（同一实体多次变更只保留最终标志位）
	for (auto& entry : m_DirtyHierarchyEntries)
	{
		if (entry.entity == entity)
		{
			entry.changeFlags |= changeFlags;
			return;
		}
	}
	m_DirtyHierarchyEntries.push_back({entity, changeFlags});
}
} // namespace NN::Runtime::Scene
