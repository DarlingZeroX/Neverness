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

#include <VGImgui/IncludeImGui.h>
#include <VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h>
#include <vector>
#include <string>
#include "EntryUIData.h"
#include "VGGalgameScriptVisual/Include/VisualSequence/SequenceComponents.h"

namespace VisionGal::Editor
{
	class SequenceComponentUI
	{
	public:
		SequenceComponentUI();
		~SequenceComponentUI() = default;

		void ShowDemoIconsUI();

		Horizon::HEventDelegate<std::string> OnIconClicked;
	private:
		std::vector<std::string> m_Headers;
		std::vector<std::vector<SequenceEntryUIData>> m_IconLabels;
	};
}
