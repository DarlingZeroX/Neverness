/**
 * @file NNDirtyTracker.cpp
 * @brief **NNDirtyTracker** 实现。
 */

#include "Runtime/NNDirtyTracker.h"

namespace NN::Runtime::Scene
{
void NNDirtyTracker::MarkEntityDirty(const NNEntity entity) noexcept
{
	if (entity != NNEntityInvalid)
	{
		m_DirtyEntities.insert(entity);
	}
}

void NNDirtyTracker::MarkComponentDirty(const NNEntity entity, const NNComponentTypeId typeId) noexcept
{
	if (entity == NNEntityInvalid || typeId == NNComponentTypeIdInvalid)
	{
		return;
	}
	m_DirtyComponents[entity].insert(typeId);
	m_DirtyEntities.insert(entity);
}

std::vector<NNEntity> NNDirtyTracker::ConsumeDirtyEntities() noexcept
{
	std::vector<NNEntity> result;
	result.reserve(m_DirtyEntities.size());
	for (const NNEntity entity : m_DirtyEntities)
	{
		result.push_back(entity);
	}
	m_DirtyEntities.clear();
	return result;
}

std::vector<NNComponentTypeId> NNDirtyTracker::ConsumeDirtyComponents(const NNEntity entity) noexcept
{
	std::vector<NNComponentTypeId> result;
	const auto it = m_DirtyComponents.find(entity);
	if (it == m_DirtyComponents.end())
	{
		return result;
	}
	result.reserve(it->second.size());
	for (const NNComponentTypeId typeId : it->second)
	{
		result.push_back(typeId);
	}
	m_DirtyComponents.erase(it);
	return result;
}

void NNDirtyTracker::Clear() noexcept
{
	m_DirtyEntities.clear();
	m_DirtyComponents.clear();
}

bool NNDirtyTracker::IsEntityDirty(const NNEntity entity) const noexcept
{
	return m_DirtyEntities.find(entity) != m_DirtyEntities.end();
}
} // namespace NN::Runtime::Scene
