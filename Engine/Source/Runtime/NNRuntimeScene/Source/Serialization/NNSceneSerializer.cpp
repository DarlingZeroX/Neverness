/**
 * @file NNSceneSerializer.cpp
 * @brief **NNSceneSerializer** 二进制快照实现（Phase 4-B：FNV-1a 稳定 TypeId）。
 */

#include "Serialization/NNSceneSerializer.h"

#include <cstring>

#include "../../../NNNativeEngineAPI/Include/EngineTypes.h"
#include "Reflection/NNComponentRegistry.h"
#include "Scene/NNRuntimeScene.h"

namespace NN::Runtime::Scene
{
namespace
{
constexpr char kMagic[4] = {'V', 'G', 'S', 'C'};
constexpr std::uint32_t kInvalidArchiveIndex = 0xFFFFFFFFu;

void WriteU32(std::vector<std::uint8_t>& buffer, const std::uint32_t value)
{
	buffer.push_back(static_cast<std::uint8_t>(value & 0xFFu));
	buffer.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xFFu));
	buffer.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xFFu));
	buffer.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xFFu));
}

bool ReadU32(const std::vector<std::uint8_t>& buffer, std::size_t& offset, std::uint32_t& out)
{
	if (offset + 4u > buffer.size())
	{
		return false;
	}
	out = static_cast<std::uint32_t>(buffer[offset])
		| (static_cast<std::uint32_t>(buffer[offset + 1u]) << 8u)
		| (static_cast<std::uint32_t>(buffer[offset + 2u]) << 16u)
		| (static_cast<std::uint32_t>(buffer[offset + 3u]) << 24u);
	offset += 4u;
	return true;
}

/** @brief 写入 64 位值（低 32 位在前，小端序）。 */
void WriteU64(std::vector<std::uint8_t>& buffer, const std::uint64_t value)
{
	WriteU32(buffer, static_cast<std::uint32_t>(value & 0xFFFFFFFFu));
	WriteU32(buffer, static_cast<std::uint32_t>((value >> 32u) & 0xFFFFFFFFu));
}

/** @brief 读取 64 位值（低 32 位在前，小端序）。 */
bool ReadU64(const std::vector<std::uint8_t>& buffer, std::size_t& offset, std::uint64_t& out)
{
	std::uint32_t lo = 0u;
	std::uint32_t hi = 0u;
	if (!ReadU32(buffer, offset, lo) || !ReadU32(buffer, offset, hi))
	{
		return false;
	}
	out = static_cast<std::uint64_t>(lo) | (static_cast<std::uint64_t>(hi) << 32u);
	return true;
}

void WriteBytes(std::vector<std::uint8_t>& buffer, const void* data, const std::size_t size)
{
	const auto* bytes = static_cast<const std::uint8_t*>(data);
	buffer.insert(buffer.end(), bytes, bytes + size);
}

bool ReadBytes(
	const std::vector<std::uint8_t>& buffer,
	std::size_t& offset,
	void* data,
	const std::size_t size)
{
	if (offset + size > buffer.size())
	{
		return false;
	}
	std::memcpy(data, buffer.data() + offset, size);
	offset += size;
	return true;
}

std::uint32_t EntityToArchiveIndex(
	const std::unordered_map<NNEntity, std::uint32_t>& handleToArchive,
	const NNEntity entity)
{
	if (entity == NNEntityInvalid)
	{
		return kInvalidArchiveIndex;
	}
	const auto it = handleToArchive.find(entity);
	return it == handleToArchive.end() ? kInvalidArchiveIndex : it->second;
}

std::vector<std::uint8_t> SerializeComponentBlob(
	const NNComponentTypeDesc& desc,
	const void* componentData,
	const std::unordered_map<NNEntity, std::uint32_t>& handleToArchive)
{
	std::vector<std::uint8_t> blob;
	for (const NNComponentFieldDesc& field : desc.Fields)
	{
		const auto* base = static_cast<const std::uint8_t*>(componentData);
		const void* fieldPtr = base + field.Offset;

		switch (field.FieldType)
		{
		case NNComponentFieldType::Float:
		{
			float value = 0.f;
			std::memcpy(&value, fieldPtr, sizeof(float));
			WriteBytes(blob, &value, sizeof(float));
			break;
		}
		case NNComponentFieldType::Float3:
			WriteBytes(blob, fieldPtr, sizeof(float) * 3u);
			break;
		case NNComponentFieldType::Float4:
		case NNComponentFieldType::Quaternion:
			WriteBytes(blob, fieldPtr, sizeof(float) * 4u);
			break;
		case NNComponentFieldType::Float4x4:
			WriteBytes(blob, fieldPtr, sizeof(float) * 16u);
			break;
		case NNComponentFieldType::UInt32:
		{
			std::uint32_t value = 0u;
			std::memcpy(&value, fieldPtr, sizeof(std::uint32_t));
			WriteU32(blob, value);
			break;
		}
		case NNComponentFieldType::UInt64:
		{
			std::uint64_t value = 0u;
			std::memcpy(&value, fieldPtr, sizeof(std::uint64_t));
			WriteU64(blob, value);
			break;
		}
		case NNComponentFieldType::Entity:
		{
			NNEntity entityValue = NNEntityInvalid;
			std::memcpy(&entityValue, fieldPtr, sizeof(NNEntity));
			WriteU32(blob, EntityToArchiveIndex(handleToArchive, entityValue));
			break;
		}
		case NNComponentFieldType::CharArray:
			/* 固定大小字符数组，按字段 Size 字节直接 memcpy */
			WriteBytes(blob, fieldPtr, field.Size);
			break;
		case NNComponentFieldType::Guid:
			{
				NNGuid guid{};
				std::memcpy(&guid, fieldPtr, sizeof(NNGuid));
				WriteU64(blob, guid.high);
				WriteU64(blob, guid.low);
				break;
			}
		case NNComponentFieldType::Bool:
		{
			std::uint8_t value = 0u;
			std::memcpy(&value, fieldPtr, sizeof(std::uint8_t));
			WriteBytes(blob, &value, sizeof(std::uint8_t));
			break;
		}
		default:
			break;
		}
	}
	return blob;
}

bool DeserializeComponentBlob(
	const NNComponentTypeDesc& desc,
	void* componentData,
	const std::vector<std::uint8_t>& blob,
	std::size_t& blobOffset,
	const NNSceneEntityArchiveMap& archiveToHandle,
	std::uint32_t archiveVersion)
{
	for (const NNComponentFieldDesc& field : desc.Fields)
	{
		auto* base = static_cast<std::uint8_t*>(componentData);
		void* fieldPtr = base + field.Offset;

		switch (field.FieldType)
		{
		case NNComponentFieldType::Float:
		{
			float value = 0.f;
			if (!ReadBytes(blob, blobOffset, &value, sizeof(float)))
			{
				return false;
			}
			std::memcpy(fieldPtr, &value, sizeof(float));
			break;
		}
		case NNComponentFieldType::Float3:
			if (!ReadBytes(blob, blobOffset, fieldPtr, sizeof(float) * 3u))
			{
				return false;
			}
			break;
		case NNComponentFieldType::Float4:
		case NNComponentFieldType::Quaternion:
			if (!ReadBytes(blob, blobOffset, fieldPtr, sizeof(float) * 4u))
			{
				return false;
			}
			break;
		case NNComponentFieldType::Float4x4:
			if (!ReadBytes(blob, blobOffset, fieldPtr, sizeof(float) * 16u))
			{
				return false;
			}
			break;
		case NNComponentFieldType::UInt32:
		{
			std::uint32_t value = 0u;
			if (!ReadU32(blob, blobOffset, value))
			{
				return false;
			}
			std::memcpy(fieldPtr, &value, sizeof(std::uint32_t));
			break;
		}
		case NNComponentFieldType::UInt64:
		{
			std::uint64_t value = 0u;
			if (!ReadU64(blob, blobOffset, value))
			{
				return false;
			}
			std::memcpy(fieldPtr, &value, sizeof(std::uint64_t));
			break;
		}
		case NNComponentFieldType::Entity:
		{
			std::uint32_t archiveIndex = kInvalidArchiveIndex;
			if (!ReadU32(blob, blobOffset, archiveIndex))
			{
				return false;
			}
			NNEntity handle = NNEntityInvalid;
			if (archiveIndex != kInvalidArchiveIndex)
			{
				const auto it = archiveToHandle.find(archiveIndex);
				if (it != archiveToHandle.end())
				{
					handle = it->second;
				}
			}
			std::memcpy(fieldPtr, &handle, sizeof(NNEntity));
			break;
		}
		case NNComponentFieldType::CharArray:
			/* 固定大小字符数组，按字段 Size 字节直接读取 */
			if (!ReadBytes(blob, blobOffset, fieldPtr, field.Size))
			{
				return false;
			}
			break;
		case NNComponentFieldType::Guid:
			{
				NNGuid guid{};
				if (archiveVersion <= 2u)
				{
					// v2 旧格式：Guid 字段实际以 UInt64/Entity 共用路径序列化（仅 4 字节）
					// 因 UInt64/Entity bug，原 asset 值已被 entity archive index 重映射
					// 降级读取：读 U32，构造 LEGA 标记 GUID
					std::uint32_t rawValue = 0u;
					if (!ReadU32(blob, blobOffset, rawValue))
						return false;
					guid.high = 0x4C45474100000000ULL;  // 'LEGA' 标记
					guid.low = static_cast<std::uint64_t>(rawValue);
				}
				else
				{
					// v3+：标准 Guid 路径（high + low，各 8 字节）
					if (!ReadU64(blob, blobOffset, guid.high) || !ReadU64(blob, blobOffset, guid.low))
						return false;
				}
				std::memcpy(fieldPtr, &guid, sizeof(NNGuid));
				break;
			}
		case NNComponentFieldType::Bool:
		{
			std::uint8_t value = 0u;
			if (!ReadBytes(blob, blobOffset, &value, sizeof(std::uint8_t)))
			{
				return false;
			}
			std::memcpy(fieldPtr, &value, sizeof(std::uint8_t));
			break;
		}
		default:
			break;
		}
	}
	return true;
}

} // namespace

std::vector<std::uint8_t> NNSceneSerializer::Serialize(const NNRuntimeScene& scene)
{
	std::vector<NNEntity> aliveEntities;
	scene.ForEachAliveEntity([&](const NNEntity handle) { aliveEntities.push_back(handle); });

	std::unordered_map<NNEntity, std::uint32_t> handleToArchive;
	for (std::uint32_t i = 0; i < aliveEntities.size(); ++i)
	{
		handleToArchive[aliveEntities[i]] = i;
	}

	std::vector<std::uint8_t> buffer;
	buffer.insert(buffer.end(), kMagic, kMagic + 4);
	WriteU32(buffer, kFormatVersion);
	WriteU32(buffer, static_cast<std::uint32_t>(aliveEntities.size()));

	const NNComponentRegistry& registry = scene.GetComponentRegistry();

	for (const NNEntity handle : aliveEntities)
	{
		std::vector<std::uint8_t> componentsPayload;
		std::uint32_t componentCount = 0u;

		/* 统一 registry-driven 序列化：遍历所有已注册组件类型 */
		registry.ForEachDescriptor([&](const NNComponentTypeDesc& desc)
		{
			if (!desc.GetComponentConstPtrFn || desc.Fields.empty())
			{
				return;
			}

			const void* ptr = desc.GetComponentConstPtrFn(&scene, handle);
			if (ptr == nullptr)
			{
				return;
			}

			const std::vector<std::uint8_t> blob =
				SerializeComponentBlob(desc, ptr, handleToArchive);
			WriteU64(componentsPayload, desc.NameHash);
			WriteU32(componentsPayload, static_cast<std::uint32_t>(blob.size()));
			WriteBytes(componentsPayload, blob.data(), blob.size());
			++componentCount;
		});

		WriteU32(buffer, componentCount);
		buffer.insert(buffer.end(), componentsPayload.begin(), componentsPayload.end());
	}

	return buffer;
}

bool NNSceneSerializer::Deserialize(
	NNRuntimeScene& scene,
	const std::vector<std::uint8_t>& buffer,
	NNSceneEntityArchiveMap* outArchiveMap)
{
	if (buffer.size() < 12u || std::memcmp(buffer.data(), kMagic, 4) != 0)
	{
		return false;
	}

	std::size_t offset = 4u;
	std::uint32_t version = 0u;
	std::uint32_t entityCount = 0u;
	if (!ReadU32(buffer, offset, version) || (version != 2u && version != 3u))
	{
		return false;
	}
	if (!ReadU32(buffer, offset, entityCount))
	{
		return false;
	}

	NNSceneEntityArchiveMap archiveToHandle;
	archiveToHandle.reserve(entityCount);

	/* 待反序列化的组件（按 nameHash 匹配） */
	struct PendingComponent
	{
		std::uint64_t NameHash = 0u;  /* FNV-1a 稳定哈希 */
		std::vector<std::uint8_t> Blob{};
	};
	std::vector<std::vector<PendingComponent>> pendingPerEntity;

	pendingPerEntity.resize(entityCount);
	for (std::uint32_t e = 0; e < entityCount; ++e)
	{
		std::uint32_t componentCount = 0u;
		if (!ReadU32(buffer, offset, componentCount))
		{
			return false;
		}
		auto& pending = pendingPerEntity[e];
		pending.reserve(componentCount);
		for (std::uint32_t c = 0; c < componentCount; ++c)
		{
			PendingComponent pc{};
			if (!ReadU64(buffer, offset, pc.NameHash))  /* 读取 8 字节 nameHash */
			{
				return false;
			}
			std::uint32_t blobSize = 0u;
			if (!ReadU32(buffer, offset, blobSize) || offset + blobSize > buffer.size())
			{
				return false;
			}
			pc.Blob.assign(buffer.begin() + static_cast<std::ptrdiff_t>(offset),
				buffer.begin() + static_cast<std::ptrdiff_t>(offset + blobSize));
			offset += blobSize;
			pending.push_back(std::move(pc));
		}

		const NNEntity handle = scene.CreateEntityWithDefaults();
		archiveToHandle[e] = handle;
	}

	const NNComponentRegistry& registry = scene.GetComponentRegistry();

	for (std::uint32_t e = 0; e < entityCount; ++e)
	{
		const NNEntity handle = archiveToHandle[e];
		for (const PendingComponent& pc : pendingPerEntity[e])
		{
			/* 按 nameHash 查找描述符（而非自增 typeId） */
			const NNComponentTypeDesc* desc = registry.FindDescByNameHash(pc.NameHash);
			if (desc == nullptr || desc->Fields.empty())
			{
				continue;
			}

			if (desc->GetComponentPtrFn == nullptr)
			{
				continue;
			}

			/* 若实体尚无此组件（如 Camera 不在 CreateEntityWithDefaults 中），先添加 */
			if (desc->HasComponentFn != nullptr && desc->AddComponentFn != nullptr
			    && !desc->HasComponentFn(&scene, handle))
			{
				desc->AddComponentFn(&scene, handle);
			}

			/* 通用反序列化：反序列化到临时缓冲区 → 写入 ECS */
			std::vector<std::uint8_t> temp(desc->SizeBytes);
			std::size_t blobOffset = 0u;
			if (DeserializeComponentBlob(*desc, temp.data(), pc.Blob, blobOffset, archiveToHandle, version))
			{
				void* dst = desc->GetComponentPtrFn(&scene, handle);
				if (dst != nullptr)
				{
					std::memcpy(dst, temp.data(), desc->SizeBytes);
				}
				/* 可选的后处理回调（如 Relationship 的 SetParent） */
				if (desc->PostDeserializeFn != nullptr)
				{
					desc->PostDeserializeFn(scene, handle);
				}
			}
		}
	}

	if (outArchiveMap != nullptr)
	{
		*outArchiveMap = std::move(archiveToHandle);
	}
	return true;
}
} // namespace NN::Runtime::Scene
