/**
 * @file NNReflectionSnapshotBuilder.cpp
 * @brief 反射类型信息快照构建器实现——两遍扫描，零硬编码组件类型。
 */

#include "Snapshot/NNReflectionSnapshotBuilder.h"

#include <cstring>
#include <vector>

#include "NNNativeEngineAPI/Include/EditorSceneAPI.h"
#include "Reflection/NNComponentRegistry.h"

namespace NN::Runtime::Scene
{

namespace
{
	/** @brief 单次遍历收集统计信息。 */
	struct TypeInfoStats
	{
		std::uint32_t typeCount       = 0u;
		std::uint32_t totalFieldCount = 0u;
		std::uint32_t namePoolBytes   = 0u;
	};

	TypeInfoStats GatherStats(const NNComponentRegistry& registry)
	{
		TypeInfoStats stats{};

		registry.ForEachDescriptor([&](const NNComponentTypeDesc& desc)
		{
			if (desc.Fields.empty())
			{
				return; // 无字段的类型不输出（无法被 Inspector 展示）
			}

			++stats.typeCount;

			// 组件名称
			if (desc.NameUtf8 != nullptr)
			{
				stats.namePoolBytes += static_cast<std::uint32_t>(std::strlen(desc.NameUtf8)) + 1u;
			}
			else
			{
				stats.namePoolBytes += 1u;
			}

			// 字段名称
			for (const NNComponentFieldDesc& field : desc.Fields)
			{
				++stats.totalFieldCount;

				if (field.NameUtf8 != nullptr)
				{
					stats.namePoolBytes += static_cast<std::uint32_t>(std::strlen(field.NameUtf8)) + 1u;
				}
				else
				{
					stats.namePoolBytes += 1u;
				}
			}
		});

		return stats;
	}

	/** @brief 将 UTF-8 字符串写入 namePool 并推进 nameOffset。返回写入的字节数。 */
	std::uint32_t WriteNameToPool(
		char*               namePool,
		std::uint32_t&      nameOffset,
		const char*         name)
	{
		const std::uint32_t len = (name != nullptr)
			? static_cast<std::uint32_t>(std::strlen(name))
			: 0u;

		if (len > 0u)
		{
			std::memcpy(namePool + nameOffset, name, len);
		}
		namePool[nameOffset + len] = '\0';
		nameOffset += len + 1u;
		return len;
	}

} // anonymous namespace

std::uint32_t NNReflectionSnapshotBuilder::EstimateSize(const NNRuntimeScene& scene)
{
	const auto& registry = scene.GetComponentRegistry();
	const TypeInfoStats stats = GatherStats(registry);

	return static_cast<std::uint32_t>(sizeof(NNSceneSnapshotHeader))
	     + stats.typeCount * static_cast<std::uint32_t>(sizeof(NNEditorComponentInfo))
	     + stats.totalFieldCount * static_cast<std::uint32_t>(sizeof(NNEditorFieldInfo))
	     + stats.namePoolBytes;
}

std::uint32_t NNReflectionSnapshotBuilder::Build(
	const NNRuntimeScene& scene,
	void*                 outBuffer,
	std::uint32_t         capacity)
{
	const auto& registry = scene.GetComponentRegistry();

	// ── 第一遍：统计 ──

	const TypeInfoStats stats = GatherStats(registry);

	const std::uint32_t needed = static_cast<std::uint32_t>(sizeof(NNSceneSnapshotHeader))
	                           + stats.typeCount * static_cast<std::uint32_t>(sizeof(NNEditorComponentInfo))
	                           + stats.totalFieldCount * static_cast<std::uint32_t>(sizeof(NNEditorFieldInfo))
	                           + stats.namePoolBytes;

	if (capacity < needed || outBuffer == nullptr)
	{
		return needed; // 返回所需大小，调用方可用于 getTypeInfoSnapshotSize
	}

	// ── 写 Header ──

	auto* header = static_cast<NNSceneSnapshotHeader*>(outBuffer);
	header->magic            = 0x56475343u; // 'VGSC'
	header->layoutVersion    = 2u;          // 类型信息快照格式版本（区别于 hierarchy 的 1）
	header->hierarchyVersion = scene.ReflectionVersion();
	header->nodeCount        = stats.typeCount;
	header->namePoolBytes    = stats.namePoolBytes;
	header->rootCount        = 0u;
	header->_pad             = 0u;

	// ── 写 ComponentInfo[] + FieldInfo[] + NamePool ──

	auto* compInfos = reinterpret_cast<NNEditorComponentInfo*>(header + 1);
	auto* fieldInfos = reinterpret_cast<NNEditorFieldInfo*>(compInfos + stats.typeCount);
	char* namePool   = reinterpret_cast<char*>(fieldInfos + stats.totalFieldCount);

	std::uint32_t compIdx    = 0u;
	std::uint32_t fieldIdx   = 0u;
	std::uint32_t nameOffset = 0u;

	registry.ForEachDescriptor([&](const NNComponentTypeDesc& desc)
	{
		if (desc.Fields.empty())
		{
			return;
		}

		// 写入组件名称到 namePool
		const std::uint32_t compNameLen = WriteNameToPool(namePool, nameOffset, desc.NameUtf8);

		// 写入 ComponentInfo
		auto& ci = compInfos[compIdx++];
		ci.typeId      = desc.NameHash;
		ci.nameOffset  = nameOffset - compNameLen - 1u; // 回退到刚写入的位置
		ci.nameLen     = compNameLen;
		ci.fieldCount  = static_cast<std::uint32_t>(desc.Fields.size());
		ci.flags       = 0u;

		// 写入 FieldInfo[]
		for (const NNComponentFieldDesc& field : desc.Fields)
		{
			const std::uint32_t fieldNameLen = WriteNameToPool(namePool, nameOffset, field.NameUtf8);

			auto& fi = fieldInfos[fieldIdx++];
			fi.nameOffset  = nameOffset - fieldNameLen - 1u;
			fi.nameLen     = fieldNameLen;
			fi.fieldType   = static_cast<std::uint32_t>(field.FieldType);
			fi.dataOffset  = field.Offset;
			fi.dataSize    = field.Size;
			fi._pad        = 0u;
		}
	});

	return needed;
}

} // namespace NN::Runtime::Scene
