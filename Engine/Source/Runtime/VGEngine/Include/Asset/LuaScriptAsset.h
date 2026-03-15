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
#include "VGCore/Interface/VGAsset.h"
#include "../EngineConfig.h"

namespace VisionGal
{
	struct LuaScriptAsset : public VGAsset
	{
		std::string Text;

		LuaScriptAsset()
			: VGAsset("LuaScript")
		{
		}
	};

	class VG_ENGINE_API LuaScriptAssetWriter : public IAssetWriter
	{
	public:
		LuaScriptAssetWriter() = default;
		~LuaScriptAssetWriter() override = default;

		bool Write(const std::string path, VGAsset* asset) override;
	};

}