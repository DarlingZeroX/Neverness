/**
 * @file NNPrefab.cpp
 * @brief NNPrefab 实例化实现（Phase 7）。
 *
 * 解析 C# PrefabImporter.SerializePrefab() 生成的二进制 blob，
 * 递归创建实体 + 组件 + 父子层级。
 */

#include "Prefab/NNPrefab.h"

#include <cstring>
#include <string>

#include "EngineTypes.h"
#include "Components/NNTagComponent.h"
#include "Components/NNTransformComponent.h"
#include "Reflection/NNComponentRegistry.h"
#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
namespace
{
/* ======================== 二进制读取辅助（与 C# BinaryWriter 小端序对齐） ======================== */

bool ReadU32(const std::uint8_t*& ptr, const std::uint8_t* end, std::uint32_t& out)
{
	if (ptr + 4 > end) return false;
	out = static_cast<std::uint32_t>(ptr[0])
		| (static_cast<std::uint32_t>(ptr[1]) << 8u)
		| (static_cast<std::uint32_t>(ptr[2]) << 16u)
		| (static_cast<std::uint32_t>(ptr[3]) << 24u);
	ptr += 4;
	return true;
}

bool ReadI32(const std::uint8_t*& ptr, const std::uint8_t* end, std::int32_t& out)
{
	std::uint32_t val = 0u;
	if (!ReadU32(ptr, end, val)) return false;
	out = static_cast<std::int32_t>(val);
	return true;
}

bool ReadU64(const std::uint8_t*& ptr, const std::uint8_t* end, std::uint64_t& out)
{
	std::uint32_t lo = 0u;
	std::uint32_t hi = 0u;
	if (!ReadU32(ptr, end, lo) || !ReadU32(ptr, end, hi)) return false;
	out = static_cast<std::uint64_t>(lo) | (static_cast<std::uint64_t>(hi) << 32u);
	return true;
}

bool ReadString(const std::uint8_t*& ptr, const std::uint8_t* end, std::string& out)
{
	std::int32_t len = 0;
	if (!ReadI32(ptr, end, len) || len < 0) return false;
	if (ptr + len > end) return false;
	out.assign(reinterpret_cast<const char*>(ptr), static_cast<std::size_t>(len));
	ptr += len;
	return true;
}

bool ReadBytes(const std::uint8_t*& ptr, const std::uint8_t* end, void* dst, std::size_t size)
{
	if (ptr + size > end) return false;
	std::memcpy(dst, ptr, size);
	ptr += size;
	return true;
}

/* ======================== 递归实例化 ======================== */

struct DeserializedEntity
{
	NNGuid Guid{};
	std::string Name{};
	struct ComponentData
	{
		std::string TypeName;
		std::vector<std::uint8_t> JsonBytes;
	};
	std::vector<ComponentData> Components{};
	std::vector<DeserializedEntity> Children{};
};

bool ReadEntity(const std::uint8_t*& ptr, const std::uint8_t* end, DeserializedEntity& out)
{
	/* guid */
	if (!ReadU64(ptr, end, out.Guid.high) || !ReadU64(ptr, end, out.Guid.low))
		return false;

	/* name */
	if (!ReadString(ptr, end, out.Name))
		return false;

	/* components */
	std::int32_t componentCount = 0;
	if (!ReadI32(ptr, end, componentCount) || componentCount < 0)
		return false;

	out.Components.resize(static_cast<std::size_t>(componentCount));
	for (std::int32_t c = 0; c < componentCount; ++c)
	{
		auto& comp = out.Components[c];

		/* type name */
		if (!ReadString(ptr, end, comp.TypeName))
			return false;

		/* json bytes */
		std::int32_t jsonLen = 0;
		if (!ReadI32(ptr, end, jsonLen) || jsonLen < 0)
			return false;
		if (ptr + jsonLen > end)
			return false;
		comp.JsonBytes.assign(ptr, ptr + jsonLen);
		ptr += jsonLen;
	}

	/* children */
	std::int32_t childCount = 0;
	if (!ReadI32(ptr, end, childCount) || childCount < 0)
		return false;

	out.Children.resize(static_cast<std::size_t>(childCount));
	for (std::int32_t i = 0; i < childCount; ++i)
	{
		if (!ReadEntity(ptr, end, out.Children[i]))
			return false;
	}

	return true;
}

NNEntity InstantiateEntity(
	NNRuntimeScene& scene,
	const DeserializedEntity& data,
	NNEntity parent)
{
	/* 创建实体 */
	NNEntity entity = scene.CreateEntityWithDefaults();

	/* 设置名称 */
	if (NNTagComponent* tag = scene.TryGet<NNTagComponent>(entity))
	{
		const std::size_t copyLen = data.Name.size() < sizeof(tag->Name) - 1
			? data.Name.size()
			: sizeof(tag->Name) - 1;
		std::memcpy(tag->Name, data.Name.c_str(), copyLen);
		tag->Name[copyLen] = '\0';
	}

	/* 设置父子关系 */
	if (parent != NNEntityInvalid)
	{
		scene.SetParent(entity, parent);
	}

	/* TODO: 根据组件类型名称注册组件数据 */
	/* 当前阶段：仅创建实体层级，组件数据需要具体类型的注册回调 */
	/* 例如 Transform 组件可从 JSON 解析 Position/Rotation/Scale */

	/* 递归子实体 */
	for (const auto& childData : data.Children)
	{
		InstantiateEntity(scene, childData, entity);
	}

	return entity;
}

} // namespace

bool NNPrefab::Instantiate(
	NNRuntimeScene& scene,
	const void* blobData,
	std::size_t blobSize,
	NNEntity& outRootEntity)
{
	if (blobData == nullptr || blobSize < 8)
		return false;

	const auto* ptr = static_cast<const std::uint8_t*>(blobData);
	const auto* end = ptr + blobSize;

	/* prefab name */
	std::string prefabName;
	if (!ReadString(ptr, end, prefabName))
		return false;

	/* entity count */
	std::int32_t entityCount = 0;
	if (!ReadI32(ptr, end, entityCount) || entityCount <= 0)
		return false;

	/* 读取根实体 */
	DeserializedEntity rootData;
	if (!ReadEntity(ptr, end, rootData))
		return false;

	/* 实例化到场景 */
	outRootEntity = InstantiateEntity(scene, rootData, NNEntityInvalid);
	return outRootEntity != NNEntityInvalid;
}

bool NNPrefab::Instantiate(
	NNRuntimeScene& scene,
	const std::vector<std::uint8_t>& blob,
	NNEntity& outRootEntity)
{
	return Instantiate(scene, blob.data(), blob.size(), outRootEntity);
}

} // namespace NN::Runtime::Scene
