/**
 * @file NNRuntimeScene.cpp
 * @brief NNRuntimeScene 实现：实体生命周期、System、事件与脏跟踪。
 */

#include "Scene/NNRuntimeScene.h"

#include <typeindex>

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
	if (m_ComponentRegistry.FindTypeId(std::type_index(typeid(NNTransformComponent))) ==
		NNComponentTypeIdInvalid)
	{
		(void)m_ComponentRegistry.Register<NNTransformComponent>("Transform");
		(void)m_ComponentRegistry.Register<NNRelationshipComponent>("Relationship");
		(void)m_ComponentRegistry.Register<NNTagComponent>("Tag");
	}
}

void NNRuntimeScene::RegisterDefaultSystems()
{
	RegisterSystem(&m_HierarchySystem);
	RegisterSystem(&m_SceneUpdateSystem);
	RegisterSystem(&m_TransformSystem);
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

	const bool destroyed = m_EntityTable.Destroy(m_Registry, handle);
	if (destroyed)
	{
		m_DirtyTracker.MarkEntityDirty(handle);
	}
	return destroyed;
}

bool NNRuntimeScene::IsAlive(const NNEntity handle) const noexcept
{
	return m_EntityTable.IsAlive(handle);
}

bool NNRuntimeScene::SetParent(const NNEntity child, const NNEntity parent) noexcept
{
	return m_HierarchySystem.SetParent(*this, child, parent);
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
} // namespace NN::Runtime::Scene
