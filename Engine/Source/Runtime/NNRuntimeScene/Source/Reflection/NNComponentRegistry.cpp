/**
 * @file NNComponentRegistry.cpp
 * @brief 组件类型注册表实现（FNV-1a 稳定 TypeId，Phase 4-B）。
 */

#include "Reflection/NNComponentRegistry.h"

#include <cstring>

namespace NN::Runtime::Scene
{
NNComponentRegistryGlobal& NNComponentRegistryGlobal::Instance() noexcept
{
	static NNComponentRegistryGlobal instance;
	return instance;
}

void NNComponentRegistryGlobal::MergeInto(NNComponentRegistry& destination) const
{
	for (const NNComponentTypeDesc& desc : m_Registry.m_Descriptors)
	{
		// 使用完整描述符注册，保留函数指针（Phase 5 Runtime Access Layer）
		(void)destination.RegisterTypeWithFields(desc);
	}
}

NNComponentTypeId NNComponentRegistry::RegisterTypeWithFields(
	const std::type_index typeIndex,
	const char* nameUtf8,
	const std::size_t sizeBytes,
	const std::initializer_list<NNComponentFieldDesc> fields)
{
	return RegisterType(typeIndex, nameUtf8, sizeBytes, fields);
}

NNComponentTypeId NNComponentRegistry::RegisterTypeWithFields(
	const std::type_index typeIndex,
	const char* nameUtf8,
	const std::size_t sizeBytes,
	const std::vector<NNComponentFieldDesc>& fields)
{
	const std::uint64_t nameHash = fnv1a_64(nameUtf8);

	// 查重：按 nameHash 幂等（同一名称重复注册返回已有 typeId）
	if (const auto it = m_NameHashToDesc.find(nameHash); it != m_NameHashToDesc.end())
	{
		return m_Descriptors[it->second].TypeId;
	}

	// 查重：按 typeIndex（同一 C++ 类型重复注册返回已有 typeId）
	if (const auto found = m_TypeToId.find(typeIndex); found != m_TypeToId.end())
	{
		return found->second;
	}

	const NNComponentTypeId typeId = nameHash;
	const std::size_t descIndex = m_Descriptors.size();

	NNComponentTypeDesc desc{};
	desc.TypeId = typeId;
	desc.NameHash = nameHash;
	desc.TypeIndex = typeIndex;
	desc.NameUtf8 = nameUtf8;
	desc.SizeBytes = sizeBytes;
	desc.Fields = fields;

	m_TypeToId.emplace(typeIndex, typeId);
	m_NameHashToDesc.emplace(nameHash, descIndex);
	m_Descriptors.push_back(std::move(desc));
	return typeId;
}

NNComponentTypeId NNComponentRegistry::RegisterTypeWithFields(const NNComponentTypeDesc& fullDesc)
{
	const std::uint64_t nameHash = fullDesc.NameHash;

	// 查重：按 nameHash → 合并函数指针到已有描述符
	if (const auto it = m_NameHashToDesc.find(nameHash); it != m_NameHashToDesc.end())
	{
		NNComponentTypeDesc& existing = m_Descriptors[it->second];

		// 合并函数指针（仅覆盖非 nullptr 的字段，保留已有绑定）
		if (fullDesc.GetComponentPtrFn)       existing.GetComponentPtrFn       = fullDesc.GetComponentPtrFn;
		if (fullDesc.GetComponentConstPtrFn)  existing.GetComponentConstPtrFn  = fullDesc.GetComponentConstPtrFn;
		if (fullDesc.HasComponentFn)          existing.HasComponentFn          = fullDesc.HasComponentFn;
		if (fullDesc.AddComponentFn)          existing.AddComponentFn          = fullDesc.AddComponentFn;
		if (fullDesc.RemoveComponentFn)       existing.RemoveComponentFn       = fullDesc.RemoveComponentFn;
		if (fullDesc.ForEachEntityFn)         existing.ForEachEntityFn         = fullDesc.ForEachEntityFn;
		if (fullDesc.PostDeserializeFn)       existing.PostDeserializeFn       = fullDesc.PostDeserializeFn;
		if (fullDesc.SerializeFn)             existing.SerializeFn             = fullDesc.SerializeFn;

		return existing.TypeId;
	}

	// 新注册：按 typeIndex 查重
	if (const auto found = m_TypeToId.find(fullDesc.TypeIndex); found != m_TypeToId.end())
	{
		return found->second;
	}

	const NNComponentTypeId typeId = fullDesc.TypeId != NNComponentTypeIdInvalid
		? fullDesc.TypeId
		: nameHash;
	const std::size_t descIndex = m_Descriptors.size();

	NNComponentTypeDesc desc = fullDesc;
	desc.TypeId = typeId;
	desc.NameHash = nameHash;

	m_TypeToId.emplace(desc.TypeIndex, typeId);
	m_NameHashToDesc.emplace(nameHash, descIndex);
	m_Descriptors.push_back(std::move(desc));
	return typeId;
}

NNComponentTypeId NNComponentRegistry::RegisterType(
	const std::type_index typeIndex,
	const char* nameUtf8,
	const std::size_t sizeBytes,
	const std::initializer_list<NNComponentFieldDesc> fields)
{
	std::vector<NNComponentFieldDesc> fieldVec(fields.begin(), fields.end());
	return RegisterTypeWithFields(typeIndex, nameUtf8, sizeBytes, fieldVec);
}

NNComponentTypeId NNComponentRegistry::FindTypeId(const std::type_index& typeIndex) const noexcept
{
	const auto it = m_TypeToId.find(typeIndex);
	if (it == m_TypeToId.end())
	{
		return NNComponentTypeIdInvalid;
	}
	return it->second;
}

const NNComponentTypeDesc* NNComponentRegistry::FindDesc(const NNComponentTypeId typeId) const noexcept
{
	for (const auto& desc : m_Descriptors)
	{
		if (desc.TypeId == typeId)
		{
			return &desc;
		}
	}
	return nullptr;
}

const NNComponentTypeDesc* NNComponentRegistry::FindDesc(const std::type_index& typeIndex) const noexcept
{
	return FindDesc(FindTypeId(typeIndex));
}

const NNComponentTypeDesc* NNComponentRegistry::FindDescByNameHash(const std::uint64_t nameHash) const noexcept
{


	const auto it = m_NameHashToDesc.find(nameHash);
	if (it == m_NameHashToDesc.end())
	{
		return nullptr;
	}
	return &m_Descriptors[it->second];
}

NNComponentTypeDesc* NNComponentRegistry::FindDescByNameHash(const std::uint64_t nameHash) noexcept
{
	const auto it = m_NameHashToDesc.find(nameHash);
	if (it == m_NameHashToDesc.end())
	{
		return nullptr;
	}
	return &m_Descriptors[it->second];
}

const NNComponentFieldDesc* NNComponentRegistry::GetFieldByName(
	const NNComponentTypeId typeId,
	const char* const fieldNameUtf8) const noexcept
{
	const NNComponentTypeDesc* desc = FindDesc(typeId);
	if (desc == nullptr || fieldNameUtf8 == nullptr)
	{
		return nullptr;
	}
	for (const NNComponentFieldDesc& field : desc->Fields)
	{
		if (field.NameUtf8 != nullptr && std::strcmp(field.NameUtf8, fieldNameUtf8) == 0)
		{
			return &field;
		}
	}
	return nullptr;
}

void NNComponentRegistry::ForEachField(
	const NNComponentTypeId typeId,
	const std::function<void(const NNComponentFieldDesc&)>& visitor) const
{
	const NNComponentTypeDesc* desc = FindDesc(typeId);
	if (desc == nullptr || !visitor)
	{
		return;
	}
	for (const NNComponentFieldDesc& field : desc->Fields)
	{
		visitor(field);
	}
}
} // namespace NN::Runtime::Scene
