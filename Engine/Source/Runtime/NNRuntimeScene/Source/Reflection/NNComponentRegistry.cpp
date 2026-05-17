/**
 * @file NNComponentRegistry.cpp
 * @brief 组件类型注册表实现（含字段元数据）。
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
		(void)destination.RegisterTypeWithFields(
			desc.TypeIndex,
			desc.NameUtf8,
			desc.SizeBytes,
			desc.Fields);
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
	if (const auto found = m_TypeToId.find(typeIndex); found != m_TypeToId.end())
	{
		return found->second;
	}

	const NNComponentTypeId typeId = m_NextTypeId++;
	NNComponentTypeDesc desc{};
	desc.TypeId = typeId;
	desc.TypeIndex = typeIndex;
	desc.NameUtf8 = nameUtf8;
	desc.SizeBytes = sizeBytes;
	desc.Fields = fields;

	m_TypeToId.emplace(typeIndex, typeId);
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
	if (typeId == 0u || typeId > m_Descriptors.size())
	{
		return nullptr;
	}
	return &m_Descriptors[static_cast<std::size_t>(typeId - 1u)];
}

const NNComponentTypeDesc* NNComponentRegistry::FindDesc(const std::type_index& typeIndex) const noexcept
{
	return FindDesc(FindTypeId(typeIndex));
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
