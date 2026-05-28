/**
 * @file NNJsonSceneSerializer.cpp
 * @brief Editor JSON 场景序列化器实现——完全 registry-driven，零硬编码组件类型。
 */

#include "Serialization/NNJsonSceneSerializer.h"

#include <charconv>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "../../../NNNativeEngineAPI/Include/EngineTypes.h"
#include <NNCore/Include/File/NlohmannJson.h>
#include "Reflection/NNComponentRegistry.h"
#include "Scene/NNRuntimeScene.h"

using json = nlohmann::json;

namespace NN::Runtime::Scene
{
namespace
{
// ── Entity 格式化 ──

/** @brief NNEntity (uint64) → "0xHHHHHHHHHHHHHHHH" 十六进制字符串。 */
std::string FormatEntityHex(const NNEntity entity)
{
	char buf[20];
	auto [ptr, ec] = std::to_chars(buf + 2, buf + sizeof(buf), entity, 16);
	buf[0] = '0';
	buf[1] = 'x';
	return std::string(buf, ptr);
}

/** @brief "0xHHHHHHHHHHHHHHHH" → NNEntity，失败返回 NNEntityInvalid。 */
NNEntity ParseEntityHex(const std::string& str)
{
	if (str.size() < 3 || str[0] != '0' || str[1] != 'x')
	{
		return NNEntityInvalid;
	}
	NNEntity value = 0;
	auto [ptr, ec] = std::from_chars(str.data() + 2, str.data() + str.size(), value, 16);
	if (ec != std::errc{} || ptr != str.data() + str.size())
	{
		return NNEntityInvalid;
	}
	return value;
}

// ── Field → JSON ──

/** @brief 单个字段 → JSON 值。 */
json FieldToJson(
	const void* componentBase,
	const NNComponentFieldDesc& field,
	const std::unordered_map<NNEntity, std::uint32_t>& handleToArchive)
{
	const auto* base = static_cast<const std::uint8_t*>(componentBase);
	const void* fieldPtr = base + field.Offset;

	switch (field.FieldType)
	{
	case NNComponentFieldType::Float:
	{
		float value = 0.f;
		std::memcpy(&value, fieldPtr, sizeof(float));
		return value;
	}
	case NNComponentFieldType::Float3:
	{
		const auto* f = static_cast<const float*>(fieldPtr);
		return json{f[0], f[1], f[2]};
	}
	case NNComponentFieldType::Float4:
	case NNComponentFieldType::Quaternion:
	{
		const auto* f = static_cast<const float*>(fieldPtr);
		return json{f[0], f[1], f[2], f[3]};
	}
	case NNComponentFieldType::Float4x4:
	{
		const auto* m = static_cast<const float*>(fieldPtr);
		json rows = json::array();
		for (int r = 0; r < 4; ++r)
		{
			rows.push_back({m[r * 4 + 0], m[r * 4 + 1], m[r * 4 + 2], m[r * 4 + 3]});
		}
		return rows;
	}
	case NNComponentFieldType::UInt32:
	{
		std::uint32_t value = 0u;
		std::memcpy(&value, fieldPtr, sizeof(std::uint32_t));
		return value;
	}
	case NNComponentFieldType::UInt64:
	{
		std::uint64_t value = 0u;
		std::memcpy(&value, fieldPtr, sizeof(std::uint64_t));
		return value;
	}
	case NNComponentFieldType::Entity:
	{
		NNEntity entityValue = NNEntityInvalid;
		std::memcpy(&entityValue, fieldPtr, sizeof(NNEntity));
		if (entityValue == NNEntityInvalid)
		{
			return nullptr;
		}
		const auto it = handleToArchive.find(entityValue);
		if (it != handleToArchive.end())
		{
			return FormatEntityHex(entityValue);
		}
		return FormatEntityHex(entityValue);
	}
	case NNComponentFieldType::CharArray:
	{
		const auto* str = reinterpret_cast<const char*>(fieldPtr);
		const std::size_t maxLen = field.Size;
		const std::size_t len = strnlen(str, maxLen);
		return std::string(str, len);
	}
	case NNComponentFieldType::Guid:
	{
		NNGuid guid{};
		std::memcpy(&guid, fieldPtr, sizeof(NNGuid));
		// 输出 "high:low" hex 格式，固定 16 位零填充
		std::ostringstream oss;
		oss << std::hex << std::setfill('0') << std::setw(16) << guid.high << ":" << std::setw(16) << guid.low;
		return oss.str();
	}
	default:
		return nullptr;
	}
}

// ── JSON → Field ──

/** @brief JSON 值 → 单个字段。 */
void JsonToField(
	const json& value,
	void* componentBase,
	const NNComponentFieldDesc& field,
	const std::unordered_map<std::uint32_t, NNEntity>& archiveToHandle)
{
	auto* base = static_cast<std::uint8_t*>(componentBase);
	void* fieldPtr = base + field.Offset;

	switch (field.FieldType)
	{
	case NNComponentFieldType::Float:
	{
		if (value.is_number())
		{
			float f = value.get<float>();
			std::memcpy(fieldPtr, &f, sizeof(float));
		}
		break;
	}
	case NNComponentFieldType::Float3:
	{
		if (value.is_array() && value.size() == 3)
		{
			float f[3];
			for (int i = 0; i < 3; ++i) f[i] = value[i].get<float>();
			std::memcpy(fieldPtr, f, sizeof(f));
		}
		break;
	}
	case NNComponentFieldType::Float4:
	case NNComponentFieldType::Quaternion:
	{
		if (value.is_array() && value.size() == 4)
		{
			float f[4];
			for (int i = 0; i < 4; ++i) f[i] = value[i].get<float>();
			std::memcpy(fieldPtr, f, sizeof(f));
		}
		break;
	}
	case NNComponentFieldType::Float4x4:
	{
		if (value.is_array() && value.size() == 4)
		{
			float m[16];
			for (int r = 0; r < 4; ++r)
			{
				if (value[r].is_array() && value[r].size() == 4)
				{
					for (int c = 0; c < 4; ++c)
						m[r * 4 + c] = value[r][c].get<float>();
				}
			}
			std::memcpy(fieldPtr, m, sizeof(m));
		}
		break;
	}
	case NNComponentFieldType::UInt32:
	{
		if (value.is_number_unsigned())
		{
			std::uint32_t v = value.get<std::uint32_t>();
			std::memcpy(fieldPtr, &v, sizeof(std::uint32_t));
		}
		else if (value.is_number_integer())
		{
			auto v = static_cast<std::uint32_t>(value.get<std::int64_t>());
			std::memcpy(fieldPtr, &v, sizeof(std::uint32_t));
		}
		break;
	}
	case NNComponentFieldType::UInt64:
	{
		if (value.is_number_unsigned())
		{
			std::uint64_t v = value.get<std::uint64_t>();
			std::memcpy(fieldPtr, &v, sizeof(std::uint64_t));
		}
		else if (value.is_number_integer())
		{
			auto v = static_cast<std::uint64_t>(value.get<std::int64_t>());
			std::memcpy(fieldPtr, &v, sizeof(std::uint64_t));
		}
		break;
	}
	case NNComponentFieldType::Entity:
	{
		NNEntity handle = NNEntityInvalid;
		if (value.is_string())
		{
			handle = ParseEntityHex(value.get<std::string>());
		}
		// null 或无效字符串保持 NNEntityInvalid
		std::memcpy(fieldPtr, &handle, sizeof(NNEntity));
		break;
	}
	case NNComponentFieldType::CharArray:
	{
		if (value.is_string())
		{
			std::string str = value.get<std::string>();
			std::size_t copyLen = std::min(str.size(), static_cast<std::size_t>(field.Size - 1));
			std::memcpy(fieldPtr, str.data(), copyLen);
			// 确保以 '\0' 结尾
			static_cast<char*>(fieldPtr)[copyLen] = '\0';
		}
		break;
	}
	case NNComponentFieldType::Guid:
	{
		if (value.is_string())
		{
			std::string str = value.get<std::string>();
			// 剥离逗号等非十六进制字符（兼容旧版 locale 千位分隔符格式）
			std::string clean;
			clean.reserve(str.size());
			for (char c : str)
			{
				if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
				    (c >= 'A' && c <= 'F') || c == ':')
				{
					clean.push_back(c);
				}
			}
			auto colonPos = clean.find(':');
			if (colonPos != std::string::npos)
			{
				NNGuid guid{};
				std::from_chars(clean.data(), clean.data() + colonPos, guid.high, 16);
				std::from_chars(clean.data() + colonPos + 1, clean.data() + clean.size(), guid.low, 16);
				std::memcpy(fieldPtr, &guid, sizeof(NNGuid));
			}
		}
		break;
	}
	default:
		break;
	}
}

} // namespace

std::string NNJsonSceneSerializer::Serialize(const NNRuntimeScene& scene)
{
	// 收集存活实体
	std::vector<NNEntity> aliveEntities;
	scene.ForEachAliveEntity([&](const NNEntity handle) { aliveEntities.push_back(handle); });

	// 建立 handle → archiveIndex 映射
	std::unordered_map<NNEntity, std::uint32_t> handleToArchive;
	for (std::uint32_t i = 0; i < aliveEntities.size(); ++i)
	{
		handleToArchive[aliveEntities[i]] = i;
	}

	const NNComponentRegistry& registry = scene.GetComponentRegistry();

	json root;
	root["format"] = "NNScene";
	root["version"] = kFormatVersion;

	json entities = json::array();

	for (const NNEntity handle : aliveEntities)
	{
		json entityJson;
		entityJson["id"] = FormatEntityHex(handle);

		json components = json::object();

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

			json componentJson = json::object();
			for (const NNComponentFieldDesc& field : desc.Fields)
			{
				if (field.NameUtf8 != nullptr)
				{
					componentJson[field.NameUtf8] = FieldToJson(ptr, field, handleToArchive);
				}
			}

			components[desc.NameUtf8] = componentJson;
		});

		entityJson["components"] = components;
		entities.push_back(std::move(entityJson));
	}

	root["entities"] = entities;

	return root.dump(4);
}

bool NNJsonSceneSerializer::Deserialize(
	NNRuntimeScene& scene,
	const std::string& jsonStr)
{
	json root;
	try
	{
		root = json::parse(jsonStr);
	}
	catch (const json::parse_error&)
	{
		return false;
	}

	if (!root.contains("format") || root["format"] != "NNScene")
	{
		return false;
	}

	if (!root.contains("entities") || !root["entities"].is_array())
	{
		return false;
	}

	const json& entities = root["entities"];
	const std::uint32_t entityCount = static_cast<std::uint32_t>(entities.size());

	// 第一遍：创建实体，建立 index→handle 映射
	std::unordered_map<std::uint32_t, NNEntity> archiveToHandle;
	archiveToHandle.reserve(entityCount);

	for (std::uint32_t i = 0; i < entityCount; ++i)
	{
		const NNEntity handle = scene.CreateEntityWithDefaults();
		archiveToHandle[i] = handle;
	}

	// 第二遍：写入组件数据
	const NNComponentRegistry& registry = scene.GetComponentRegistry();

	for (std::uint32_t i = 0; i < entityCount; ++i)
	{
		const NNEntity handle = archiveToHandle[i];
		const json& entityJson = entities[i];

		if (!entityJson.contains("components") || !entityJson["components"].is_object())
		{
			continue;
		}

		const json& components = entityJson["components"];

		for (auto it = components.begin(); it != components.end(); ++it)
		{
			const std::string& name = it.key();
			const json& componentJson = it.value();

			const NNComponentTypeDesc* desc = registry.FindDescByNameHash(fnv1a_64(name.c_str()));
			if (desc == nullptr || desc->GetComponentPtrFn == nullptr || desc->Fields.empty())
			{
				continue;
			}

			// 若实体尚无此组件（如 Camera 不在 CreateEntityWithDefaults 中），先添加
			if (desc->HasComponentFn != nullptr && desc->AddComponentFn != nullptr
			    && !desc->HasComponentFn(&scene, handle))
			{
				desc->AddComponentFn(&scene, handle);
			}

			// 分配临时缓冲区
			std::vector<std::uint8_t> temp(desc->SizeBytes, 0);

			// 按字段反序列化
			for (const NNComponentFieldDesc& field : desc->Fields)
			{
				if (field.NameUtf8 != nullptr && componentJson.contains(field.NameUtf8))
				{
					JsonToField(componentJson[field.NameUtf8], temp.data(), field, archiveToHandle);
				}
			}

			// 写入 ECS
			void* dst = desc->GetComponentPtrFn(&scene, handle);
			if (dst != nullptr)
			{
				std::memcpy(dst, temp.data(), desc->SizeBytes);
			}

			// 后处理回调（如 Relationship 的 SetParent）
			if (desc->PostDeserializeFn != nullptr)
			{
				desc->PostDeserializeFn(scene, handle);
			}
		}
	}

	return true;
}
} // namespace NN::Runtime::Scene
