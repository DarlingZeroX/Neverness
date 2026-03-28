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
#include "../VGGalCoreConfig.h"
#include "VGCore/Include/Data/DataContainer.h"

namespace VisionGal::GalGame
{
	class VG_GALGAME_CORE_API ArchiveDataContainer: public VGDataContainer
	{
	public:
		ArchiveDataContainer() = default;
		~ArchiveDataContainer() = default;

		VGDataNamespace* GetChoicesNamespace();
		VGDataNamespace* GetInputNamespace();

		static void InitializeLuaBinding(sol::table& L);
	};
}