/**
 * @file NNEntityHandle.cpp
 * @brief 实体槽位与世代表实现（与托管 EntityWorld 世代语义对齐）。
 */

#include "Scene/NNEntityHandle.h"

namespace NN::Runtime::Scene
{
void NNEntityHandleTable::EnsureSlotCapacity(const std::uint32_t index)
{
	if (index >= m_Slots.size())
	{
		m_Slots.resize(static_cast<std::size_t>(index) + 1u);
	}
}

NNEntity NNEntityHandleTable::Create(entt::registry& registry, entt::entity& outEntt) noexcept
{
	const std::uint32_t index = m_NextIndex++;
	EnsureSlotCapacity(index);

	EntitySlot& slot = m_Slots[index];
	if (slot.Alive)
	{
		outEntt = entt::null;
		return NNEntityInvalid;
	}

	outEntt = registry.create();
	slot.EnttEntity = outEntt;
	if (slot.Generation == 0u)
	{
		slot.Generation = 1u;
	}
	slot.Alive = true;
	m_EnttToIndex[static_cast<std::uint32_t>(entt::to_integral(outEntt))] = index;

	return PackEntityHandle(index, slot.Generation);
}

bool NNEntityHandleTable::Destroy(entt::registry& registry, const NNEntity handle) noexcept
{
	const NNEntityHandleParts parts = UnpackEntityHandle(handle);
	if (parts.Index == 0u)
	{
		return false;
	}
	if (parts.Index >= m_Slots.size())
	{
		return false;
	}

	EntitySlot& slot = m_Slots[parts.Index];
	if (!slot.Alive || slot.Generation != parts.Generation)
	{
		return false;
	}

	slot.Alive = false;
	slot.Generation += 1u;
	m_EnttToIndex.erase(static_cast<std::uint32_t>(entt::to_integral(slot.EnttEntity)));
	registry.destroy(slot.EnttEntity);
	slot.EnttEntity = entt::null;
	return true;
}

bool NNEntityHandleTable::IsAlive(const NNEntity handle) const noexcept
{
	const NNEntityHandleParts parts = UnpackEntityHandle(handle);
	if (parts.Index == 0u)
	{
		return false;
	}
	if (parts.Index >= m_Slots.size())
	{
		return false;
	}
	const EntitySlot& slot = m_Slots[parts.Index];
	return slot.Alive && slot.Generation == parts.Generation;
}

entt::entity NNEntityHandleTable::Resolve(const NNEntity handle) const noexcept
{
	if (!IsAlive(handle))
	{
		return entt::null;
	}
	const std::uint32_t index = UnpackEntityHandle(handle).Index;
	return m_Slots[index].EnttEntity;
}

NNEntity NNEntityHandleTable::HandleFromEntt(const entt::entity entity) const noexcept
{
	const auto it = m_EnttToIndex.find(static_cast<std::uint32_t>(entt::to_integral(entity)));
	if (it == m_EnttToIndex.end())
	{
		return NNEntityInvalid;
	}
	const std::uint32_t index = it->second;
	if (index >= m_Slots.size())
	{
		return NNEntityInvalid;
	}
	const EntitySlot& slot = m_Slots[index];
	if (!slot.Alive)
	{
		return NNEntityInvalid;
	}
	return PackEntityHandle(index, slot.Generation);
}
} // namespace NN::Runtime::Scene
