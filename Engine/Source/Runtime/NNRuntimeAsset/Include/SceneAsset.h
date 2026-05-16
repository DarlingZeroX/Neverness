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
#include "NNRuntimeCore/Interface/VGAsset.h"
#include "NNRuntimeCore/Interface/SceneInterface.h"
//#include "../Scene/Scene.h"

namespace NN::Runtime
{
	struct SceneAsset : public VGAsset
	{
		Ref<IScene> LoadedScene = nullptr;
		IScene* WriteScene = nullptr;

		SceneAsset()
			: VGAsset("Scene")
		{}
	};

	struct SceneSerializeFormatHeader {
		std::string magic = "VGSCENE";  // 魔数
		uint32_t majorVersion = 0;
		uint32_t minorVersion = 1;
		std::string loader = "DEFAULT;"; // 加载器标识;
	};
}