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
#include <unordered_map>
#include <HCore/Interface/HConfig.h>
#include <VGEditorCore/Interface/UIInterface.h>
#include "../../VGGalEditorConfig.h"
#include "ComponentDrawerInterface.h"

namespace VisionGal::Editor
{
	class VG_GALGAME_EDITOR_API GalSeqComDrawerRegistry
	{
	public:
		GalSeqComDrawerRegistry();
		~GalSeqComDrawerRegistry() = default;

		bool RegisterDrawer(const Ref<IGalSeqComDrawer>& drawer);

		IGalSeqComDrawer* GetDrawer(const std::string& type);

		static GalSeqComDrawerRegistry& GetInstance();
	private:
		std::unordered_map<std::string,Ref< IGalSeqComDrawer >> m_Drawers;
	};
}
