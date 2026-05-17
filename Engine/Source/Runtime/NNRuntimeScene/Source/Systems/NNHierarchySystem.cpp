/**
 * @file NNHierarchySystem.cpp
 * @brief **NNHierarchySystem** 实现。
 */

#include "Systems/NNHierarchySystem.h"

#include <algorithm>

#include "Components/NNRelationshipComponent.h"
#include "Runtime/NNSceneEventBus.h"
#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
void NNHierarchySystem::Tick(NNRuntimeScene& scene, const float deltaTimeSeconds) noexcept
{
	(void)scene;
	(void)deltaTimeSeconds;
}

bool NNHierarchySystem::SetParent(
	NNRuntimeScene& scene,
	const NNEntity child,
	const NNEntity parent) noexcept
{
	if (!scene.IsAlive(child))
	{
		return false;
	}
	if (parent != NNEntityInvalid && !scene.IsAlive(parent))
	{
		return false;
	}
	if (child == parent)
	{
		return false;
	}
	if (parent != NNEntityInvalid && WouldCreateCycle(child, parent))
	{
		return false;
	}

	const NNEntity oldParent = GetParent(scene, child);
	RemoveFromParentList(child);

	if (parent != NNEntityInvalid)
	{
		m_ChildToParent[child] = parent;
		m_ParentToChildren[parent].push_back(child);
	}

	SyncRelationshipComponent(scene, child);
	if (oldParent != NNEntityInvalid)
	{
		SyncRelationshipComponent(scene, oldParent);
	}
	if (parent != NNEntityInvalid)
	{
		SyncRelationshipComponent(scene, parent);
	}
	std::uint32_t childDepth = 0u;
	if (parent != NNEntityInvalid)
	{
		const NNRelationshipComponent* parentRel = scene.TryGet<NNRelationshipComponent>(parent);
		childDepth = parentRel != nullptr ? parentRel->Depth + 1u : 1u;
	}
	RecomputeDepth(scene, child, childDepth);

	NNSceneEntityEvent evt{};
	evt.Type = NNSceneEventType::ParentChanged;
	evt.Entity = child;
	evt.OtherEntity = parent;
	scene.GetEventBus().Emit(evt);
	scene.GetDirtyTracker().MarkEntityDirty(child);
	if (oldParent != NNEntityInvalid)
	{
		scene.GetDirtyTracker().MarkEntityDirty(oldParent);
	}
	if (parent != NNEntityInvalid)
	{
		scene.GetDirtyTracker().MarkEntityDirty(parent);
	}
	return true;
}

NNEntity NNHierarchySystem::GetParent(const NNRuntimeScene& scene, const NNEntity entity) const noexcept
{
	(void)scene;
	const auto it = m_ChildToParent.find(entity);
	if (it == m_ChildToParent.end())
	{
		return NNEntityInvalid;
	}
	return it->second;
}

std::vector<NNEntity> NNHierarchySystem::GetChildren(const NNRuntimeScene& scene, const NNEntity entity) const
{
	(void)scene;
	const auto it = m_ParentToChildren.find(entity);
	if (it == m_ParentToChildren.end())
	{
		return {};
	}
	return it->second;
}

void NNHierarchySystem::OnEntityDestroyed(const NNEntity entity) noexcept
{
	const NNEntity parent = m_ChildToParent.count(entity) ? m_ChildToParent[entity] : NNEntityInvalid;
	RemoveFromParentList(entity);
	m_ChildToParent.erase(entity);
	m_ParentToChildren.erase(entity);
	if (parent != NNEntityInvalid)
	{
		auto& siblings = m_ParentToChildren[parent];
		siblings.erase(
			std::remove(siblings.begin(), siblings.end(), entity),
			siblings.end());
	}
}

bool NNHierarchySystem::WouldCreateCycle(const NNEntity child, NNEntity cursor) const noexcept
{
	while (cursor != NNEntityInvalid)
	{
		if (cursor == child)
		{
			return true;
		}
		const auto it = m_ChildToParent.find(cursor);
		if (it == m_ChildToParent.end())
		{
			break;
		}
		cursor = it->second;
	}
	return false;
}

void NNHierarchySystem::RemoveFromParentList(const NNEntity child) noexcept
{
	const auto it = m_ChildToParent.find(child);
	if (it == m_ChildToParent.end())
	{
		return;
	}
	const NNEntity parent = it->second;
	m_ChildToParent.erase(it);
	auto parentIt = m_ParentToChildren.find(parent);
	if (parentIt != m_ParentToChildren.end())
	{
		auto& list = parentIt->second;
		list.erase(std::remove(list.begin(), list.end(), child), list.end());
	}
}

void NNHierarchySystem::SyncRelationshipComponent(NNRuntimeScene& scene, const NNEntity entity) noexcept
{
	if (!scene.Has<NNRelationshipComponent>(entity))
	{
		return;
	}
	NNRelationshipComponent* rel = scene.TryGet<NNRelationshipComponent>(entity);
	if (rel == nullptr)
	{
		return;
	}
	rel->Parent = GetParent(scene, entity);
	const auto childrenIt = m_ParentToChildren.find(entity);
	rel->ChildCount = childrenIt == m_ParentToChildren.end()
		? 0u
		: static_cast<std::uint32_t>(childrenIt->second.size());
}

void NNHierarchySystem::RecomputeDepth(
	NNRuntimeScene& scene,
	const NNEntity root,
	const std::uint32_t depth) noexcept
{
	if (!scene.IsAlive(root))
	{
		return;
	}
	if (scene.Has<NNRelationshipComponent>(root))
	{
		if (NNRelationshipComponent* rel = scene.TryGet<NNRelationshipComponent>(root))
		{
			rel->Depth = depth;
		}
	}
	const auto it = m_ParentToChildren.find(root);
	if (it == m_ParentToChildren.end())
	{
		return;
	}
	for (const NNEntity child : it->second)
	{
		RecomputeDepth(scene, child, depth + 1u);
	}
}
} // namespace NN::Runtime::Scene
