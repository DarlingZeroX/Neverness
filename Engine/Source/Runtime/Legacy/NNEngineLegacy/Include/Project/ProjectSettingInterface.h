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
#include <NNCore/Include/File/NlohmannJson.h>

namespace NN::Runtime
{
	struct ProjectSettingInterface
	{
		virtual ~ProjectSettingInterface() = default;

		virtual void Load(const nlohmann::json& json) = 0;
		virtual void Save(nlohmann::json& json) = 0;
	};

}
