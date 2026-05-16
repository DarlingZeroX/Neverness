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

#include "Data/DataContainer.h"

namespace VisionGal
{
	VGDataContainer::VGDataContainer()
	{
	}

	VGDataNamespace* VGDataContainer::GetNamespace(const std::string& name)
	{
		if (m_Namespaces.find(name) != m_Namespaces.end())
		{
			return m_Namespaces[name].get();
		}

		// 没有就创建一个新的
		Ref<VGDataNamespace> ns = std::make_shared<VGDataNamespace>();
		m_Namespaces[name] = ns;
		return ns.get();
	}

	void VGDataContainer::Clear()
	{
		m_Namespaces.clear();
	}

	void VGDataContainer::Serialize(nlohmann::json& json)
	{
		for (auto& [name, ns] : m_Namespaces)
		{
			nlohmann::json nsJson;
			ns->Serialize(nsJson);
			json[name] = nsJson;
		}
	}

	void VGDataContainer::Deserialize(nlohmann::json& json)
	{
		for (const auto& [name, ns] : json.items())
		{
			GetNamespace(name)->Deserialize(ns);
		}
	}
}
