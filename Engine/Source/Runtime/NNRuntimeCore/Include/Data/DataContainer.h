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

#pragma once
#include "../Core/Core.h"
#include "DataNamespace.h"
#include <NNKernel/Include/File/nlohmann/json.hpp>

namespace VisionGal
{
	class VG_CORE_API VGDataContainer
	{
	public:
		VGDataContainer();
		~VGDataContainer() = default;

		VGDataNamespace* GetNamespace(const std::string& ns);
		void Clear();

		void Serialize(nlohmann::json& json);
		void Deserialize(nlohmann::json& json);
	private:
		std::unordered_map<std::string, Ref<VGDataNamespace>> m_Namespaces;
	};
}
