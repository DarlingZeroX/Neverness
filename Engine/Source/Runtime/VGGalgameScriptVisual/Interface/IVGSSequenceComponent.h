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

#include <memory>
#include <string>
#include <unordered_map>

#include "../VGGalScriptVisualConfig.h"
#include <HCore/Interface/HConfig.h>

namespace VisionGal
{
	using IVGSSeqComID = unsigned;

	using VGSCharacterObjectID = unsigned;

	// Visual GalGame Script Sequence Component Interface
	// 可视化 GalGame 脚本序列组件接口
	struct IVGSSequenceComponent
	{
		virtual ~IVGSSequenceComponent() = default;

		virtual std::string GetTypeNameID() = 0;
		virtual Ref<IVGSSequenceComponent> Clone() = 0;

		unsigned SequenceIndex; // 所属序列 ID
	};

	struct VG_GALGAME_SCRIPT_VISUAL_API IVGSSequenceComponentManager
	{
		IVGSSequenceComponentManager();
		~IVGSSequenceComponentManager() = default;

		static IVGSSequenceComponentManager& Get();

		Ref<IVGSSequenceComponent> CreateSequenceEntryByTypeNameID(const std::string& typeNameID);

		std::unordered_map<std::string, Ref<IVGSSequenceComponent>> RegisteredComponents;
	};

	VG_GALGAME_SCRIPT_VISUAL_API Ref<IVGSSequenceComponent> CreateSequenceEntryByTypeNameID(const std::string& typeNameID);
}
