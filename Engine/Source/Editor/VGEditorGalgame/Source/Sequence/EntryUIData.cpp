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

#include "Sequence/EntryUIData.h"
#include "VGImgui/Include/ImGuiEx/IconFont/IconsFontAwesome5Pro.h"
#include "VGGalgameScriptVisual/Include/VisualSequence/SequenceComponents.h"

namespace VisionGal::Editor
{
	SequenceEntryUIData::SequenceEntryUIData(const std::string& iconLabel, const std::string& label)
	{
		FullLabel = iconLabel + " " + label;
	}

	SequenceEntryUIDataManager& SequenceEntryUIDataManager::GetInstance()
	{
		static SequenceEntryUIDataManager instance;
		return instance;
	}

	SequenceEntryUIData SequenceEntryUIDataManager::GetDataByTypeNameID(const std::string& typeNameID)
	{
		auto& instance = GetInstance();
		auto it = instance.DataMap.find(typeNameID);
		if (it != instance.DataMap.end())
		{
			return it->second;
		}
		return SequenceEntryUIData("", "Unknown");
	}

	SequenceEntryUIDataManager::SequenceEntryUIDataManager()
	{
		SequenceEntryUIData dialog(ICON_FA_COMMENT_ALT, "普通对话");
		dialog.TypeNameID = VGSSC_CommonDialogue::StaticGetTypeNameID();
		DataMap[dialog.TypeNameID] = dialog;

		SequenceEntryUIData changeFigure(ICON_FA_USER, "切换立绘");
		changeFigure.TypeNameID = VGSSC_ChangeFigure::StaticGetTypeNameID();
		DataMap[changeFigure.TypeNameID] = changeFigure;

		SequenceEntryUIData changeBackground(ICON_FA_IMAGES, "切换背景");
		changeBackground.TypeNameID = VGSSC_ChangeBackground::StaticGetTypeNameID();
		DataMap[changeBackground.TypeNameID] = changeBackground;
	}
}
