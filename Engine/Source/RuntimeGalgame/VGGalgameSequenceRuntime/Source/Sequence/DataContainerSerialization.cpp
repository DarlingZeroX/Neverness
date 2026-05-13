/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Sequence/DataContainerSerialization.h"
#include "Sequence/Components.h"
#include "IVGSSequenceComponent.h"

namespace VisionGal
{
	// =========================================================================
	// 内置序列组件：component_to_json / component_from_json
	//
	// 说明：二者与具体组件类型同处命名空间 VisionGal，供
	// TVGSSequenceComponentJsonBinding<T> 通过 ADL 解析调用。
	// 新增字段时只需修改对应函数体，并在注释中注明版本/兼容性。
	// =========================================================================

	void component_to_json(const VGSSC_CommonDialogue& c, nlohmann::json& out)
	{
		out = nlohmann::json{
			{ "characterID", c.CharacterID },
			{ "dialogueCharacterName", c.DialogueCharacterName },
			{ "dialogueText", c.DialogueText },
			{ "wait", c.wait },
		};
	}

	void component_from_json(const nlohmann::json& in, VGSSC_CommonDialogue& c)
	{
		c.CharacterID = in.value("characterID", static_cast<VGSSCharacterObjectID>(0));
		c.DialogueCharacterName = in.value("dialogueCharacterName", std::string{});
		c.DialogueText = in.value("dialogueText", std::string{});
		c.wait = in.value("wait", true);
	}

	void component_to_json(const VGSSC_ChangeFigure& c, nlohmann::json& out)
	{
		out = nlohmann::json{
			{ "characterID", c.CharacterID },
			{ "textureID", c.TextureID },
			{ "textureResourcePath", c.TextureResourcePath },
			{ "showState", c.ShowState },
			{ "wait", c.Wait },
		};
	}

	void component_from_json(const nlohmann::json& in, VGSSC_ChangeFigure& c)
	{
		c.CharacterID = in.value("characterID", static_cast<VGSSCharacterObjectID>(0));
		c.TextureID = in.value("textureID", static_cast<VGSSSpriteObjectID>(0));
		c.TextureResourcePath = in.value("textureResourcePath", std::string{});
		c.ShowState = in.value("showState", true);
		c.Wait = in.value("wait", true);
	}

	void component_to_json(const VGSSC_ChangeBackground& c, nlohmann::json& out)
	{
		out = nlohmann::json{
			{ "textureID", c.TextureID },
			{ "textureResourcePath", c.TextureResourcePath },
			{ "showState", c.ShowState },
			{ "wait", c.Wait },
		};
	}

	void component_from_json(const nlohmann::json& in, VGSSC_ChangeBackground& c)
	{
		c.TextureID = in.value("textureID", static_cast<VGSSSpriteObjectID>(0));
		c.TextureResourcePath = in.value("textureResourcePath", std::string{});
		c.ShowState = in.value("showState", true);
		c.Wait = in.value("wait", true);
	}

	// =========================================================================
	// 注册表实现
	// =========================================================================

	VGSSequenceComponentJsonRegistry& VGSSequenceComponentJsonRegistry::Get()
	{
		static VGSSequenceComponentJsonRegistry instance;
		return instance;
	}

	void VGSSequenceComponentJsonRegistry::Register(std::string typeNameID, VGSSequenceComponentJsonBindingPtr binding)
	{
		if (!binding || typeNameID.empty())
			return;
		m_BindingsByType[std::move(typeNameID)] = std::move(binding);
	}

	VGSSequenceComponentJsonBindingPtr VGSSequenceComponentJsonRegistry::Find(const std::string& typeNameID) const
	{
		const auto it = m_BindingsByType.find(typeNameID);
		if (it == m_BindingsByType.end())
			return nullptr;
		return it->second;
	}

	// =========================================================================
	// 静态注册：确保在使用序列化前已挂载内置类型的绑定
	//
	// 注意：type 字符串必须与各组件 GetTypeNameID() 完全一致，
	// 否则 CreateSequenceEntryByTypeNameID 无法创建实例，反序列化会失败。
	// =========================================================================

	namespace
	{
		struct VGSSequenceJsonBuiltinRegistration
		{
			VGSSequenceJsonBuiltinRegistration()
			{
				auto& reg = VGSSequenceComponentJsonRegistry::Get();
				reg.Register(
					VGSSC_CommonDialogue::StaticGetTypeNameID(),
					std::make_shared<TVGSSequenceComponentJsonBinding<VGSSC_CommonDialogue>>());
				reg.Register(
					VGSSC_ChangeFigure::StaticGetTypeNameID(),
					std::make_shared<TVGSSequenceComponentJsonBinding<VGSSC_ChangeFigure>>());
				reg.Register(
					VGSSC_ChangeBackground::StaticGetTypeNameID(),
					std::make_shared<TVGSSequenceComponentJsonBinding<VGSSC_ChangeBackground>>());
			}
		};

		const VGSSequenceJsonBuiltinRegistration g_VGSSequenceJsonBuiltinRegistration;
	}

	// =========================================================================
	// 容器级序列化 / 反序列化
	// =========================================================================

	nlohmann::json SerializeVGSSequenceDataContainerToJson(const VGSSequenceDataContainer& container)
	{
		nlohmann::json sequence = nlohmann::json::array();
		auto& reg = VGSSequenceComponentJsonRegistry::Get();

		for (const Ref<IVGSSequenceComponent>& entry : container.m_Sequence)
		{
			if (!entry)
				continue;

			const std::string typeId = entry->GetTypeNameID();
			nlohmann::json data = nlohmann::json::object();
			if (const auto binding = reg.Find(typeId))
				binding->SerializeData(entry.get(), data);

			nlohmann::json item = nlohmann::json::object();
			item[kVGSSequenceJsonEntryTypeKey] = typeId;
			item[kVGSSequenceJsonEntrySequenceIndexKey] = entry->SequenceIndex;
			item[kVGSSequenceJsonEntryDataKey] = std::move(data);
			sequence.push_back(std::move(item));
		}

		nlohmann::json root = nlohmann::json::object();
		root[kVGSSequenceJsonFormatVersionKey] = kVGSSequenceJsonFormatVersion;
		root[kVGSSequenceJsonSequenceKey] = std::move(sequence);
		return root;
	}

	bool DeserializeVGSSequenceDataContainerFromJson(const nlohmann::json& root, VGSSequenceDataContainer& out)
	{
		if (!root.is_object())
			return false;

		const int version = root.value(kVGSSequenceJsonFormatVersionKey, kVGSSequenceJsonFormatVersion);
		(void)version;
		// 预留：version < kVGSSequenceJsonFormatVersion 时在此做字段迁移

		if (!root.contains(kVGSSequenceJsonSequenceKey) || !root[kVGSSequenceJsonSequenceKey].is_array())
			return false;

		const nlohmann::json& sequence = root[kVGSSequenceJsonSequenceKey];
		VGSSequenceDataContainer tmp;
		auto& reg = VGSSequenceComponentJsonRegistry::Get();

		for (const auto& el : sequence)
		{
			if (!el.is_object())
				return false;

			const std::string typeId = el.value(kVGSSequenceJsonEntryTypeKey, std::string{});
			if (typeId.empty())
				return false;

			Ref<IVGSSequenceComponent> comp = CreateSequenceEntryByTypeNameID(typeId);
			if (!comp)
				return false;

			const auto binding = reg.Find(typeId);
			if (!binding)
				return false;

			comp->SequenceIndex = el.value(kVGSSequenceJsonEntrySequenceIndexKey, 0u);

			const nlohmann::json& data = el.contains(kVGSSequenceJsonEntryDataKey)
				? el[kVGSSequenceJsonEntryDataKey]
				: nlohmann::json::object();

			if (!data.is_object())
				return false;

			if (!binding->DeserializeData(data, comp.get()))
				return false;

			tmp.AppendEntry(comp);
		}

		out = std::move(tmp);
		return true;
	}

	std::string SerializeVGSSequenceDataContainerToString(const VGSSequenceDataContainer& container, int indent)
	{
		return SerializeVGSSequenceDataContainerToJson(container).dump(indent);
	}

	bool DeserializeVGSSequenceDataContainerFromString(const std::string& text, VGSSequenceDataContainer& out)
	{
		try
		{
			const nlohmann::json root = nlohmann::json::parse(text);
			return DeserializeVGSSequenceDataContainerFromJson(root, out);
		}
		catch (const nlohmann::json::parse_error&)
		{
			return false;
		}
		catch (const nlohmann::json::type_error&)
		{
			return false;
		}
	}
}
